#include "AprilTagWrapper.hpp"

#include "tagCustom29h10.hpp"
#include "utils/Assert.hpp"
#include "utils/Cross.hpp"

#include <apriltag/apriltag.h>
#include <apriltag/tagCircle21h7.h>
#include <apriltag/tagStandard41h12.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

AprilTagWrapper::AprilTagWrapper(MarkerFamily family, double quadDecimate, int threadCount)
    : mDetector(apriltag_detector_create()), mFamilyType(family)
{
    mDetector->quad_decimate = static_cast<float>(quadDecimate);
    mDetector->nthreads = threadCount;
    if (mFamilyType == MarkerFamily::Standard41h12)
    {
        mFamilyPtr = tagStandard41h12_create();
    }
    else if (mFamilyType == MarkerFamily::Circle21h7)
    {
        mFamilyPtr = tagCircle21h7_create();
    }
    else if (mFamilyType == MarkerFamily::Custom29h10)
    {
        mFamilyPtr = tagCustom29h10_create();
    }
    else
    {
        utils::Unreachable();
    }
    apriltag_detector_add_family(mDetector, mFamilyPtr);
}

AprilTagWrapper::~AprilTagWrapper()
{
    if (mDetector != nullptr)
    {
        apriltag_detector_destroy(mDetector);
        mDetector = nullptr;
    }
    if (mFamilyPtr != nullptr)
    {
        if (mFamilyType == MarkerFamily::Standard41h12)
        {
            tagStandard41h12_destroy(mFamilyPtr);
        }
        else if (mFamilyType == MarkerFamily::Circle21h7)
        {
            tagCircle21h7_destroy(mFamilyPtr);
        }
        else if (mFamilyType == MarkerFamily::Custom29h10)
        {
            tagCustom29h10_destroy(mFamilyPtr);
        }
        else
        {
            utils::Unreachable();
        }
        mFamilyPtr = nullptr;
    }
}

void AprilTagWrapper::DetectMarkers(cv::Mat& frame, MarkerDetectionList& outList)
{
    ATT_ASSERT(frame.type() == CV_8U);
    image_u8_t frameRef{
        frame.cols,
        frame.rows,
        frame.cols, // stride = bytes per line
        frame.data};

    zarray_t* const detections = apriltag_detector_detect(mDetector, &frameRef);
    const int size = zarray_size(detections);
    outList.ids.resize(size);
    outList.corners.resize(size);
    outList.centers.resize(size);

    for (int i = 0; i < size; ++i)
    {
        const apriltag_detection_t* det = nullptr;
        zarray_get_volatile(detections, i, &det);
        ATT_ASSERT(det != nullptr);

        outList.ids[i] = det->id;
        outList.centers[i] = cv::Point2d(det->c[0], det->c[1]);

        for (int cornerIdx = 0; cornerIdx < 4; ++cornerIdx)
        {
            /// apriltag returns CW order, while we need CCW for opencv
            const int detCornerIdx = 3 - cornerIdx;
            outList.corners[i][cornerIdx] = cv::Point2d(
                det->p[detCornerIdx][0], det->p[detCornerIdx][1]);
        }
    }
    apriltag_detections_destroy(detections);
}

std::vector<std::string> AprilTagWrapper::GetTimeProfile()
{
    const timeprofile_t* const tp = mDetector->tp;
    if (tp == nullptr) return {};

    std::vector<std::string> rows;
    int64_t lastutime = tp->utime;

    for (int i = 0; i < zarray_size(tp->stamps); i++)
    {
        timeprofile_entry* stamp = nullptr;
        zarray_get_volatile(tp->stamps, i, &stamp);
        const double cumtime = static_cast<double>(stamp->utime - tp->utime) / 1000000.0;
        const double parttime = static_cast<double>(stamp->utime - lastutime) / 1000000.0;
        std::array<char, 128> buffer{};
        std::snprintf(buffer.data(), buffer.size(), "%2d %32s %15f ms %15f ms", i, stamp->name, parttime * 1000, cumtime * 1000);
        rows.emplace_back(buffer.data());
        lastutime = stamp->utime;
    }
    return rows;
}

void AprilTagWrapper::DrawTimeProfile(cv::Mat& image, const cv::Point2d& textOrigin)
{
    const auto font = cv::FONT_HERSHEY_SIMPLEX;
    const auto color = cv::Scalar(255, 255, 255);
    const auto fontScale = 0.5;
    const int textHeight = cv::getTextSize(" ", font, fontScale, 1, nullptr).height;
    const int dy = static_cast<int>(1.5 * textHeight);
    cv::Point cursor = textOrigin;
    for (const auto& row : GetTimeProfile())
    {
        cv::putText(image, row, cursor, font, fontScale, color);
        cursor.y += dy;
    }
}
