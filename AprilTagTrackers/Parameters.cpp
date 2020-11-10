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
		fs["smoothingFactor"] >> smoothingFactor;
		cv::FileNode fn = fs["trackers"];
		if (!fn.empty())		//load all saved markers
		{
			cv::FileNodeIterator curMarker = fn.begin(), it_end = fn.end();
			for (; curMarker != it_end; ++curMarker)
			{
				std::vector<std::vector<cv::Point3f>> boardCorners;
				std::vector<int> boardIds;
				cv::FileNode item = *curMarker;
				item["trackerIds"] >> boardIds;
				//data.push_back(tmp);
				cv::FileNode fnCorners = item["trackerCorners"];
				if (!fnCorners.empty())
				{
					cv::FileNodeIterator curCorners = fnCorners.begin(), itCorners_end = fnCorners.end();
					for (; curCorners != itCorners_end; ++curCorners)
					{
						std::vector<cv::Point3f> corners;
						cv::FileNode item2 = *curCorners;
						item2 >> corners;
						boardCorners.push_back(corners);
					}
				}
				cv::Ptr<cv::aruco::Board> arBoard = cv::aruco::Board::create(boardCorners, cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50), boardIds);
				trackers.push_back(arBoard);
			}
		}
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
	fs << "smoothingFactor" << smoothingFactor;
	fs << "trackers";
	fs << "{";
	for (int i = 0; i < trackers.size(); i++)
	{
		fs << "tracker_" + std::to_string(i);
		fs << "{";
		fs << "trackerIds";
		fs << trackers[i]->ids;
		fs << "trackerCorners";
		fs << "{";
		for (int j = 0; j < trackers[i]->objPoints.size(); j++)
		{
			fs << "trackerCorners_" + std::to_string(j);
			fs << trackers[i]->objPoints[j];
		}
		fs << "}";
		fs << "}";
	}
	fs << "}";
}