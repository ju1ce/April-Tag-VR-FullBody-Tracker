#pragma once
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/aruco.hpp>
#include "Quaternion.h"

class Parameters
{
public:
	Parameters();
	void Load();
	void Save();
	std::string cameraAddr = "0";
	cv::Mat camMat;
	cv::Mat distCoefs;
	std::vector<cv::Ptr<cv::aruco::Board>> trackers;
	int trackerNum = 1;
	double markerSize = 0.05;
	int numOfPrevValues = 5;
	double quadDecimate = 1;
	double searchWindow = 0.25;
	bool usePredictive = true;
	int calibrationTracker = 0;
	bool ignoreTracker0 = false;
	bool rotateCl = false;
	bool rotateCounterCl = false;
	double calibOffsetX = 0;
	double calibOffsetY = 100;
	double calibOffsetZ = 100;
	bool circularWindow = true;
	double smoothingFactor = 0.7;
	int camFps = 30;
	int camHeight = 0;
	int camWidth = 0;
	cv::Mat wtranslation;
	Quaternion<double> wrotation;
	bool cameraSettings = false;
};