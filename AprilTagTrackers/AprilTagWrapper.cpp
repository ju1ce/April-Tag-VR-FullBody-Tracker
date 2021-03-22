#include "AprilTagWrapper.h"

#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <apriltag.h>
#include <tagStandard41h12.h>
#include <tagCircle21h7.h>

#include "Parameters.h"

AprilTagWrapper::AprilTagWrapper(const Parameters* params)
    : td{apriltag_detector_create()}
    , parameters(params)
{
    td->quad_decimate = parameters->quadDecimate;
    apriltag_family_t* tf;
    if (!parameters->circularMarkers)
        tf = tagStandard41h12_create();
    else
        tf = tagCircle21h7_create();
    apriltag_detector_add_family(td, tf);
}

AprilTagWrapper::~AprilTagWrapper()
{
    apriltag_detector_destroy(td);
}

void AprilTagWrapper::detectMarkers(
    const cv::Mat& frame,
    std::vector<std::vector<cv::Point2f> >* corners,
    std::vector<int>* ids,
    std::vector<cv::Point2f>* centers)
{
    cv::Mat gray;
    if (frame.type() != CV_8U)
    {
        cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    }
    else
    {
        gray = frame;
    }

    corners->clear();
    ids->clear();
    centers->clear();

    image_u8_t im = {
        gray.cols,
        gray.rows,
        static_cast<int32_t>(gray.step1()),
        gray.data,
    };

    zarray_t* detections = apriltag_detector_detect(td, &im);

    for (int i = 0; i < zarray_size(detections); i++) {
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
