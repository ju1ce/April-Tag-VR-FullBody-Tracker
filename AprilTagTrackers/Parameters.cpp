#include "Parameters.h"

Parameters::Parameters()
{
	Load();
}

void Parameters::Load()
{
	cv::FileStorage fs("params.yml", cv::FileStorage::READ);		//open test.yml file to read parameters

	if (!fs["cameraAddr"].empty())			//if file exists, load all parameters from file into variables
	{
		fs["cameraAddr"] >> cameraAddr;
		fs["cameraMatrix"] >> camMat;
		fs["distortionCoeffs"] >> distCoefs;
		fs["trackerNum"] >> trackerNum;
		fs["markerSize"] >> markerSize;
		fs["numOfPrevValues"] >> numOfPrevValues;
		fs["quadDecimate"] >> quadDecimate;
		fs["searchWindow"] >> searchWindow;
		fs["usePredictive"] >> usePredictive;
		fs["calibrationTracker"] >> calibrationTracker;
		fs["rotate"] >> rotate;
		fs["calibOffsetX"] >> calibOffsetX;
		fs["calibOffsetY"] >> calibOffsetY;
		fs["calibOffsetZ"] >> calibOffsetZ;
		fs["circularWindow"] >> circularWindow;
	}
	fs.release();
}

void Parameters::Save()
{
	cv::FileStorage fs("params.yml", cv::FileStorage::WRITE);

	fs << "cameraAddr" << cameraAddr;
	fs << "cameraMatrix" << camMat;
	fs << "distortionCoeffs" << distCoefs;
	fs << "trackerNum" << trackerNum;
	fs << "markerSize" << markerSize;
	fs << "numOfPrevValues" << numOfPrevValues;
	fs << "quadDecimate" << quadDecimate;
	fs << "searchWindow" << searchWindow;
	fs << "usePredictive" << usePredictive;
	fs << "calibrationTracker" << calibrationTracker;
	fs << "rotate" << rotate;
	fs << "calibOffsetX" << calibOffsetX;
	fs << "calibOffsetY" << calibOffsetY;
	fs << "calibOffsetZ" << calibOffsetZ;
	fs << "circularWindow" << circularWindow;
}