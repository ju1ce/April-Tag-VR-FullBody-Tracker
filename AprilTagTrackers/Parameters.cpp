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
        fs["cameraApiPreference"] >> cameraApiPreference;
        fs["camFps"] >> camFps;
        fs["camHeight"] >> camHeight;
        fs["camWidth"] >> camWidth;
        fs["cameraMatrix"] >> camMat;
        fs["distortionCoeffs"] >> distCoeffs;
        fs["stdDeviationsIntrinsics"] >> stdDeviationsIntrinsics;
        fs["perViewErrors"] >> perViewErrors;
        fs["allCharucoCorners"] >> allCharucoCorners;
        fs["allCharucoIds"] >> allCharucoIds;
        fs["trackerNum"] >> trackerNum;
        fs["markerSize"] >> markerSize;
        fs["numOfPrevValues"] >> numOfPrevValues;
        fs["quadDecimate"] >> quadDecimate;
        fs["searchWindow"] >> searchWindow;
        fs["usePredictive"] >> usePredictive;
        fs["calibrationTracker"] >> calibrationTracker;
        fs["rotateCl"] >> rotateCl;
        fs["rotateCounterCl"] >> rotateCounterCl;
        fs["calibOffsetX"] >> calibOffsetX;
        fs["calibOffsetY"] >> calibOffsetY;
        fs["calibOffsetZ"] >> calibOffsetZ;
        fs["calibOffsetA"] >> calibOffsetA;
        fs["calibOffsetB"] >> calibOffsetB;
        fs["calibOffsetC"] >> calibOffsetC;
        fs["circularWindow"] >> circularWindow;
        fs["smoothingFactor"] >> smoothingFactor;
        fs["ignoreTracker0"] >> ignoreTracker0;
        fs["wtranslation"] >> wtranslation;
        cv::Mat wrotmat;
        fs["wrotation"] >> wrotmat;
        fs["cameraSettings"] >> cameraSettings;
        fs["chessboardCalib"] >> chessboardCalib;
        fs["camLatency"] >> camLatency;
        fs["circularMarkers"] >> circularMarkers;
        fs["trackerCalibDistance"] >> trackerCalibDistance;
        if (trackerCalibDistance < 0.5)
            trackerCalibDistance = 0.5;
        fs["cameraCalibSamples"] >> cameraCalibSamples;
        if (cameraCalibSamples < 15)
            cameraCalibSamples = 15;
        fs["settingsParameters"] >> settingsParameters;
        fs["cameraAutoexposure"] >> cameraAutoexposure;
        fs["cameraExposure"] >> cameraExposure;
        fs["cameraGain"] >> cameraGain;
        if(!wrotmat.empty())
            wrotation = Quaternion<double>(wrotmat.at<double>(0), wrotmat.at<double>(1), wrotmat.at<double>(2), wrotmat.at<double>(3));
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
    fs << "cameraApiPreference" << cameraApiPreference;
    fs << "camFps" << camFps;
    fs << "camHeight" << camHeight;
    fs << "camWidth" << camWidth;
    fs << "cameraMatrix" << camMat;
    fs << "distortionCoeffs" << distCoeffs;
    fs << "stdDeviationsIntrinsics" << stdDeviationsIntrinsics;
    fs << "perViewErrors" << perViewErrors;
    fs << "allCharucoCorners" << allCharucoCorners;
    fs << "allCharucoIds" << allCharucoIds;
    fs << "trackerNum" << trackerNum;
    fs << "markerSize" << markerSize;
    fs << "numOfPrevValues" << numOfPrevValues;
    fs << "quadDecimate" << quadDecimate;
    fs << "searchWindow" << searchWindow;
    fs << "usePredictive" << usePredictive;
    fs << "calibrationTracker" << calibrationTracker;
    fs << "rotateCl" << rotateCl;
    fs << "rotateCounterCl" << rotateCounterCl;
    fs << "calibOffsetX" << calibOffsetX;
    fs << "calibOffsetY" << calibOffsetY;
    fs << "calibOffsetZ" << calibOffsetZ;
    fs << "calibOffsetA" << calibOffsetA;
    fs << "calibOffsetB" << calibOffsetB;
    fs << "calibOffsetC" << calibOffsetC;
    fs << "circularWindow" << circularWindow;
    fs << "smoothingFactor" << smoothingFactor;
    fs << "ignoreTracker0" << ignoreTracker0;
    fs << "wtranslation" << wtranslation;
    fs <<"wrotation" << (cv::Mat_<double>(4,1) << wrotation.w,wrotation.x,wrotation.y,wrotation.z);
    fs << "cameraSettings" << cameraSettings;
    fs << "chessboardCalib" << chessboardCalib;
    fs << "camLatency" << camLatency;
    fs << "circularMarkers" << circularMarkers;
    fs << "trackerCalibDistance" << trackerCalibDistance;
    fs << "cameraCalibSamples" << cameraCalibSamples;
    fs << "settingsParameters" << settingsParameters;
    fs << "cameraAutoexposure" << cameraAutoexposure;
    fs << "cameraExposure" << cameraExposure;
    fs << "cameraGain" << cameraGain;
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
