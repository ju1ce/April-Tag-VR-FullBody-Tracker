#pragma once

#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

#include <string>
#include <vector>

constexpr int APRILTAG_STANDARD = 0;
constexpr int APRILTAG_CIRCULAR = 1;
constexpr int ARUCO_4X4 = 2;
constexpr int APRILTAG_COLOR = 3;
constexpr int APRILTAG_CUSTOM29H10 = 4;

enum class MarkerFamily
{
    Standard41h12,
    Circle21h7,
    Custom29h10,
};

struct MarkerDetectionList
{
    std::vector<int> ids{};
    std::vector<std::array<cv::Point2d, 4>> corners{}; /// 4 corners in CCW order
    std::vector<cv::Point2d> centers{};

    cv::_InputArray GetCorners()
    {
        return cv::_InputArray::rawIn(corners);
    }
};

struct apriltag_detector;
struct apriltag_family;

class AprilTagWrapper
{
public:
    AprilTagWrapper(MarkerFamily family, double quadDecimate, int threadCount);
    ~AprilTagWrapper();
    AprilTagWrapper(AprilTagWrapper&& other) noexcept
        : mDetector(other.mDetector),
          mFamilyType(other.mFamilyType), mFamilyPtr(other.mFamilyPtr)
    {
        other.mDetector = nullptr;
        other.mFamilyPtr = nullptr;
    }
    AprilTagWrapper& operator=(AprilTagWrapper&& rhs) noexcept
    {
        std::swap(mDetector, rhs.mDetector);
        mFamilyType = rhs.mFamilyType;
        std::swap(mFamilyPtr, rhs.mFamilyPtr);
        return *this;
    }

    AprilTagWrapper(const AprilTagWrapper&) = delete;
    AprilTagWrapper& operator=(const AprilTagWrapper&) = delete;

    /// convert BGR image to single channel grayscale
    static void ConvertGrayscale(const cv::Mat& image, cv::Mat& outImage)
    {
        cv::cvtColor(image, outImage, cv::COLOR_BGR2GRAY);
    }
    /// convert BGR image to single channel chroma red extracted from YCrCb
    static void ConvertChromaRed(const cv::Mat& image, cv::Mat& outImage)
    {
        static cv::Mat tempImage;
        cv::cvtColor(image, tempImage, cv::COLOR_BGR2YCrCb);
        constexpr int chromaRedChannel = 1;
        cv::extractChannel(tempImage, outImage, chromaRedChannel);
    }

    void DetectMarkers(cv::Mat& frame, MarkerDetectionList& outList);

    std::vector<std::string> GetTimeProfile();
    void DrawTimeProfile(cv::Mat& image, const cv::Point2d& textOrigin);

private:
    apriltag_detector* mDetector; /// owning
    MarkerFamily mFamilyType;
    apriltag_family* mFamilyPtr = nullptr; /// owning
};
