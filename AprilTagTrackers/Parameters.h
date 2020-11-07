#pragma once
#include <iostream>
#include <opencv2/core.hpp>

class Parameters
{
public:
	Parameters();
	void Load();
	void Save();
	std::string cameraAddr = "0";
	cv::Mat camMat;
	cv::Mat distCoefs;
	int trackerNum = 1;
	double markerSize = 0;
	int numOfPrevValues = 10;
	double quadDecimate = 1;
	double searchWindow = 0.25;
	bool usePredictive = true;
	int calibrationTracker = 0;
	bool rotate = true;
	double calibOffsetX = 0;
	double calibOffsetY = 1;
	double calibOffsetZ = 0;
	bool circularWindow = false;
};