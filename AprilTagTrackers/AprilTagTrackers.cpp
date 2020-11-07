#include "AprilTagTrackers.h"

wxIMPLEMENT_APP(MyApp);

int MyApp::OnExit()
{      
    tracker->cameraRunning = false;
    tracker->mainThreadRunning = false;
    Sleep(2000);
    return 0;
}
bool MyApp::OnInit()
{
    
    params = new Parameters();
    conn = new Connection(params);
    tracker = new Tracker(params, conn);

    GUI* simple = new GUI(wxT("Heloooooo"),params);
    simple->Show(true);

    Connect(GUI::CAMERA_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedCamera));
    Connect(GUI::CAMERA_CALIB_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedCameraCalib));
    Connect(GUI::CAMERA_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedCameraPreview));
    Connect(GUI::CONNECT_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedConnect));
    Connect(GUI::TRACKER_CALIB_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedTrackerCalib));
    Connect(GUI::START_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedStart));
    Connect(GUI::SPACE_CALIB_CHECKBOX, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyApp::ButtonPressedSpaceCalib));

    return true;
}

void MyApp::ButtonPressedCamera(wxCommandEvent& event)
{
    tracker->StartCamera(params->cameraAddr);
}

void MyApp::ButtonPressedCameraPreview(wxCommandEvent& event)
{
    if (event.IsChecked())
        tracker->previewCamera = true;
    else
        tracker->previewCamera = false;
}

void MyApp::ButtonPressedCameraCalib(wxCommandEvent& event)
{
    tracker->StartCameraCalib();
}

void MyApp::ButtonPressedConnect(wxCommandEvent& event)
{
    conn->StartConnection();
}

void MyApp::ButtonPressedTrackerCalib(wxCommandEvent& event)
{
    tracker->StartTrackerCalib();
}

void MyApp::ButtonPressedStart(wxCommandEvent& event)
{
    tracker->Start();
}

void MyApp::ButtonPressedSpaceCalib(wxCommandEvent& event)
{
    if (event.IsChecked())
        tracker->recalibrate = true;
    else
        tracker->recalibrate = false;
}

Tracker::Tracker(Parameters* params, Connection* conn)
{
    parameters = params;
    connection = conn;
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
    if (id.length() <= 2)		//if camera adress is a single character, try to open webcam
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
            wxT("Camera error"), wxT("Error"), wxOK | wxICON_ERROR);
        dial->ShowModal();
        return;
    }
    cap.set(cv::CAP_PROP_FPS, 60);
    cameraRunning = true;
    cameraThread = std::thread(&Tracker::CameraLoop, this);
    cameraThread.detach();
}

void Tracker::CameraLoop()
{
    cv::Mat img;
    bool rotate = false;
    if (parameters->rotate)
        rotate = true;
    if (!cap.read(img))
    {
        wxMessageDialog* dial = new wxMessageDialog(NULL,
            wxT("Camera error"), wxT("Error"), wxOK | wxICON_ERROR);
        dial->ShowModal();
        cameraRunning = false;
        cap.release();
        return;
    }
    if (rotate)
        cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);
    im = image_u8_create_stride(img.cols, img.rows, img.cols);
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
            cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);
        img.copyTo(retImage);
        imageReady = true;
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
        mainThreadRunning = false;
        return;
    }

    mainThreadRunning = true;
    mainThread = std::thread(&Tracker::CalibrateCamera, this);
    mainThread.detach();
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
    imshow("Chessboard", chessBoard);

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
    while (i < 15)
    {
        if (!mainThreadRunning || !cameraRunning)
            return;
        while (!imageReady)
            Sleep(1);
        imageReady = false;
        retImage.copyTo(image);
        cv::putText(image, std::to_string(i) + "/15", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
        cv::imshow("out", image);
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

            cv::imshow("out", image);
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
    td->quad_decimate = 1;
    apriltag_family_t* tf = tagStandard41h12_create();
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
        for (int i = 0; i < boardIds.size(); i++)
        {
            cv::Ptr<cv::aruco::Board> arBoard = cv::aruco::Board::create(boardCorners[i], dictionary, boardIds[i]);
            //cv::Vec3d boardRvec, boardTvec;
            //bool boardFound = false;
            if (cv::aruco::estimatePoseBoard(corners, ids, arBoard, parameters->camMat, parameters->distCoefs, boardRvec[i], boardTvec[i],boardFound[i]) > 0)
            {
                cv::aruco::drawAxis(image, parameters->camMat, parameters->distCoefs, boardRvec[i], boardTvec[i], 0.1);
                boardFound[i] = true;
            }
            else
            {
                boardFound[i] = false;
            }

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

        cv::imshow("out", image);
        cv::waitKey(1);
    }
    trackers.clear();
    for (int i = 0; i < boardIds.size(); i++)
    {
        cv::Ptr<cv::aruco::Board> arBoard = cv::aruco::Board::create(boardCorners[i], dictionary, boardIds[i]);
        trackers.push_back(arBoard);
    }
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

    std::vector<std::vector<std::vector<double>>> prevLocValues;
    std::vector<std::vector<std::vector<double>>> prevLocValuesRaw;
    std::vector<double> prevLocValuesX;

    int numOfPrevValues = parameters->numOfPrevValues;

    for (int k = 0; k < trackers.size(); k++)
    {
        std::vector<std::vector<double>> vv;
        for (int i = 0; i < 6; i++)
        {
            std::vector<double> v;
            for (int j = 0; j < numOfPrevValues; j++)
            {
                v.push_back(0.0);
            }
            vv.push_back(v);
        }
        prevLocValuesRaw.push_back(vv);
    }

    //the X axis, it is simply numbers 0-10 (or the amount of previous values we have)
    for (int j = 0; j < numOfPrevValues; j++)
    {
        prevLocValuesX.push_back(j);
    }

    for (int i = 0; i < trackers.size(); i++)
    {
        boardRvec.push_back(cv::Vec3d(0, 0, 0));
        boardTvec.push_back(cv::Vec3d(0, 0, 0));
        boardFound.push_back(false);
    }

    apriltag_detector_t* td = apriltag_detector_create();
    td->quad_decimate = parameters->quadDecimate;
    apriltag_family_t* tf = tagStandard41h12_create();
    apriltag_detector_add_family(td, tf);

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


        if (maskCenters.size() > 0)
        {
            //Then define your mask image
            cv::Mat mask = cv::Mat::zeros(image.size(), image.type());

            cv::Mat dstImage = cv::Mat::zeros(image.size(), image.type());

            int size = image.rows * parameters->searchWindow;

            //I assume you want to draw the circle at the center of your image, with a radius of 50
            for (int i = 0; i < maskCenters.size(); i++)
            {
                if (parameters->circularWindow)
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

        detectMarkersApriltag(image, &corners, &ids, &centers, td);

        for (int i = 0; i < centers.size(); i++)
        {
            int tracker = ids[i] / 45;

            while (tracker >= maskCenters.size())
            {
                maskCenters.push_back(centers[i]);
            }
            maskCenters[tracker] = centers[i];
        }
        for (int i = 0; i < trackers.size(); ++i) {

            //estimate the pose of current board

            if (cv::aruco::estimatePoseBoard(corners, ids, trackers[i], parameters->camMat, parameters->distCoefs, boardRvec[i], boardTvec[i], boardFound[i] && parameters->usePredictive) <= 0)
            {
                for (int j = 0; j < 6; j++)
                {
                    //push new values into previous values list end and remove the one on beggining
                    if(prevLocValuesRaw[i][j].size() > 0)
                        prevLocValuesRaw[i][j].erase(prevLocValuesRaw[i][j].begin());
                }
                boardFound[i] = false;
                continue;
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
            double b = -rpos.at<double>(2, 0);
            double c =  rpos.at<double>(1, 0);

            //cv::putText(drawImg, std::to_string(q.w) + ", " + std::to_string(q.x) + ", " + std::to_string(q.y) + ", " + std::to_string(q.z), cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
            //cv::putText(drawImg, std::to_string(boardRvec[i][0]) + ", " + std::to_string(boardRvec[i][1]) + ", " + std::to_string(boardRvec[i][2]), cv::Point(10, 80), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));

            connection->Send(i, a, b, c, q.w, q.x, q.y, q.z);
            if (recalibrate && i == parameters->calibrationTracker)
            {
                cv::aruco::drawAxis(drawImg, parameters->camMat, parameters->distCoefs, boardRvec[i], boardTvec[i], 0.10);
                wtranslation = getSpaceCalib(boardRvec[i], boardTvec[i], parameters->calibOffsetX, parameters->calibOffsetY, parameters->calibOffsetZ);
                wrotation = rodr2quat(boardRvec[i][0], boardRvec[i][1], boardRvec[i][2]).conjugate();
            } 
        }

        if (ids.size() > 0)
            cv::aruco::drawDetectedMarkers(drawImg, corners, ids);

        end = clock();
        double frameTime = double(end - start) / double(CLOCKS_PER_SEC);

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


    //im = image_u8_create_stride(gray.cols, gray.rows, gray.cols);
    im->buf = gray.data;

    zarray_t* detections = apriltag_detector_detect(td, im);

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
