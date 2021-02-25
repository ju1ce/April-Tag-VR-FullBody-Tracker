#pragma once
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <wx/wx.h>

#include "Quaternion.h"

void drawMarker(cv::Mat, std::vector<cv::Point2f>, cv::Scalar);
void transformMarkerSpace(std::vector<cv::Point3f>, cv::Vec3d, cv::Vec3d, cv::Vec3d, cv::Vec3d, std::vector<cv::Point3f>*);
void getMedianMarker(std::vector<std::vector<cv::Point3f>>, std::vector<cv::Point3f>*);
Quaternion<double> rodr2quat(double, double, double);
cv::Mat getSpaceCalib(cv::Vec3d, cv::Vec3d, double, double, double);
cv::Mat eulerAnglesToRotationMatrix(cv::Vec3f& theta);
bool isRotationMatrix(cv::Mat& R);
cv::Vec3f rotationMatrixToEulerAngles(cv::Mat& R);
Quaternion<double> mRot2Quat(const cv::Mat& m);
cv::Mat getSpaceCalibEuler(cv::Vec3d rvec, cv::Vec3d tvec, double xOffset, double yOffset, double zOffset);
