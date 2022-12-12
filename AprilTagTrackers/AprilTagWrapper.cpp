#include "AprilTagWrapper.hpp"

#include "tagCustom29h10.hpp"

#include <apriltag/apriltag.h>
#include <apriltag/tagCircle21h7.h>
#include <apriltag/tagStandard41h12.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

AprilTagWrapper::AprilTagWrapper(const UserConfig& user_config, const ArucoConfig& aruco_config)
    : td{apriltag_detector_create()}, user_config(user_config), aruco_config(aruco_config)
{
    td->quad_decimate = user_config.videoStreams[0]->quadDecimate;
    td->nthreads = 1; // TODO: make nthreads a parameter or calculate it somehow
    apriltag_family_t* tf;
    if (user_config.markerLibrary == APRILTAG_CIRCULAR)
        tf = tagCircle21h7_create();
    else if (user_config.markerLibrary == APRILTAG_CUSTOM29H10)
        tf = tagCustom29h10_create();
    else
        tf = tagStandard41h12_create();

    if (user_config.markerLibrary == ARUCO_4X4)
    {
        aruco_dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_250);
    }

    apriltag_detector_add_family(td, tf);
}

AprilTagWrapper::~AprilTagWrapper()
{
    apriltag_detector_destroy(td);
}

void AprilTagWrapper::convertToSingleChannel(const cv::Mat& src, cv::Mat& dst)
{
    if (user_config.markerLibrary == APRILTAG_COLOR)
    {
        cv::Mat ycc;
        cv::cvtColor(src, ycc, cv::COLOR_BGR2YCrCb);
        dst.create(src.size(), CV_8U);
        int fromTo[] = {1, 0};
        cv::mixChannels(&ycc, 1, &dst, 1, fromTo, 1);
    }
    else
    {
        cv::cvtColor(src, dst, cv::COLOR_BGR2GRAY);
    }
}

void AprilTagWrapper::detectMarkers(
    const cv::Mat& image,
    std::vector<std::vector<cv::Point2f>>* corners,
    std::vector<int>* ids,
    std::vector<cv::Point2f>* centers,
    std::vector<cv::Ptr<cv::aruco::Board>> trackers)
{
    cv::Mat gray;
    if (image.type() != CV_8U)
    {
        convertToSingleChannel(image, gray);
    }
    else
    {
        gray = image;
    }

    corners->clear();
    ids->clear();
    centers->clear();

    if (user_config.markerLibrary == ARUCO_4X4)
    {
        std::vector<std::vector<cv::Point2f>> rejectedCorners;
        cv::aruco::detectMarkers(gray, aruco_dictionary, *corners, *ids, aruco_config.params, rejectedCorners);
        // for(int i = 0; i<trackers.size();i++)
        //     cv::aruco::refineDetectedMarkers(gray, trackers[i], *corners, *ids, rejectedCorners);

        return;
    }

    image_u8_t im = {
        gray.cols,
        gray.rows,
        static_cast<int32_t>(gray.step1()),
        gray.data,
    };

    zarray_t* detections = apriltag_detector_detect(td, &im);

    for (int i = 0; i < zarray_size(detections); i++)
    {
        apriltag_detection_t* det;
        zarray_get(detections, i, &det);

        ids->push_back(det->id);
        centers->push_back(cv::Point2f(det->c[0], det->c[1]));

        std::vector<cv::Point2f> temp;

        for (int j = 3; j >= 0; j--)
        {
            temp.push_back(cv::Point2f(det->p[j][0], det->p[j][1]));
        }

        corners->push_back(temp);
    }
    apriltag_detections_destroy(detections);
}

std::vector<std::string> AprilTagWrapper::getTimeProfile()
{
    const auto tp = td->tp;
    if (tp == nullptr)
    {
        return {};
    }

    std::vector<std::string> rows;
    int64_t lastutime = tp->utime;

    for (int i = 0; i < zarray_size(tp->stamps); i++)
    {
        timeprofile_entry* stamp;
        zarray_get_volatile(tp->stamps, i, &stamp);
        double cumtime = (stamp->utime - tp->utime) / 1000000.0;
        double parttime = (stamp->utime - lastutime) / 1000000.0;
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "%2d %32s %15f ms %15f ms", i, stamp->name, parttime * 1000, cumtime * 1000);
        rows.push_back(buffer);
        lastutime = stamp->utime;
    }
    return rows;
}

void AprilTagWrapper::drawTimeProfile(cv::Mat& image, const cv::Point& textOrigin)
{
    const auto font = cv::FONT_HERSHEY_SIMPLEX;
    const auto color = cv::Scalar(255, 255, 255);
    const auto fontScale = 0.5;
    const int dy = cv::getTextSize(" ", font, fontScale, 1, nullptr).height * 1.5;
    cv::Point cursor = textOrigin;
    for (const auto& row : getTimeProfile())
    {
        cv::putText(image, row, cursor, font, fontScale, color);
        cursor.y += dy;
    }
}
