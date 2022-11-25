#include "Helpers.hpp"
#include "IPC/IPC.hpp"
#include "math/CVHelpers.hpp"
#include "utils/Env.hpp"
#include "utils/LogBatch.hpp"
#include "utils/SteadyTimer.hpp"
#include "utils/Types.hpp"

#include <opencv2/core/opengl.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <array>
#include <condition_variable>
#include <format>
#include <mutex>
#include <random>
#include <set>
#include <sstream>
#include <thread>

inline std::ostream& operator<<(std::ostream& os, const Pose& pose)
{
    return os << pose.position.x << ' '
              << pose.position.y << ' '
              << pose.position.z << ' '
              << pose.rotation.w << ' '
              << pose.rotation.x << ' '
              << pose.rotation.y << ' '
              << pose.rotation.z;
}

inline std::istream& operator>>(std::istream& is, Pose& pose)
{
    return is >> pose.position.x >> pose.position.y >> pose.position.z >>
           pose.rotation.w >> pose.rotation.x >> pose.rotation.y >> pose.rotation.z;
}

constexpr double Lerp(const double a, const double b, const double f)
{
    return (a * (1.0 - f)) + (b * f);
}

constexpr double Map(double value, double inStart, double inEnd, double outStart, double outEnd)
{
    const double slope = (outEnd - outStart) / (inEnd - inStart);
    return outStart + slope * (value - inStart);
}
template <typename TRatio>
consteval double RatioToDouble()
{
    return static_cast<double>(TRatio::num) / static_cast<double>(TRatio::den);
}
template <typename inStart, typename inEnd, typename outStart, typename outEnd>
constexpr double CompMap(double value)
{
    constexpr double in1 = RatioToDouble<inStart>();
    constexpr double in2 = RatioToDouble<inEnd>();
    constexpr double out1 = RatioToDouble<outStart>();
    constexpr double out2 = RatioToDouble<outEnd>();
    constexpr double slope = (out2 - out1) / (in2 - in1);
    return out1 + slope * (value - in1);
}

static inline const cv::Quatd IdentityQuat{1, 0, 0, 0};

static constexpr int CX = 0; /// index the x component of a cv::Vec
static constexpr int CY = 1; /// index the y component of a cv::Vec
static constexpr int CZ = 2; /// index the z component of a cv::Vec

/// assume most functions return positions and rotations in openvrs coordinate system
inline void QuatCVToVR(cv::Quatd& q)
{
    q.y = -q.y;
}

inline cv::Quatd ShortestArc(const cv::Vec3d& a, const cv::Vec3d& b)
{
    const cv::Vec3d xyz = a.cross(b);
    const double w = std::sqrt(a.dot(a) * b.dot(b)) + a.dot(b);
    return {w, xyz[0], xyz[1], xyz[2]};
}

template <typename TFactor = std::ratio<1, 100>>
struct Smooth
{
    static constexpr double factor = RatioToDouble<TFactor>();

    constexpr explicit Smooth(double initial) : value(initial) {}
    constexpr Smooth& operator=(double rhs)
    {
        value = Lerp(value, rhs, factor);
        return *this;
    }
    constexpr explicit operator double() const { return value; }

    double value;
};

template <typename T, int WindowSize = 10>
struct Average
{
    constexpr explicit Average(const T& initial)
    {
        std::fill(values.begin(), values.end(), initial);
    }
    constexpr Average& operator=(const T& rhs)
    {
        values[index++] = rhs;
        if (index >= WindowSize) index = 0;
        return *this;
    }
    constexpr explicit operator T() const
    {
        if constexpr (std::is_integral_v<T>)
        {
            long long total = 0;
            for (const auto& value : values)
            {
                total += value;
            }
            return static_cast<T>(total / static_cast<long long>(WindowSize));
        }
        else
        {
            double total = 0;
            for (const auto& value : values)
            {
                total += static_cast<double>(value);
            }
            return static_cast<T>(total / static_cast<double>(WindowSize));
        }
    }

    std::array<T, WindowSize> values{};
    int index = 0;
};

template <typename TRandEngine>
inline cv::Vec3d RandomPosition(TRandEngine& randEngine, const cv::Vec3d& lower, const cv::Vec3d& upper)
{
    return {std::uniform_real_distribution<double>(lower[0], upper[0])(randEngine),
            std::uniform_real_distribution<double>(lower[1], upper[1])(randEngine),
            std::uniform_real_distribution<double>(lower[2], upper[2])(randEngine)};
}

struct RandomMover
{
    RandomMover(const cv::Vec3d& pos, const cv::Vec3d& low, const cv::Vec3d& upp, double s)
        : start(pos), target(pos), lower(low), upper(upp), step(s) {}

    template <typename TRand>
    cv::Vec3d Next(TRand& randEngine)
    {
        if (dist >= 1.0)
        {
            dist = 0.0;
            start = target;
            target = RandomPosition(randEngine, lower, upper);
        }

        dist += step;
        return (start * (1.0 - dist)) + (target * dist);
    }
    cv::Vec3d GetForward() const
    {
        const cv::Vec3d vec = target - start;
        return vec / Length(vec);
    }

    cv::Vec3d start;
    cv::Vec3d target;
    cv::Vec3d lower;
    cv::Vec3d upper;
    double step;

    double dist = 1.0;
};

static const cv::Scalar COLOR_GRID_ACCENT{255, 190, 190};
static const cv::Scalar COLOR_FORWARD_LINE{255, 40, 10};
static const cv::Scalar COLOR_WHITE{255, 255, 255};
static const cv::Scalar COLOR_GRID{230, 230, 230};
static const cv::Scalar COLOR_MARKER_UP{52, 235, 103};
static const cv::Scalar COLOR_MARKER_RIGHT{255, 89, 64};
static const cv::Scalar COLOR_MARKER_FRONT{39, 80, 150};
static const cv::Scalar COLOR_MARKER{255, 124, 107};
static const cv::Scalar COLOR_MARKER_INVERSE{255, 66, 167};
static const cv::Scalar COLOR_TEXT{0, 0, 0};

struct SceneImage
{
    static inline const cv::Size2i imageSize{800, 800};
    static inline const cv::Point2i imageCenter = imageSize / 2;
    static inline const double meterToPx = imageSize.width / 4.0;

    // openvr is +x right, +y up, -z forward
    static inline const cv::Vec3d oneForward{0, 0, -1};
    static inline const cv::Vec3d oneUp{0, 1, 0};
    static inline const cv::Vec3d oneRight{1, 0, 0};
    static inline const cv::Vec3d dirForward{0, 0, -1};
    static inline const cv::Vec3d dirUp{0, 1, 0};
    static inline const cv::Vec3d dirRight{1, 0, 0};
    static inline const cv::Vec2d oneUp2d{0, -1};
    static inline const cv::Vec2d oneRight2d{1, 0};
    static inline const cv::Point2i zero2i{};
    static inline const cv::Vec3d zero3d{};

    static constexpr Index numCorners = 4;
    static constexpr Index numFaces = 6;
    static inline const std::array<cv::Vec3d, numCorners> modelMarkerFace{
        cv::Vec3d(-1.0, 1, -1.0), // front left
        cv::Vec3d(1.0, 1, -1.0),  // front right
        cv::Vec3d(1.0, 1, 1.0),   // back right
        cv::Vec3d(-1.0, 1, 1.0)}; // back left

    template <math::VecLike<2> T>
    static cv::Point2i PosToPix(const T& pos)
    {
        return {static_cast<int>(math::IndexVec(pos, 0) * meterToPx) + imageCenter.x,
                static_cast<int>(math::IndexVec(pos, 1) * meterToPx) + imageCenter.y};
    }
    template <math::VecLike<3> T>
    static cv::Point2i PosToPix(const T& pos)
    {
        return {static_cast<int>(math::IndexVec(pos, 0) * meterToPx) + imageCenter.x,
                static_cast<int>(math::IndexVec(pos, 2) * meterToPx) + imageCenter.y};
    }

    SceneImage() : image(imageSize, CV_8UC3)
    {
        cv::namedWindow("scene");
        image = COLOR_WHITE; // NOLINT: fills with color
        cv::imshow("scene", image);
    }

    void DrawGrid(const int cellSize, const cv::Scalar& color)
    {
        const int widthMaxCells = imageSize.width / cellSize; // floored
        for (int i = 0; i < widthMaxCells; ++i)
        {
            const int pos = ((i - (widthMaxCells / 2)) * cellSize) + imageCenter.x;
            cv::line(image, cv::Point2i(pos, 0), cv::Point2i(pos, imageSize.height), color);
        }
        const int heightMaxCells = imageSize.height / cellSize; // floored
        for (int i = 0; i < heightMaxCells; ++i)
        {
            const int pos = ((i - (heightMaxCells / 2)) * cellSize) + imageCenter.y;
            cv::line(image, cv::Point2i(0, pos), cv::Point2i(imageSize.width, pos), color);
        }
    }

    void DrawLine(const cv::Point2d& start, const cv::Point2d& end, const cv::Scalar& color)
    {
        cv::line(image, PosToPix(start), PosToPix(end), color);
    }

    void DrawText(const cv::Vec2d& pos, const std::string& str)
    {
        cv::putText(image, str, PosToPix(pos), cv::FONT_HERSHEY_TRIPLEX, 0.55, cv::Scalar(0));
    }

    struct Quads
    {
        Quads(const std::array<cv::Point2i, 4>& corners, cv::Scalar color, const double depth)
            : corners(corners), color(std::move(color)), depth(depth) {}
        std::array<cv::Point2i, 4> corners;
        cv::Scalar color;
        double depth = 0;
        constexpr auto operator<(const Quads& rhs) const noexcept { return depth < rhs.depth; }
    };
    std::set<Quads> mTriDrawList{};
    void DrawQuads()
    {
        for (const auto& tri : mTriDrawList)
        {
            cv::fillConvexPoly(image, tri.corners.data(), static_cast<int>(tri.corners.size()), tri.color, cv::LINE_8);
        }
        mTriDrawList.clear();
    }
    void AddDrawBoxFace(const cv::Vec3d& halfSize, const cv::Vec3d& pos, const cv::Quatd& rot, const cv::Scalar& color)
    {
        std::array<cv::Point2i, numCorners> points;
        double avgHeight = 0;
        for (Index i = 0; i < numCorners; ++i)
        {
            cv::Vec3d corner = modelMarkerFace[i].mul(halfSize);
            RotateVecByQuat(corner, rot);
            corner += pos;
            avgHeight += corner[CY];
            points[i] = PosToPix(corner);
        }
        avgHeight /= numCorners;
        mTriDrawList.emplace(points, color, avgHeight);
    }
    void AddDrawBox(cv::Size2d size, const cv::Vec3d& pos, const cv::Quatd& rot)
    {
        const double height = CompMap<std::ratio<0>, std::ratio<3>, std::ratio<0>, std::ratio<1, 8>>(pos[CY]);
        const double sl = (size.width / 2.0) + height;
        const double ss = (size.height / 2.0) + height;
        constexpr Index halfFaces = 3;
        const std::array sizes{cv::Vec3d(sl, ss, sl), cv::Vec3d(ss, sl, sl), cv::Vec3d(sl, sl, ss)};
        static const std::array rots{IdentityQuat, cv::Quatd::createFromZRot(PI / 2.0), cv::Quatd::createFromXRot(PI / 2.0)};
        static const std::array colors{COLOR_MARKER_UP, COLOR_MARKER_RIGHT, COLOR_MARKER_FRONT};
        for (Index i = 0; i < halfFaces; ++i)
        {
            AddDrawBoxFace(sizes[i], pos, rot * rots[i], colors[i]);
            AddDrawBoxFace(sizes[i], pos, rot * rots[i].inv(cv::QUAT_ASSUME_UNIT), colors[i]);
        }
    }

    void DrawMarkerInfo(int id, const cv::Vec3d& positionM, const cv::Quatd& rotationM)
    {
        cv::Vec3d forward = dirForward;
        RotateVecByQuat(forward, rotationM);
        cv::Vec2d forward2 = cv::Vec2d{forward[0], forward[2]};
        forward2 /= Length(forward2);
        const cv::Vec2d centerP = cv::Vec2d(positionM[CX], positionM[CZ]);
        DrawLine(centerP, centerP + forward2 * 0.4, COLOR_FORWARD_LINE);
        DrawText(centerP + cv::Vec2d(-0.02, +0.01), std::format("{}", id));
    }

    void Draw()
    {
        DrawGrid(static_cast<int>(meterToPx), COLOR_GRID);

        DrawLine(cv::Point2d(-oneRight2d), cv::Point2d(oneRight2d), COLOR_GRID_ACCENT);
        DrawLine(cv::Point2d(-oneUp2d), cv::Point2d(oneUp2d), COLOR_GRID_ACCENT);
    }

    void Show()
    {
        cv::imshow("scene", image);
        image = COLOR_WHITE;
        index = 0;
    }

    void DebugText(const std::string& str)
    {
        constexpr int lineHeight = 13;
        cv::putText(image, str,
                    cv::Point2i(10, lineHeight * (++index)),
                    cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0));
    }

    cv::Mat image;
    int index = 0;

    std::random_device mRandDevice{};
    std::default_random_engine mRandEngine{mRandDevice()};
};

enum class KeyCode
{
    None = 0,
    Escape = 27,
};
KeyCode WaitEndOfFrame(utils::NanoS frameTime, utils::NanoS targetFrameTime)
{
    const utils::NanoS timeToWait = targetFrameTime - frameTime;
    if (timeToWait.count() <= 0) return KeyCode::None;
    const auto ms = duration_cast<utils::MilliS>(timeToWait);
    const int key = cv::waitKey(static_cast<int>(ms.count()));
    return static_cast<KeyCode>(key);
}

template <typename... TArgs>
std::string BuildCommand(std::string_view name, const TArgs&... args)
{
    std::ostringstream ss;
    ss << name
       << std::fixed << std::setprecision(6);
    ((ss << ' ' << args), ...);
    return ss.str();
}

class ServerThread
{
public:
    explicit ServerThread(std::unique_ptr<IPC::IServer>&& server) : mServer(std::move(server))
    {
        ATT_ASSERT(utils::IsMainThread());
        mThread = std::thread(&ServerThread::ThreadLoop, this);
    }
    ~ServerThread()
    {
        ATT_ASSERT(utils::IsMainThread());
        AllowThreadToStop();
        if (mThread.joinable()) mThread.join();
    }

    [[nodiscard]] std::string GetWaiting()
    {
        ATT_ASSERT(!utils::IsThread(mThread));
        const std::lock_guard lock{mMsgMutex};
        if (mMsgWaiting.empty()) return {};
        std::string temp = mMsgWaiting;
        mMsgWaiting.clear();
        return temp;
    }
    void SetToSend(std::string message)
    {
        ATT_ASSERT(!utils::IsThread(mThread));
        ATT_ASSERT(!message.empty());
        {
            const std::lock_guard lock{mMsgMutex};
            if (!mMsgWaiting.empty()) throw utils::MakeError("unhandled message waiting");
            if (!mMsgToSend.empty()) return; // dont overwrite
            mMsgToSend = std::move(message);
            mIsReadyToSend = true;
        }
        mIsReadyToSendCond.notify_one();
    }

private:
    void ThreadWork()
    {
        const auto conn = mServer->Accept();
        const std::string_view msg = conn->Recv();
        if (msg.empty()) return;

        std::unique_lock lock{mMsgMutex};
        if (!mMsgWaiting.empty()) throw utils::MakeError("unhandled message waiting");
        if (!mMsgToSend.empty()) throw utils::MakeError("unhandled message to send");
        mMsgWaiting = msg;

        mIsReadyToSendCond.wait(lock, [&]
                                { return mIsReadyToSend; });
        mIsReadyToSend = false;

        if (!mMsgWaiting.empty()) throw utils::MakeError("unhandled message waiting");
        if (mMsgToSend.empty()) throw utils::MakeError("no message to send");
        conn->Send(mMsgToSend);
        mMsgToSend.clear();
    }

    void ThreadLoop()
    {
        mIsThreadRunning = true;
        while (mIsThreadRunning)
        {
            try
            {
                ThreadWork();
            }
            catch (const std::exception& e)
            {
                ATT_LOG_ERROR("exception in server thread: ", e.what());
                return;
            }
        }
    }

    void AllowThreadToStop()
    {
        mIsThreadRunning = false;
        {
            const std::lock_guard lock{mMsgMutex};
            mMsgToSend = "";
            mMsgWaiting = "";
            mIsReadyToSend = true;
        }
        mIsReadyToSendCond.notify_one();
    }

    std::unique_ptr<IPC::IServer> mServer;

    /// message queue that only allows depth of 1
    std::string mMsgWaiting{};
    std::string mMsgToSend{};
    bool mIsReadyToSend = false;
    std::condition_variable mIsReadyToSendCond{};
    std::mutex mMsgMutex{};

    bool mIsThreadRunning = false;
    std::thread mThread{};
};

class OpenVRDriver
{
    static constexpr std::string_view DRIVER_VERSION = ATT_STRINGIZE(ATT_DRIVER_VERSION);

    static int GetIdFromName(std::string_view name)
    {
        static constexpr std::string_view namePrefix = "ApriltagTracker";
        if (name.size() < namePrefix.size()) throw utils::MakeError("short tracker name: ", name);
        if (!name.starts_with(namePrefix)) throw utils::MakeError("invalid tracker name: ", name);
        int outId = 0;
        auto [ptr, ec] = std::from_chars(name.data() + namePrefix.size(), name.data() + name.size(), outId);
        if (ec != std::errc() || ptr != (name.data() + name.size())) throw utils::MakeError("invalid tracker id: ", name);
        if (outId < 0) throw utils::MakeError("negative tracker id: ", name);
        return outId;
    }

public:
    enum class TrackerStatus
    {
        Invalid = -1,
        Valid = 0,
        Late = 1,
    };
    struct Station
    {
        Pose pose = Pose::Ident();
    };
    struct TrackerDevice
    {
        TrackerDevice(std::string name, std::string role) : name(std::move(name)), role(std::move(role)) {}

        std::string name;
        std::string role;
        Pose pose = Pose::Ident();
        TrackerStatus status = TrackerStatus::Valid;
        utils::FSeconds lastUpdate = utils::FSeconds::zero();
        utils::FSeconds timeOffset = utils::FSeconds::zero();
    };

    explicit OpenVRDriver(std::unique_ptr<IPC::IServer>&& server) : mServer(std::move(server)) {}

    void PollMessage()
    {
        static std::string received{};
        received = mServer.GetWaiting();
        if (received.empty()) return;
        auto trimSpace = received.find_first_not_of(' ');
        if (trimSpace != std::string::npos) received.erase(0, trimSpace);
        std::istringstream iss{std::move(received)};

        static std::string response{};
        try
        {
            response = HandleMessage(iss);
        }
        catch (const std::exception& e)
        {
            response = "command '" + received + "' error: ";
            response += e.what();
        }
        mServer.SetToSend(response);
    }

    const std::vector<Station>& GetStations() const { return mStations; }
    const std::vector<TrackerDevice>& GetTrackers() const { return mTrackers; }

private:
    std::string HandleMessage(std::istream& msg)
    {
        std::string name;
        msg >> name;
        // pose = x y z qw qx qy qz
        // 'updatepose' id pose time smoothing -> 'updated'
        if (name == "updatepose")
        {
            int inId = 0;
            Pose inPose = Pose::Ident();
            double inTimeOffset = 0;
            double inSmoothing = 0; // ignored
            msg >> inId >> inPose >> inTimeOffset >> inSmoothing;
            auto& device = mTrackers.at(inId);
            device.pose = inPose;
            device.timeOffset = utils::FSeconds(inTimeOffset);
            device.lastUpdate = utils::FSeconds(std::chrono::steady_clock::now().time_since_epoch());
            ATT_LOG_BATCH("cmd: updatepose", inId);
            return "updated";
        }
        // 'settings' saved factor additional -> 'changed'
        if (name == "settings")
        {
            msg >> mMaxSavedFrames >> mSmoothingFactor >> mAdditionalSmoothing;
            ATT_LOG_DEBUG("cmd: settings");
            return "changed";
        }
        // 'gettrackerpose' id offset -> 'trackerpose' id pose status
        // status: -1 = invalid, 0 = valid, 1 = late
        if (name == "gettrackerpose")
        {
            int inId = 0;
            double inTimeOffset = 0;
            msg >> inId >> inTimeOffset;
            const auto& device = mTrackers.at(inId);
            ATT_LOG_BATCH("cmd: gettrackerpose", '.');
            return BuildCommand("trackerpose", inId, device.pose, static_cast<int>(device.status));
        }
        // 'updatestation' id pose -> 'updated'
        if (name == "updatestation")
        {
            int inId = 0;
            Pose inPose = Pose::Ident();
            msg >> inId >> inPose;
            auto& station = mStations.at(inId);
            station.pose = inPose;
            ATT_LOG_BATCH("cmd: updatestation", inId);
            return "updated";
        }
        // 'numtrackers' -> 'numtrackers' count version
        if (name == "numtrackers")
        {
            ATT_LOG_DEBUG("cmd: numtrackers");
            return BuildCommand("numtrackers", mTrackers.size(), DRIVER_VERSION);
        }
        // 'addtracker' name role -> 'added'
        if (name == "addtracker")
        {
            std::string inName;
            std::string inRole;
            msg >> inName >> inRole;
            const int id = GetIdFromName(inName);
            if (id != static_cast<int>(mTrackers.size())) throw utils::MakeError("out of order tracker id: ", id);
            ATT_LOG_DEBUG("cmd: addtracker ", inName, inRole);
            mTrackers.emplace_back(std::move(inName), std::move(inRole));
            return "added";
        }
        // 'addstation' -> 'added'
        if (name == "addstation")
        {
            mStations.emplace_back();
            ATT_LOG_DEBUG("cmd: addstation");
            return "added";
        }

        throw utils::MakeError("unexpected command: ", name);
    }

    ServerThread mServer;

    int mMaxSavedFrames = 0;
    double mSmoothingFactor = 0;
    double mAdditionalSmoothing = 0;

    std::vector<TrackerDevice> mTrackers{};
    std::vector<Station> mStations{};
};

int main(int, char**)
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);

    SceneImage scene{};

    utils::SteadyTimer frameTimer;
    const utils::SteadyTimer appTimer;

    Smooth<std::ratio<1, 10>> smoothFrameTime{1};

    OpenVRDriver driver{IPC::CreateDriverServer()};

    while (true)
    {
        frameTimer.Restart();
        driver.PollMessage();

        for (const auto& device : driver.GetTrackers())
        {
            if (device.role == "TrackerRole_Disabled") continue;
            scene.AddDrawBox(cv::Size2d(0.05, 0.1), device.pose.position, device.pose.rotation);
        }
        for (const auto& device : driver.GetStations())
        {
            scene.AddDrawBox(cv::Size2d(0.02, 0.04), device.pose.position, device.pose.rotation);
        }
        scene.DrawQuads();

        for (int id = 0; const auto& device : driver.GetTrackers())
        {
            if (device.role == "TrackerRole_Disabled") continue;
            scene.DrawMarkerInfo(id++, device.pose.position, device.pose.rotation);
        }

        scene.Draw();
        scene.DebugText(std::format("{:5.5} fps", 1 / static_cast<double>(smoothFrameTime)));
        scene.Show();

        const utils::NanoS frameTime = frameTimer.Get();
        const KeyCode key = WaitEndOfFrame(frameTime, utils::MilliS(16));
        if (key == KeyCode::Escape) break;

        const utils::NanoS totalFrameTime = frameTimer.Get();
        smoothFrameTime = utils::FSeconds(totalFrameTime).count();
    }

    return 0;
}
