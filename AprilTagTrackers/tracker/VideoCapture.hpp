#pragma once

#include "config/VideoStream.hpp"
#include "RefPtr.hpp"
#include "utils/Concepts.hpp"
#include "utils/SteadyTimer.hpp"

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <condition_variable>
#include <memory>
#include <mutex>

namespace tracker
{

struct CapturedFrame
{
    cv::Mat image;
    utils::SteadyTimer::TimePoint timestamp;
};

inline void swap(CapturedFrame& lhs, CapturedFrame& rhs) // NOLINT(*-naming)
{
    using std::swap;
    swap(lhs.image, rhs.image);
    swap(lhs.timestamp, rhs.timestamp);
}

class AwaitedFrame
{
public:
    void Set(CapturedFrame& inFrame)
    {
        { // lock scope
            std::lock_guard lock(mMutex);
            swap(inFrame, mFrame);
            mIsReady = true;
        }
        mReadyCond.notify_one();
    }
    void Get(CapturedFrame& outFrame)
    {
        std::unique_lock lock{mMutex};
        mReadyCond.wait(lock, [&] { return mIsReady; });

        mIsReady = false;
        swap(mFrame, outFrame);
    }

private:
    CapturedFrame mFrame{};
    std::mutex mMutex{};
    std::condition_variable mReadyCond{};
    bool mIsReady = false;
};

/// capture video from a camera
/// wrapper for opencv VideoCapture
class VideoCapture
{
public:
    /// fake api for user to specify custom ps3eye capture implementation
    /// must not conflict with existing cv::VideoCapture api
    static constexpr int CAP_PS3EYE = 9100;

    /// instance is linked to a CameraInfo, so a list of VideoCaptures will sync with the list of cameras in gui
    explicit VideoCapture(RefPtr<cfg::Camera> cameraInfo) : mCameraInfo(cameraInfo) {}

    bool TryOpen();
    void Close();
    /// blocks till frame is ready, matches fps of camera
    bool TryReadFrame(CapturedFrame& outFrame);
    bool IsOpen() const { return mCapture && mCapture->isOpened(); }

private:
    bool TryOpenCapture();
    bool TryOpenGStreamerCapture(int index);
    void SetCaptureOptions();
    void LogCaptureOptions();

    RefPtr<cfg::Camera> mCameraInfo;
    std::unique_ptr<cv::VideoCapture> mCapture = std::make_unique<cv::VideoCapture>();
};

template <typename T>
class SwapChannel
{
    struct SharedState
    {
        template <typename... Args>
        explicit SharedState(Args&&... args) : buffer(std::forward<Args>(args)...) {}

        T buffer;
        bool isReady = false;

        std::mutex mtx;
        std::condition_variable cv;
    };

public:
    class Consumer
    {
    public:
        friend class SwapChannel<T>;

        Consumer(Consumer&&) noexcept = default;
        Consumer(const Consumer&) = delete;

        /// @param outValue swapped with internal buffer
        /// @return bool whether new image was swapped
        bool Consume(T& outValue)
        {
            std::lock_guard lock(mState->mtx);
            if (!mState->isReady) return false;
            using std::swap;
            swap(outValue, mState->buffer); // ADL
            return true;
        }

        /// @param outValue swapped with internal buffer
        /// @return bool whether new image was swapped, indicates producer has been destroyed
        bool WaitConsume(T& outValue)
        {
            std::unique_lock lock(mState->mtx);
            mState->cv.wait(lock, [&] {
                return mState->isReady || mState.use_count() == 1;
            });
            if (!mState->isReady) return false;
            using std::swap;
            swap(outValue, mState->buffer);
            return true;
        }

        /// estimate if producer is still alive
        bool IsOpen() const { return mState.use_count() == 2; }

    private:
        explicit Consumer(std::shared_ptr<SharedState>&& state) : mState(std::move(state))
        {
            ATT_ASSERT(mState);
        }

        std::shared_ptr<SharedState> mState;
    };
    class Producer
    {
    public:
        friend class SwapChannel<T>;

        Producer(Producer&&) noexcept = default;
        Producer(const Producer&) = delete;

        /// @param value swapped with internal buffer
        void Produce(T& value)
        {
            { // lock scope
                std::lock_guard lock(mState->mtx);
                using std::swap;
                swap(value, mState->buffer); // ADL
                mState->isReady = true;
            }
            mState->cv.notify_one();
        }

        /// estimate is consumer still alive
        bool IsOpen() const { return mState.use_count() == 2; }

    private:
        explicit Producer(std::shared_ptr<SharedState>&& state) : mState(std::move(state))
        {
            ATT_ASSERT(mState);
        }

        std::shared_ptr<SharedState> mState;
    };

    template <typename... Args>
    static std::tuple<Producer, Consumer> Create(Args&&... bufferInit)
    {
        auto proPtr = std::make_shared<SharedState>(std::forward<Args>(bufferInit)...);
        auto conPtr = proPtr; // copy
        return std::make_tuple(Producer(std::move(proPtr)), Consumer(std::move(conPtr)));
    }
};

} // namespace tracker
