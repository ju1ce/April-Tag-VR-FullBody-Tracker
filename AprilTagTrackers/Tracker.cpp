#include <iostream>
#include <vector>

#include <wx/wx.h>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <thread>

#include <apriltag.h>
#include <tagStandard41h12.h>
#include <tagCircle21h7.h>

#include "Connection.h"
#include "GUI.h"
#include "Helpers.h"
#include "Parameters.h"
#include "Tracker.h"

Tracker::Tracker(Parameters* params, Connection* conn)
{
    parameters = params;
    connection = conn;
    if (!parameters->trackers.empty())
    {
        trackers = parameters->trackers;
        trackersCalibrated = true;
    }
    if (!parameters->wtranslation.empty())
    {
        wtranslation = parameters->wtranslation;
        wrotation = parameters->wrotation;
    }
}

void Tracker::StartCamera(std::string id)
{
    if (cameraRunning)
    {
        cameraRunning = false;
        //cameraThread.join();
        Sleep(1000);
        return;
    }
    if (id.length() <= 2)		//if camera address is a single character, try to open webcam
    {
        int i = std::stoi(id);	//convert to int
        cap = cv::VideoCapture(i);
    }
    else
    {			//if address is longer, we try to open it as an ip address
        cap = cv::VideoCapture(id);
    }

    if (!cap.isOpened())
    {
        wxMessageDialog* dial = new wxMessageDialog(NULL,
            wxT("Could not start camera. Make sure you entered the correct ID or IP of your camera in the params.\n"
            "For USB cameras, it will be a number, usually 0,1,2... try a few until it works.\n"
            "For IP webcam, the address will be in the format http://'ip - here':8080/video"), wxT("Error"), wxOK | wxICON_ERROR);
        dial->ShowModal();
        return;
    }

    if(parameters->camWidth != 0)
        cap.set(cv::CAP_PROP_FRAME_WIDTH, parameters->camWidth);
    if (parameters->camHeight != 0)
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, parameters->camHeight);
    cap.set(cv::CAP_PROP_FPS, parameters->camFps);
    if(parameters->cameraSettings)
        cap.set(cv::CAP_PROP_SETTINGS, 1);
    cameraRunning = true;
    cameraThread = std::thread(&Tracker::CameraLoop, this);
    cameraThread.detach();
}

void Tracker::CameraLoop()
{
    cv::Mat img;
    bool rotate = false;
    int rotateFlag = -1;
    if (!cap.read(img))
    {
        wxMessageDialog* dial = new wxMessageDialog(NULL,
            wxT("Camera error"), wxT("Error"), wxOK | wxICON_ERROR);
        dial->ShowModal();
        cameraRunning = false;
        cap.release();
        return;
    }
    if (parameters->rotateCl && parameters->rotateCounterCl)
    {
        cv::rotate(img, img, cv::ROTATE_180);
        rotate = true;
        rotateFlag = cv::ROTATE_180;
    }
    else if (parameters->rotateCl)
    {
        cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);
        rotate = true;
        rotateFlag = cv::ROTATE_90_CLOCKWISE;
    }
    else if (parameters->rotateCounterCl)
    {
        cv::rotate(img, img, cv::ROTATE_90_COUNTERCLOCKWISE);
        rotate = true;
        rotateFlag = cv::ROTATE_90_COUNTERCLOCKWISE;
    }
    while (cameraRunning)
    {
        if (!cap.read(img))
        {
            wxMessageDialog* dial = new wxMessageDialog(NULL,
                wxT("Camera error"), wxT("Error"), wxOK | wxICON_ERROR);
            dial->ShowModal();
            cameraRunning = false;
            break;
        }
        if (rotate)
            cv::rotate(img, img, rotateFlag);
        img.copyTo(retImage);
        imageReady = true;
        last_frame_time = clock();
        if (previewCamera)
        {
            cv::imshow("Preview", img);
            cv::waitKey(1);
        }
        else
        {
            cv::destroyWindow("Preview");
        }
    }
    cv::destroyAllWindows();
    cap.release();
}

void Tracker::StartCameraCalib()
{
    if (mainThreadRunning)
    {
        mainThreadRunning = false;
        return;
    }
    if (!cameraRunning)
    {
        wxMessageDialog* dial = new wxMessageDialog(NULL,
            wxT("Camera not running"), wxT("Error"), wxOK | wxICON_ERROR);
        dial->ShowModal();
        mainThreadRunning = false;
        return;
    }

    mainThreadRunning = true;
    if(!parameters->chessboardCalib)
        mainThread = std::thread(&Tracker::CalibrateCameraCharuco, this);
    else
        mainThread = std::thread(&Tracker::CalibrateCamera, this);
    mainThread.detach();
}

void Tracker::CalibrateCameraCharuco()
{
    //function to calibrate our camera

    bool success;

    cv::Mat image;

    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
    cv::Ptr<cv::aruco::DetectorParameters> params = cv::aruco::DetectorParameters::create();

    //generate and show our charuco board that will be used for calibration
    cv::Ptr<cv::aruco::CharucoBoard> board = cv::aruco::CharucoBoard::create(8, 7, 0.04f, 0.02f, dictionary);
    cv::Mat boardImage;
    //board->draw(cv::Size(1500, 1000), boardImage, 10, 1);
    //imshow("calibration", boardImage);
    //cv::imwrite("charuco_board.jpg", boardImage);
    //cv::waitKey(1);

    std::vector<std::vector<cv::Point2f>> allCharucoCorners;
    std::vector<std::vector<int>> allCharucoIds;

    //set our detectors marker border bits to 1 since thats what charuco uses
    params->markerBorderBits = 1;

    int framesSinceLast = -2 * parameters->camFps;

    int i = 0;

    std::thread th{ [=]() {
        wxMessageDialog* dial = new wxMessageDialog(NULL,
        "Camera calibration started! \n\n"
        "Place the printed Charuco calibration board on a flat surface. The camera will take a picture every second - take pictures of the board from as many diffrent angles and distances as you can. \n\n"
        "Alternatively, you can use the board shown on a monitor or switch to old chessboard calibration in params, but both will have worse results or might not work at all. \n\n"
        "Press OK to close this window.", wxT("Message"), wxOK);
    dial->ShowModal();

    mainThreadRunning = false;

    } };

    th.detach();

    //get calibration data from 20 images

    int picNum = parameters->cameraCalibSamples;

    while (i < picNum)
    {
        if (!mainThreadRunning || !cameraRunning)
        {
            cv::destroyAllWindows();
            return;
        }
        while (!imageReady)
            Sleep(1);
        imageReady = false;
        retImage.copyTo(image);
        cv::putText(image, std::to_string(i) + "/" + std::to_string(picNum), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
        cv::Mat drawImg;
        int cols, rows;
        if (image.cols > image.rows)
        {
            cols = image.cols * drawImgSize / image.rows;
            rows = drawImgSize;
        }
        else
        {
            cols = drawImgSize;
            rows = image.rows * drawImgSize / image.cols;
        }

        //std::vector<int> markerIds;
        //std::vector<std::vector<cv::Point2f>> markerCorners;

        //cv::aruco::detectMarkers(image, dictionary, markerCorners, markerIds, params);
        //cv::aruco::drawDetectedMarkers(image, markerCorners, markerIds);

        cv::resize(image, drawImg, cv::Size(cols, rows));

        cv::imshow("out", drawImg);
        char key = (char)cv::waitKey(1);

        //continue;

        framesSinceLast++;
        if (key != -1 || framesSinceLast > parameters->camFps)
        {
            framesSinceLast = 0;
            //if any button was pressed
            cvtColor(image, image, cv::COLOR_BGR2GRAY);

            std::vector<int> markerIds;
            std::vector<std::vector<cv::Point2f>> markerCorners;

            //detect our markers
            cv::aruco::detectMarkers(image, dictionary, markerCorners, markerIds, params);

            if (markerIds.size() > 0)
            {
                //if markers were found, try to add calibration data
                std::vector<cv::Point2f> charucoCorners;
                std::vector<int> charucoIds;
                //using data from aruco detection we refine the search of chessboard corners for higher accuracy
                cv::aruco::interpolateCornersCharuco(markerCorners, markerIds, image, board, charucoCorners, charucoIds);
                if (charucoIds.size() > 15)
                {
                    //if corners were found, we draw them
                    cv::aruco::drawDetectedCornersCharuco(image, charucoCorners, charucoIds);
                    //we then add our corners to the array
                    allCharucoCorners.push_back(charucoCorners);
                    allCharucoIds.push_back(charucoIds);
                    i++;
                }
            }
            cv::resize(image, drawImg, cv::Size(cols, rows));
            cv::imshow("out", drawImg);
            cv::waitKey(1000);
        }
    }
    cv::Mat cameraMatrix, distCoeffs, R, T;

    //calibrate camera using our data and save to our global params cameraMatrix and distCoeffs
    cv::aruco::calibrateCameraCharuco(allCharucoCorners, allCharucoIds, board, cv::Size(image.rows, image.cols), cameraMatrix, distCoeffs, R, T, 0);

    parameters->camMat = cameraMatrix;
    parameters->distCoefs = distCoeffs;
    parameters->Save();
    mainThreadRunning = false;
    cv::destroyAllWindows();
    wxMessageDialog* dial = new wxMessageDialog(NULL,
        wxT("Calibration complete."), wxT("Info"), wxOK);
    dial->ShowModal();
}

void Tracker::CalibrateCamera()
{

    int CHECKERBOARD[2]{ 7,7 };

    int blockSize = 125;
    int imageSizeX = blockSize * (CHECKERBOARD[0] + 1);
    int imageSizeY = blockSize * (CHECKERBOARD[1] + 1);
    cv::Mat chessBoard(imageSizeX, imageSizeY, CV_8UC3, cv::Scalar::all(0));
    unsigned char color = 0;

    for (int i = 0; i < imageSizeX-1; i = i + blockSize) {
        if(CHECKERBOARD[1]%2 == 1)
            color = ~color;
        for (int j = 0; j < imageSizeY-1; j = j + blockSize) {
            cv::Mat ROI = chessBoard(cv::Rect(j, i, blockSize, blockSize));
            ROI.setTo(cv::Scalar::all(color));
            color = ~color;
        }
    }
    //cv::namedWindow("Chessboard", cv::WINDOW_KEEPRATIO);
    //imshow("Chessboard", chessBoard);
    //cv::imwrite("chessboard.png", chessBoard);

    std::vector<std::vector<cv::Point3f>> objpoints;
    std::vector<std::vector<cv::Point2f>> imgpoints;
    std::vector<cv::Point3f> objp;

    for (int i{ 0 }; i < CHECKERBOARD[0]; i++)
    {
        for (int j{ 0 }; j < CHECKERBOARD[1]; j++)
        {
            objp.push_back(cv::Point3f(j, i, 0));
        }
    }

    std::vector<cv::Point2f> corner_pts;
    bool success;

    cv::Mat image;

    int i = 0;
    int framesSinceLast = -100;

    int picNum = parameters->cameraCalibSamples;

    while (i < picNum)
    {
        if (!mainThreadRunning || !cameraRunning)
        {
            cv::destroyAllWindows();
            return;
        }
        while (!imageReady)
            Sleep(1);
        imageReady = false;
        retImage.copyTo(image);
        cv::putText(image, std::to_string(i) + "/" + std::to_string(picNum), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
        cv::Mat drawImg;
        int cols, rows;
        if (image.cols > image.rows)
        {
            cols = image.cols * drawImgSize / image.rows;
            rows = drawImgSize;
        }
        else
        {
            cols = drawImgSize;
            rows = image.rows * drawImgSize / image.cols;
        }
        cv::resize(image, drawImg, cv::Size(cols,rows));
        cv::imshow("out", drawImg);
        char key = (char)cv::waitKey(1);
        framesSinceLast++;
        if (key != -1 || framesSinceLast > 50)
        {
            framesSinceLast = 0;
            cv::cvtColor(image, image,cv:: COLOR_BGR2GRAY);

            success = findChessboardCorners(image, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts);

            if (success)
            {
                i++;
                cv::TermCriteria criteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.001);

                cornerSubPix(image, corner_pts, cv::Size(11, 11), cv::Size(-1, -1), criteria);

                drawChessboardCorners(image, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, success);

                objpoints.push_back(objp);
                imgpoints.push_back(corner_pts);
            }

            cv::resize(image, drawImg, cv::Size(cols, rows));
            cv::imshow("out", drawImg);
            cv::waitKey(1000);
        }
    }

    cv::Mat cameraMatrix, distCoeffs, R, T;

    calibrateCamera(objpoints, imgpoints, cv::Size(image.rows, image.cols), cameraMatrix, distCoeffs, R, T);

    parameters->camMat = cameraMatrix;
    parameters->distCoefs = distCoeffs;
    parameters->Save();
    mainThreadRunning = false;
    cv::destroyAllWindows();
    wxMessageDialog* dial = new wxMessageDialog(NULL,
        wxT("Calibration complete."), wxT("Info"), wxOK);
    dial->ShowModal();
}

void Tracker::StartTrackerCalib()
{
    if (mainThreadRunning)
    {
        mainThreadRunning = false;
        return;
    }
    if (!cameraRunning)
    {
        wxMessageDialog* dial = new wxMessageDialog(NULL,
            wxT("Camera not running"), wxT("Error"), wxOK | wxICON_ERROR);
        dial->ShowModal();
        mainThreadRunning = false;
        return;
    }
    if (parameters->camMat.empty())
    {
        wxMessageDialog* dial = new wxMessageDialog(NULL,
            wxT("Camera not calibrated"), wxT("Error"), wxOK | wxICON_ERROR);
        dial->ShowModal();
        mainThreadRunning = false;
        return;
    }

    mainThreadRunning = true;
    mainThread = std::thread(&Tracker::CalibrateTracker, this);
    mainThread.detach();


    //make a new thread with message box, and stop main thread when we press OK
    std::thread th{ [=]() {
        wxMessageDialog* dial = new wxMessageDialog(NULL,
        "Tracker calibration started! \n\nBefore calibrating, set the number of trackers and marker size parameters (measure the white square). Make sure the trackers are completely rigid and cannot bend,"
        "neither the markers or at the connections between markers - use images on github for reference. Wear your trackers, then calibrate them by moving them to the camera closer than 30cm \n\n"
        "Green: This marker is calibrated and can be used to calibrate other markers.\n"
        "Blue: This marker is not part of any used trackers. You probably have to increase number of trackers in params.\n"
        "Purple: This marker is too far from the camera to be calibrated. Move it closer than 30cm.\n"
        "Red: This marker cannot be calibrated as no green markers are seen. Rotate the tracker until a green marker is seen along this one.\n"
        "Yellow: The marker is being calibrated. Hold it still for a second.\n\n"
        "When all the markers on all trackers are shown as green, press OK to finish calibration.", wxT("Message"), wxOK);
    dial->ShowModal();

    mainThreadRunning = false;

    } };

    th.detach();
}

void Tracker::Start()
{
    if (mainThreadRunning)
    {
        mainThreadRunning = false;
        return;
    }
    if (!cameraRunning)
    {
        wxMessageDialog* dial = new wxMessageDialog(NULL,
            wxT("Camera not running"), wxT("Error"), wxOK | wxICON_ERROR);
        dial->ShowModal();
        mainThreadRunning = false;
        return;
    }
    if (parameters->camMat.empty())
    {
        wxMessageDialog* dial = new wxMessageDialog(NULL,
            wxT("Camera not calibrated"), wxT("Error"), wxOK | wxICON_ERROR);
        dial->ShowModal();
        mainThreadRunning = false;
        return;
    }
    if (!trackersCalibrated)
    {
        wxMessageDialog* dial = new wxMessageDialog(NULL,
            wxT("Trackers not calibrated"), wxT("Error"), wxOK | wxICON_ERROR);
        dial->ShowModal();
        mainThreadRunning = false;
        return;
    }
    if (connection->status != connection->CONNECTED)
    {
        wxMessageDialog* dial = new wxMessageDialog(NULL,
            wxT("Not connected to steamVR"), wxT("Error"), wxOK | wxICON_ERROR);
        dial->ShowModal();
        mainThreadRunning = false;
        //return;
    }
    mainThreadRunning = true;
    mainThread = std::thread(&Tracker::MainLoop, this);
    mainThread.detach();
}

void Tracker::CalibrateTracker()
{
    std::vector<std::vector<int>> boardIds;
    std::vector<std::vector < std::vector<cv::Point3f >>> boardCorners;
    std::vector<bool> boardFound;

    //making a marker model of our markersize for later use
    std::vector<cv::Point3f> modelMarker;
    double markerSize = parameters->markerSize;
    modelMarker.push_back(cv::Point3f(-markerSize / 2, markerSize / 2, 0));
    modelMarker.push_back(cv::Point3f(markerSize / 2, markerSize / 2, 0));
    modelMarker.push_back(cv::Point3f(markerSize / 2, -markerSize / 2, 0));
    modelMarker.push_back(cv::Point3f(-markerSize / 2, -markerSize / 2, 0));

    apriltag_detector_t* td = apriltag_detector_create();
    td->quad_decimate = parameters->quadDecimate;
    apriltag_family_t* tf;
    if(!parameters->circularMarkers)
        tf = tagStandard41h12_create();
    else
        tf = tagCircle21h7_create();
    apriltag_detector_add_family(td, tf);

    int markersPerTracker = 45;
    int trackerNum = parameters->trackerNum;

    std::vector<cv::Vec3d> boardRvec, boardTvec;

    for (int i = 0; i < trackerNum; i++)
    {
        std::vector<int > curBoardIds;
        std::vector < std::vector<cv::Point3f >> curBoardCorners;
        curBoardIds.push_back(i * markersPerTracker);
        curBoardCorners.push_back(modelMarker);
        boardIds.push_back(curBoardIds);
        boardCorners.push_back(curBoardCorners);
        boardFound.push_back(false);
        boardRvec.push_back(cv::Vec3d());
        boardTvec.push_back(cv::Vec3d());
    }
    cv::Mat image;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

    std::vector<int> idsList;
    std::vector<std::vector < std::vector<cv::Point3f >>> cornersList;

    while (cameraRunning && mainThreadRunning)
    {
        while (!imageReady)
            Sleep(1);
        retImage.copyTo(image);
        imageReady = false;

        clock_t start, end;
        //clock for timing of detection
        start = clock();

        //detect and draw all markers on image
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f> > corners;
        std::vector<cv::Point2f> centers;

        //cv::aruco::detectMarkers(image, dictionary, corners, ids, params);
        detectMarkersApriltag(image, &corners, &ids, &centers, td);

        cv::aruco::drawDetectedMarkers(image, corners, cv::noArray(), cv::Scalar(255, 0, 0));

        //estimate pose of our markers
        std::vector<cv::Vec3d> rvecs, tvecs;
        cv::aruco::estimatePoseSingleMarkers(corners, markerSize, parameters->camMat, parameters->distCoefs, rvecs, tvecs);
        /*
        for (int i = 0; i < rvecs.size(); ++i) {
            //draw axis for each marker
            auto rvec = rvecs[i];	//rotation vector of our marker
            auto tvec = tvecs[i];	//translation vector of our marker

            //rotation/translation vectors are shown as offset of our camera from the marker

            cv::aruco::drawAxis(image, parameters->camMat, parameters->distCoefs, rvec, tvec, parameters->markerSize);
        }
        */

        float maxDist = parameters->trackerCalibDistance;

        for (int i = 0; i < boardIds.size(); i++)
        {
            cv::Ptr<cv::aruco::Board> arBoard = cv::aruco::Board::create(boardCorners[i], dictionary, boardIds[i]);
            //cv::Vec3d boardRvec, boardTvec;
            //bool boardFound = false;
            try
            {
                if (cv::aruco::estimatePoseBoard(corners, ids, arBoard, parameters->camMat, parameters->distCoefs, boardRvec[i], boardTvec[i], false) > 0)
                {
                    cv::aruco::drawAxis(image, parameters->camMat, parameters->distCoefs, boardRvec[i], boardTvec[i], 0.1);
                    boardFound[i] = true;
                }
                else
                {
                    boardFound[i] = false;
                }
            }
            catch (std::exception& e)
            {
                wxMessageDialog* dial = new wxMessageDialog(NULL,
                    wxT("Something went wrong. Try again."), wxT("Error"), wxOK | wxICON_ERROR);
                dial->ShowModal();
                cv::destroyWindow("out");
                apriltag_detector_destroy(td);
                mainThreadRunning = false;
                return;
            }

            std::string testStr = std::to_string(boardTvec[i][0]) + " " + std::to_string(boardTvec[i][1]) + " " + std::to_string(boardTvec[i][2]);

            for (int j = 0; j < ids.size(); j++)
            {
                if (ids[j] >= i * markersPerTracker && ids[j] < (i + 1) * markersPerTracker)
                {
                    bool markerInBoard = false;
                    for (int k = 0; k < boardIds[i].size(); k++)
                    {
                        if (boardIds[i][k] == ids[j])
                        {
                            markerInBoard = true;
                            break;
                        }
                    }
                    if (markerInBoard == true)
                    {
                        drawMarker(image, corners[j], cv::Scalar(0, 255, 0));
                        continue;
                    }
                    if (boardFound[i])
                    {
                        if (sqrt(tvecs[j][0] * tvecs[j][0] + tvecs[j][1] * tvecs[j][1] + tvecs[j][2] * tvecs[j][2]) > maxDist)
                        {
                            drawMarker(image, corners[j], cv::Scalar(255, 0, 255));
                            continue;
                        }

                        drawMarker(image, corners[j], cv::Scalar(0, 255, 255));
                        std::vector<cv::Point3f> marker;
                        transformMarkerSpace(modelMarker, boardRvec[i], boardTvec[i], rvecs[j], tvecs[j], &marker);

                        int listIndex = -1;
                        for (int k = 0; k < idsList.size(); k++)
                        {
                            if (idsList[k] == ids[j])
                            {
                                listIndex = k;
                            }
                        }
                        if (listIndex < 0)
                        {
                            listIndex = idsList.size();
                            idsList.push_back(ids[j]);
                            cornersList.push_back(std::vector<std::vector<cv::Point3f>>());
                        }

                        cornersList[listIndex].push_back(marker);
                        if (cornersList[listIndex].size() > 50)
                        {
                            std::vector<cv::Point3f> medianMarker;

                            getMedianMarker(cornersList[listIndex], &medianMarker);

                            boardIds[i].push_back(ids[j]);
                            boardCorners[i].push_back(medianMarker);
                        }

                    }
                    else
                    {
                        drawMarker(image, corners[j], cv::Scalar(0, 0, 255));
                    }
                }
            }
        }
        cv::Mat drawImg;
        int cols, rows;
        if (image.cols > image.rows)
        {
            cols = image.cols * drawImgSize / image.rows;
            rows = drawImgSize;
        }
        else
        {
            cols = drawImgSize;
            rows = image.rows * drawImgSize / image.cols;
        }
        cv::resize(image, drawImg, cv::Size(cols, rows));
        cv::imshow("out", drawImg);
        cv::waitKey(1);
    }
    trackers.clear();
    for (int i = 0; i < boardIds.size(); i++)
    {
        cv::Ptr<cv::aruco::Board> arBoard = cv::aruco::Board::create(boardCorners[i], dictionary, boardIds[i]);
        trackers.push_back(arBoard);
    }
    parameters->trackers = trackers;
    parameters->Save();
    trackersCalibrated = true;

    cv::destroyWindow("out");
    apriltag_detector_destroy(td);
    mainThreadRunning = false;
}

void Tracker::MainLoop()
{
    std::vector<int> prevIds;
    std::vector<std::vector<cv::Point2f> > prevCorners;
    std::vector<cv::Point2f> prevCenters;
    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f> > corners;
    std::vector<cv::Point2f> centers;

    std::vector<cv::Point2f> maskCenters;

    cv::Mat  prevImg;

    cv::Mat image, drawImg;

    std::vector<cv::Vec3d> boardRvec, boardTvec;
    std::vector<bool> boardFound;

    std::vector<cv::Vec3d> prevLoc;
    std::vector<Quaternion<double>> prevRot;
    std::vector<std::vector<std::vector<double>>> prevLocValues;
    std::vector<std::vector<std::vector<double>>> prevLocValuesRaw;
    std::vector<double> prevLocValuesX;

    int trackerNum = parameters->trackerNum;

    int numOfPrevValues = parameters->numOfPrevValues;

    for (int k = 0; k < trackerNum; k++)
    {
        std::vector<std::vector<double>> vv;

        for (int i = 0; i < 6; i++)
        {

            std::vector<double> v;
            /*
            for (int j = 0; j < numOfPrevValues; j++)
            {
                v.push_back(0.0);
            }
            */
            vv.push_back(v);
        }

        prevLocValuesRaw.push_back(vv);

        prevLoc.push_back(cv::Vec3d(0, 0, 0));
        prevRot.push_back(Quaternion<double>());
    }

    //the X axis, it is simply numbers 0-10 (or the amount of previous values we have)
    for (int j = 0; j < numOfPrevValues; j++)
    {
        prevLocValuesX.push_back(j);
    }

    for (int i = 0; i < trackerNum; i++)
    {
        boardRvec.push_back(cv::Vec3d(0, 0, 0));
        boardTvec.push_back(cv::Vec3d(0, 0, 0));
        boardFound.push_back(false);
    }

    apriltag_detector_t* td = apriltag_detector_create();
    td->quad_decimate = parameters->quadDecimate;
    apriltag_family_t* tf;
    if (!parameters->circularMarkers)
        tf = tagStandard41h12_create();
    else
        tf = tagCircle21h7_create();
    apriltag_detector_add_family(td, tf);
    apriltag_detector_add_family(td, tf);

    int framesSinceLastSeen = 0;
    int framesToCheckAll = 20;

    cv::Mat stationPos = (cv::Mat_<double>(4, 1) << 0, 0, 0, 1);
    stationPos = wtranslation * stationPos;

    Quaternion<double> stationQ = Quaternion<double>(0, 0, 1, 0) * (wrotation * Quaternion<double>(1, 0, 0, 0));

    double a = -stationPos.at<double>(0, 0);
    double b = stationPos.at<double>(1, 0);
    double c = -stationPos.at<double>(2, 0);

    connection->SendStation(0, a, b, c, stationQ.w, stationQ.x, stationQ.y, stationQ.z);

    while(mainThreadRunning && cameraRunning)
    {
        while (!imageReady)
            Sleep(1);

        retImage.copyTo(image);
        imageReady = false;

        image.copyTo(drawImg);
        cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);

        clock_t start, end;
        //for timing our detection
        start = clock();

        bool circularWindow = parameters->circularWindow;

        for (int i = 0; i < boardFound.size(); i++)
        {
            if (!boardFound[i])
            {
                framesSinceLastSeen++;
                if (framesSinceLastSeen > framesToCheckAll)
                    circularWindow = false;
                break;
            }
        }
        if (!circularWindow)
            framesSinceLastSeen = 0;

        if (maskCenters.size() > 0)
        {
            //Then define your mask image
            cv::Mat mask = cv::Mat::zeros(image.size(), image.type());

            cv::Mat dstImage = cv::Mat::zeros(image.size(), image.type());

            int size = image.rows * parameters->searchWindow;

            //I assume you want to draw the circle at the center of your image, with a radius of 50
            for (int i = 0; i < maskCenters.size(); i++)
            {
                if (circularWindow)
                {
                    cv::circle(mask, maskCenters[i], size, cv::Scalar(255, 0, 0), -1, 8, 0);
                    cv::circle(drawImg, maskCenters[i], size, cv::Scalar(255, 0, 0), 2, 8, 0);
                }
                else
                {
                    rectangle(mask, cv::Point(maskCenters[i].x - size, 0), cv::Point(maskCenters[i].x + size, image.rows), cv::Scalar(255, 0, 0), -1);
                    rectangle(drawImg, cv::Point(maskCenters[i].x - size, 0), cv::Point(maskCenters[i].x + size, image.rows), cv::Scalar(255, 0, 0), 3);
                }
            }

            //Now you can copy your source image to destination image with masking
            image.copyTo(dstImage, mask);
            image = dstImage;
            //cv::imshow("test", image);
        }

        if (manualRecalibrate)
        {
            //wtranslation = getSpaceCalib(boardRvec[i], boardTvec[i], parameters->calibOffsetX, parameters->calibOffsetY, parameters->calibOffsetZ);
            cv::Vec3d calibRot(gui->manualCalibA->value * 0.01745, gui->manualCalibB->value * 0.01745, gui->manualCalibC->value * 0.01745);
            cv::Vec3d calibPos(gui->manualCalibX->value / 100, gui->manualCalibY->value / 100, gui->manualCalibZ->value / 100);
            cv::Vec3d calibRodr(cos(calibRot[0]) * cos(calibRot[1]) *3.14, sin(calibRot[1]) * 3.14, sin(calibRot[0]) * cos(calibRot[1]) * 3.14);
            /*
            std::string teststr = std::to_string(calibRodr[0]) + " " + std::to_string(calibRodr[1]) + " " + std::to_string(calibRodr[2]);

            wxMessageDialog* dial = new wxMessageDialog(NULL,
                teststr, wxT("Error"), wxOK | wxICON_ERROR);
            dial->ShowModal();
            */
            wtranslation = getSpaceCalibEuler(calibRot, cv::Vec3d(0, 0, 0), calibPos(0), calibPos(1), calibPos(2));
            //wrotation = rodr2quat(calibRodr[0], calibRodr[1], calibRodr[2]);
            //wrotation = rodr2quat(boardRvec[i][0], boardRvec[i][1], boardRvec[i][2]).conjugate();
            wrotation = mRot2Quat(eulerAnglesToRotationMatrix(cv::Vec3f(calibRot)));
            /*
            teststr = std::to_string(wrotation.w) + " " + std::to_string(wrotation.x) + " " + std::to_string(wrotation.y) + " " + std::to_string(wrotation.z);

            dial = new wxMessageDialog(NULL,
                teststr, wxT("Error"), wxOK | wxICON_ERROR);
            dial->ShowModal();

            Sleep(2000);
            */
            cv::Mat stationPos = (cv::Mat_<double>(4, 1) << 0, 0, 0, 1);
            stationPos = wtranslation * stationPos;

            Quaternion<double> stationQ = Quaternion<double>(0, 0, 1, 0) * (wrotation * Quaternion<double>(1, 0, 0, 0));

            double a = -stationPos.at<double>(0, 0);
            double b = stationPos.at<double>(1, 0);
            double c = -stationPos.at<double>(2, 0);

            connection->SendStation(0, a, b, c, stationQ.w, stationQ.x, stationQ.y, stationQ.z);
        }

        detectMarkersApriltag(image, &corners, &ids, &centers, td);

        for (int i = 0; i < centers.size(); i++)
        {
            int tracker = ids[i] / 45;

            int limit = trackerNum;

            if (parameters->ignoreTracker0)
            {
                if (tracker == 0)
                    continue;
                tracker--;
                limit--;
            }

            if (tracker < limit)
            {
                while (tracker >= maskCenters.size())
                {
                    maskCenters.push_back(centers[i]);
                }
                maskCenters[tracker] = centers[i];
            }
        }
        for (int i = 0; i < trackerNum; ++i) {

            //estimate the pose of current board

            if (parameters->ignoreTracker0 && i == 0)
                continue;
            try
            {
                if (cv::aruco::estimatePoseBoard(corners, ids, trackers[i], parameters->camMat, parameters->distCoefs, boardRvec[i], boardTvec[i], boardFound[i] && parameters->usePredictive) <= 0)
                {
                    for (int j = 0; j < 6; j++)
                    {
                        //push new values into previous values list end and remove the one on beggining
                        if (prevLocValuesRaw[i][j].size() > 0)
                            prevLocValuesRaw[i][j].erase(prevLocValuesRaw[i][j].begin());
                    }
                    boardFound[i] = false;
                    continue;
                }
            }
            catch (std::exception& e)
            {
                wxMessageDialog* dial = new wxMessageDialog(NULL,
                    wxT("Something went wrong when estimating tracker pose. Try again! \nIf the problem persists, try to recalibrate camera and trackers."), wxT("Error"), wxOK | wxICON_ERROR);
                dial->ShowModal();
                cv::destroyWindow("out");
                apriltag_detector_destroy(td);
                mainThreadRunning = false;
                return;
            }
            boardFound[i] = true;

            double posValues[6] = { boardTvec[i][0],boardTvec[i][1],boardTvec[i][2],boardRvec[i][0],boardRvec[i][1],boardRvec[i][2] };

            for (int j = 0; j < 6; j++)
            {
                //push new values into previous values list end and remove the one on beggining
                prevLocValuesRaw[i][j].push_back(posValues[j]);
                if (prevLocValuesRaw[i][j].size() > numOfPrevValues)
                {
                    prevLocValuesRaw[i][j].erase(prevLocValuesRaw[i][j].begin());
                }

                std::vector<double> valArray(prevLocValuesRaw[i][j]);
                sort(valArray.begin(), valArray.end());

                posValues[j] = valArray[valArray.size() / 2];

            }
            //save fitted values back to our variables
            boardTvec[i][0] = posValues[0];
            boardTvec[i][1] = posValues[1];
            boardTvec[i][2] = posValues[2];
            boardRvec[i][0] = posValues[3];
            boardRvec[i][1] = posValues[4];
            boardRvec[i][2] = posValues[5];

            cv::Mat rpos = cv::Mat_<double>(4, 1);

            //transform boards position based on our calibration data

            for (int x = 0; x < 3; x++)
            {
                rpos.at<double>(x, 0) = boardTvec[i][x];
            }
            rpos.at<double>(3, 0) = 1;
            rpos = wtranslation * rpos;

            //convert rodriguez rotation to quaternion
            Quaternion<double> q = rodr2quat(boardRvec[i][0], boardRvec[i][1], boardRvec[i][2]);

            //mirror our rotation
            //q.z = -q.z;
            //q.x = -q.x;

            q = Quaternion<double>(0, 0, 1, 0) * (wrotation * q) * Quaternion<double>(0, 0, 1, 0);

            double a =  -rpos.at<double>(0, 0);
            double b =   rpos.at<double>(1, 0);
            double c =  -rpos.at<double>(2, 0);

            double factor;
            factor = parameters->smoothingFactor;

            if (factor < 0)
                factor = 0;
            else if (factor >= 1)
                factor = 0.99;

            /*
            a = (1 - factor) * prevLoc[i][0] + (factor)*a;
            b = (1 - factor) * prevLoc[i][1] + (factor)*b;
            c = (1 - factor) * prevLoc[i][2] + (factor)*c;

            q = q.UnitQuaternion();

            //to ensure we rotate quaternion into correct direction
            double dot = q.x * prevRot[i].x + q.y * prevRot[i].y + q.z * prevRot[i].z + q.w * prevRot[i].w;

            if (dot < 0)
            {
                q.x = (factor)*q.x - (1 - factor) * prevRot[i].x;
                q.y = (factor)*q.y - (1 - factor) * prevRot[i].y;
                q.z = (factor)*q.z - (1 - factor) * prevRot[i].z;
                q.w = (factor)*q.w - (1 - factor) * prevRot[i].w;
            }
            else
            {
                q.x = (factor)*q.x + (1 - factor) * prevRot[i].x;
                q.y = (factor)*q.y + (1 - factor) * prevRot[i].y;
                q.z = (factor)*q.z + (1 - factor) * prevRot[i].z;
                q.w = (factor)*q.w + (1 - factor) * prevRot[i].w;
            }

            q = q.UnitQuaternion();
            */

            //save values for next frame
            prevRot[i] = q;
            prevLoc[i] = cv::Vec3d(a, b, c);

            //cv::putText(drawImg, std::to_string(q.w) + ", " + std::to_string(q.x) + ", " + std::to_string(q.y) + ", " + std::to_string(q.z), cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
            //cv::putText(drawImg, std::to_string(boardRvec[i][0]) + ", " + std::to_string(boardRvec[i][1]) + ", " + std::to_string(boardRvec[i][2]), cv::Point(10, 80), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));

            end = clock();
            double frameTime = double(end - last_frame_time) / double(CLOCKS_PER_SEC);


            //send all the values
            //frame time is how much time passed since frame was acquired. It doesn't work as expected...
            connection->SendTracker(i, a, b, c, q.w, q.x, q.y, q.z,-frameTime-parameters->camLatency,factor);

        }

        if (ids.size() > 0)
            cv::aruco::drawDetectedMarkers(drawImg, corners, ids);

        end = clock();
        double frameTime = double(end - start) / double(CLOCKS_PER_SEC);

        int cols, rows;
        if (image.cols > image.rows)
        {
            cols = image.cols * drawImgSize / image.rows;
            rows = drawImgSize;
        }
        else
        {
            cols = drawImgSize;
            rows = image.rows * drawImgSize / image.cols;
        }
        cv::resize(drawImg, drawImg, cv::Size(cols, rows));
        cv::putText(drawImg, std::to_string(frameTime).substr(0,5), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
        cv::imshow("out", drawImg);
        cv::waitKey(1);
        //time of marker detection
    }
    cv::destroyWindow("out");
}

void Tracker::detectMarkersApriltag(cv::Mat frame, std::vector<std::vector<cv::Point2f> >* corners, std::vector<int>* ids, std::vector<cv::Point2f>* centers, apriltag_detector_t* td)
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
