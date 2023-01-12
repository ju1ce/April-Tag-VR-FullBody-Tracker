#include "Tracker.hpp"

#include "AprilTagWrapper.hpp"
#include "Connection.hpp"
#include "Helpers.hpp"
#include "ImageDrawing.hpp"
#include "utils/Assert.hpp"

#include <bal_problem.hpp>
#include <bundle_adjuster.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <ps3eye/PSEyeVideoCapture.h>

#include <algorithm>
#include <array>
#include <exception>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <vector>

Tracker::Tracker(UserConfig& _userConfig, CalibrationConfig& _calibConfig, ArucoConfig& _arucoConfig, const Localization& _lc)
    : connection(std::make_unique<Connection>(_userConfig)),
      user_config(_userConfig), calib_config(_calibConfig), aruco_config(_arucoConfig), lc(_lc)
{
    if (!calib_config.trackers.empty())
    {
        trackers = calib_config.trackers;
        trackersCalibrated = true;
    }
    SetWorldTransform(user_config.manualCalib.GetAsReal());
}

void Tracker::StartCamera(RefPtr<cfg::CameraInfo> cam)
{
    if (cameraRunning)
    {
        cameraRunning = false;
        mainThreadRunning = false;
        // cameraThread.join();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return;
    }

    if (cam->address.length() <= 2) // if camera address is a single character, try to open webcam
    {
        int i = std::stoi(cam->address); // convert to int
#ifdef ATT_OS_LINUX
        // On Linux cv::VideoCapture does not work when GStreamer backend is used and
        // camera is set to MJPG pixel format. As a work around we manually setup the
        // GStreamer pipeline with suitable decoding before feeding the stream into
        // application.
        if ((cam->api == cv::CAP_ANY) || (cam->api == cv::CAP_GSTREAMER))
        {
            std::stringstream ss;
            ss << "v4l2src device=/dev/video" << i << " ! image/jpeg";
            if (cam->resolution.width != 0)
                ss << ",width=" << cam->resolution.width;
            if (cam->resolution.height != 0)
                ss << ",height=" << cam->resolution.height;
            ss << ",framerate=" << cam->fps << "/1";
            ss << " ! jpegdec ! video/x-raw,format=I420 ! videoconvert ! appsink";
            cap = cv::VideoCapture(ss.str(), cam->api);
        }
        else
#endif
            if (cam->api == 9100)
        {
            cap = PSEyeVideoCapture(i);
        }
        else
        {
            cap = cv::VideoCapture(i, cam->api);
        }
    }
    else
    {
        // if address is longer, we try to open it as an ip address
        cap = cv::VideoCapture(cam->address, cam->api);
    }

    if (!cap.isOpened())
    {
        gui->ShowPopup(lc.TRACKER_CAMERA_START_ERROR, PopupStyle::Error);
        return;
    }

    // On Linux and when GStreamer backend is used we already setup the camera pixel format,
    // width, height and FPS above when the GStreamer pipeline was created.
#ifdef ATT_OS_LINUX
    if ((cam->api != cv::CAP_ANY) && (cam->api != cv::CAP_GSTREAMER))
#endif
    {
        // cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('m', 'j', 'p', 'g'));
        // cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));

        if (cam->resolution.width != 0)
            cap.set(cv::CAP_PROP_FRAME_WIDTH, cam->resolution.width);
        if (cam->resolution.height != 0)
            cap.set(cv::CAP_PROP_FRAME_HEIGHT, cam->resolution.height);
        cap.set(cv::CAP_PROP_FPS, cam->fps);
    }
    if (cam->openDirectShowSettings)
        cap.set(cv::CAP_PROP_SETTINGS, 1);
    if (cam->extraSettings.enabled)
    {
        cap.set(cv::CAP_PROP_AUTOFOCUS, 0);
        cap.set(cv::CAP_PROP_AUTO_EXPOSURE, cam->extraSettings.autoExposure);
        cap.set(cv::CAP_PROP_EXPOSURE, cam->extraSettings.exposure);
        cap.set(cv::CAP_PROP_GAIN, cam->extraSettings.gain);
    }

    double codec = 0x47504A4D; // code by FPaul. Should use MJPEG codec to enable fast framerates.
    cap.set(cv::CAP_PROP_FOURCC, codec);

    cameraRunning = true;
    cameraThread = std::thread(&Tracker::CameraLoop, this);
    cameraThread.detach();
}

void Tracker::StartCamera()
{
    StartCamera(&user_config.videoStreams[0]->camera);
}

void Tracker::CameraLoop()
{
    RefPtr<cfg::CameraInfo> cam = &user_config.videoStreams[0]->camera;

    bool rotate = false;
    int rotateFlag = -1;

    bool mirror = false;

    if (cam->rotateCl >= 0)
    {
        rotate = true;
        rotateFlag = cam->rotateCl;
    }

    if (cam->mirror)
    {
        mirror = true;
    }

    cv::Mat img;
    cv::Mat drawImg;
    double fps = 0;
    auto last_preview_time = std::chrono::steady_clock::now();
    last_frame_time = std::chrono::steady_clock::now();
    bool frame_visible = false;
    gui->SetStatus(true, StatusItem::Camera);

    while (cameraRunning)
    {
        if (!cap.read(img) || img.empty())
        {
            gui->ShowPopup(lc.TRACKER_CAMERA_ERROR, PopupStyle::Error);
            cameraRunning = false;
            break;
        }
        auto curtime = std::chrono::steady_clock::now();
        fps = 0.95 * fps + 0.05 / std::chrono::duration<double>(curtime - last_frame_time).count();
        last_frame_time = curtime;
        if (rotate)
        {
            cv::rotate(img, img, rotateFlag);
        }
        if (mirror)
        {
            cv::flip(img, img, 1);
        }
        // Ensure that preview isnt shown more than 60 times per second.
        // In some cases opencv will return a solid color image without any blocking delay (unlike a camera locked to a framerate),
        // and we would just be drawing fps text on nothing.
        double timeSinceLast = std::chrono::duration<double>(curtime - last_preview_time).count();
        if (timeSinceLast > 0.015)
        {
            if (gui->IsPreviewVisible(PreviewId::Camera))
            {
                last_preview_time = std::chrono::steady_clock::now();
                img.copyTo(drawImg);
                cv::putText(drawImg, std::to_string(static_cast<int>(std::ceil(fps))), cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 255, 0), 2);
                std::string resolution = std::to_string(img.cols) + "x" + std::to_string(img.rows);
                cv::putText(drawImg, resolution, cv::Point(10, 120), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 255, 0), 2);

                // Draw lines to identify midpoint on camera preview to help with calibration
                cv::line(drawImg, cv::Point(img.cols/2, 0), cv::Point(img.cols/2, img.rows), cv::Scalar(0, 0, 255));
                cv::line(drawImg, cv::Point(0, img.rows/2), cv::Point(img.cols, img.rows/2), cv::Scalar(0, 0, 255));

                if (previewCameraCalibration)
                    drawCalibration(drawImg, *calib_config.cameras[0]);
                gui->UpdatePreview(drawImg, PreviewId::Camera);
            }
        }
        {
            std::lock_guard<std::mutex> lock(cameraImageMutex);
            // Swap avoids copying the pixel buffer. It only swaps pointers and metadata.
            // The pixel buffer from cameraFrame.image can be reused if the size and format matches.
            cv::swap(img, cameraFrame.image);
            // TODO: does opencv really care if img.read is called on the wrong size of matrix?
            if (img.size() != cameraFrame.image.size() || img.flags != cameraFrame.image.flags)
            {
                img.release();
            }
            cameraFrame.ready = true;
            cameraFrame.time = last_frame_time;
        }

        if (connection->PollQuitEvent())
            mainThreadRunning = false;
        HandleConnectionErrors();
    }
    cap.release();
    gui->SetStatus(false, StatusItem::Camera);
}

void Tracker::CopyFreshCameraImageTo(FrameData& frame)
{
    /// TODO: replace with std::condition_variable
    // Sleep happens between each iteration when the mutex is not locked.
    for (;; std::this_thread::sleep_for(std::chrono::milliseconds(1)))
    {
        std::lock_guard<std::mutex> lock(cameraImageMutex);
        if (cameraFrame.ready)
        {
            cameraFrame.ready = false;
            // Swap metadata and pointers to pixel buffers.
            cv::swap(frame.image, cameraFrame.image);
            // We don't want to overwrite shared data so release the image unless we are the only user of it.
            if (!(cameraFrame.image.u && cameraFrame.image.u->refcount == 1))
            {
                cameraFrame.image.release();
            }
            frame.ready = true;
            frame.time = cameraFrame.time;
            return;
        }
        else
        {
            frame.ready = false;
        }
    }
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
        gui->ShowPopup(lc.TRACKER_CAMERA_NOTRUNNING, PopupStyle::Error);
        mainThreadRunning = false;
        return;
    }

    mainThreadRunning = true;
    if (!user_config.chessboardCalib)
    {
        mainThread = std::thread(&Tracker::CalibrateCameraCharuco, this);
    }
    else
    {
        mainThread = std::thread(&Tracker::CalibrateCamera, this);
    }
    mainThread.detach();
}

/// function to calibrate our camera
void Tracker::CalibrateCameraCharuco()
{
    FrameData frame;
    cv::Mat gray;
    cv::Mat drawImg;

    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
    cv::Ptr<cv::aruco::DetectorParameters> params = cv::aruco::DetectorParameters::create();

    // generate and show our charuco board that will be used for calibration
    cv::Ptr<cv::aruco::CharucoBoard> board = cv::aruco::CharucoBoard::create(8, 7, 0.04f, 0.02f, dictionary);
    cv::Mat boardImage;

    // set our detectors marker border bits to 1 since thats what charuco uses
    params->markerBorderBits = 1;

    // int framesSinceLast = -2 * user_config.camFps;
    auto timeOfLast = std::chrono::steady_clock::now();

    bool promptSaveCalib = false;
    gui->ShowPrompt(lc.TRACKER_CAMERA_CALIBRATION_INSTRUCTIONS,
                    [&](bool pressedOk)
                    {
                        promptSaveCalib = pressedOk;
                        mainThreadRunning = false;
                    });

    cv::Mat cameraMatrix, distCoeffs, R, T;
    cv::Mat1d stdDeviationsIntrinsics, stdDeviationsExtrinsics;
    std::vector<double> perViewErrors;
    std::vector<std::vector<cv::Point2f>> allCharucoCorners;
    std::vector<std::vector<int>> allCharucoIds;
    cv::Mat outImg;

    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners;
    std::vector<std::vector<cv::Point2f>> rejectedCorners;

    auto preview = gui->CreatePreviewControl();

    int picsTaken = 0;
    while (mainThreadRunning && cameraRunning)
    {
        CopyFreshCameraImageTo(frame);
        cv::Mat &image = frame.image;
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

        image.copyTo(drawImg);
        cv::putText(drawImg, std::to_string(picsTaken), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));

        drawCalibration(
            drawImg,
            cameraMatrix,
            distCoeffs,
            stdDeviationsIntrinsics,
            perViewErrors,
            allCharucoCorners,
            allCharucoIds);

        // check the highest per view error and remove it if its higher than 1px.

        if (perViewErrors.size() > 10)
        {
            double maxPerViewError = 0;
            int maxPerViewErrorIdx = 0;

            for (int i = 0; i < perViewErrors.size(); i++)
            {
                if (perViewErrors[i] > maxPerViewError)
                {
                    maxPerViewError = perViewErrors[i];
                    maxPerViewErrorIdx = i;
                }
            }

            if (maxPerViewError > 1)
            {
                perViewErrors.erase(perViewErrors.begin() + maxPerViewErrorIdx);
                allCharucoCorners.erase(allCharucoCorners.begin() + maxPerViewErrorIdx);
                allCharucoIds.erase(allCharucoIds.begin() + maxPerViewErrorIdx);

                // recalibrate camera without the problematic frame
                cv::aruco::calibrateCameraCharuco(allCharucoCorners, allCharucoIds, board, cv::Size(image.rows, image.cols),
                                                  cameraMatrix, distCoeffs, R, T, stdDeviationsIntrinsics, stdDeviationsExtrinsics, perViewErrors,
                                                  cv::CALIB_USE_LU);

                picsTaken--;
            }
        }

        cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        cv::aruco::detectMarkers(gray, dictionary, markerCorners, markerIds, params, rejectedCorners);

        // TODO: If markers are detected, the image gets updated, and then the calibration timer below
        // captures another image, in the time before the opencv loop updates the preview on screen,
        // then the masked out tags will still be visible, it probably won't effect much though.
        for (const auto& corners : markerCorners)
        {
            ATT_ASSERT(static_cast<int>(corners.size()) == 4, "A square has four corners.");
            const std::array<cv::Point, 4> points = {corners[0], corners[1], corners[2], corners[3]};
            // much faster than fillPoly, and we know they will be convex
            cv::fillConvexPoly(drawImg, points.data(), points.size(), cv::Scalar::all(255));
        }

        cv::resize(drawImg, outImg, cv::Size(cols, rows));
        preview.Update(outImg);

        // if more than one second has passed since last calibration image, add current frame to calibration images
        if (std::chrono::duration<double>(std::chrono::steady_clock::now() - timeOfLast).count() > 1)
        {
            // detect our markers
            cv::aruco::refineDetectedMarkers(gray, board, markerCorners, markerIds, rejectedCorners);

            if (markerIds.size() > 0)
            {
                // if markers were found, try to add calibration data
                std::vector<cv::Point2f> charucoCorners;
                std::vector<int> charucoIds;
                // using data from aruco detection we refine the search of chessboard corners for higher accuracy
                cv::aruco::interpolateCornersCharuco(markerCorners, markerIds, gray, board, charucoCorners, charucoIds);
                if (charucoIds.size() > 15)
                {
                    // if corners were found, we draw them
                    // cv::aruco::drawDetectedCornersCharuco(drawImg, charucoCorners, charucoIds);
                    // we then add our corners to the array
                    allCharucoCorners.push_back(charucoCorners);
                    allCharucoIds.push_back(charucoIds);
                    picsTaken++;

                    if (picsTaken >= 3)
                    {
                        try
                        {
                            // Calibrate camera using our data
                            cv::aruco::calibrateCameraCharuco(allCharucoCorners, allCharucoIds, board, cv::Size(image.rows, image.cols),
                                                              cameraMatrix, distCoeffs, R, T, stdDeviationsIntrinsics, stdDeviationsExtrinsics, perViewErrors,
                                                              cv::CALIB_USE_LU);
                        }
                        catch (const cv::Exception& e)
                        {
                            ATT_LOG_ERROR(e.what());
                        }

                        size_t curI = perViewErrors.size();
                    }
                }
            }

            timeOfLast = std::chrono::steady_clock::now();
        }
    }

    mainThreadRunning = false;
    if (promptSaveCalib)
    {
        if (cameraMatrix.empty())
        {
            gui->ShowPopup(lc.TRACKER_CAMERA_CALIBRATION_NOTDONE, PopupStyle::Warning);
        }
        else
        {

            // some checks of the camera calibration values. The thresholds should be adjusted to prevent any false  negatives
            //
            // this was before bad frames were being removed, and should no longer be necessary and is commented out since it caused too many false negatives
            /*
            double avgPerViewError = 0;
            double maxPerViewError = 0;

            for (int i = 0; i < perViewErrors.size(); i++)
            {
                avgPerViewError += perViewErrors[i];
                if (perViewErrors[i] > maxPerViewError)
                    maxPerViewError = perViewErrors[i];
            }

            avgPerViewError /= perViewErrors.size();


            if (avgPerViewError > 0.5)          //a big reprojection error indicates that calibration wasnt done properly
            {
                wxMessageDialog dial(NULL, wxT("WARNING:\nThe avarage reprojection error is over 0.5 pixel. This usualy indicates a bad calibration."), wxT("Warning"), wxOK | wxICON_ERROR);
                dial.ShowModal();
            }
            if (maxPerViewError > 10)           //having one reprojection error very high indicates that one frame had missdetections
            {
                wxMessageDialog dial(NULL, wxT("WARNING:\nOne or more reprojection errors are over 10 pixels. This usualy indicates something went wrong during calibration."), wxT("Warning"), wxOK | wxICON_ERROR);
                dial.ShowModal();
            }

            volatile double test = stdDeviationsIntrinsics.at<double>(0);
            test = stdDeviationsIntrinsics.at<double>(1);
            test = stdDeviationsIntrinsics.at<double>(2);
            test = stdDeviationsIntrinsics.at<double>(3);

            if (stdDeviationsIntrinsics.at<double>(0) > 5 || stdDeviationsIntrinsics.at<double>(1) > 5)         //high uncertiancy is bad
            {
                wxMessageDialog dial(NULL, wxT("WARNING:\nThe calibration grid doesnt seem very stable. This usualy indicates a bad calibration."), wxT("Warning"), wxOK | wxICON_ERROR);
                dial.ShowModal();
            }
            */

            // Save calibration to our global params cameraMatrix and distCoeffs
            RefPtr<cfg::CameraCalibration> camCalib = calib_config.cameras[0];
            camCalib->cameraMatrix = cameraMatrix;
            camCalib->distortionCoeffs = distCoeffs;
            camCalib->stdDeviationsIntrinsics = stdDeviationsIntrinsics;
            camCalib->perViewErrors = perViewErrors;
            camCalib->allCharucoCorners = allCharucoCorners;
            camCalib->allCharucoIds = allCharucoIds;
            calib_config.Save();
            gui->ShowPopup(lc.TRACKER_CAMERA_CALIBRATION_COMPLETE, PopupStyle::Info);
        }
    }
}

void Tracker::CalibrateCamera()
{
    // old calibration function, only still here for legacy reasons.

    int CHECKERBOARD[2]{7, 7};

    int blockSize = 125;
    int imageSizeX = blockSize * (CHECKERBOARD[0] + 1);
    int imageSizeY = blockSize * (CHECKERBOARD[1] + 1);
    cv::Mat chessBoard(imageSizeX, imageSizeY, CV_8UC3, cv::Scalar::all(0));
    unsigned char color = 0;

    for (int i = 0; i < imageSizeX - 1; i = i + blockSize)
    {
        if (CHECKERBOARD[1] % 2 == 1)
            color = ~color;
        for (int j = 0; j < imageSizeY - 1; j = j + blockSize)
        {
            cv::Mat ROI = chessBoard(cv::Rect(j, i, blockSize, blockSize));
            ROI.setTo(cv::Scalar::all(color));
            color = ~color;
        }
    }
    // cv::namedWindow("Chessboard", cv::WINDOW_KEEPRATIO);
    // imshow("Chessboard", chessBoard);
    // cv::imwrite("chessboard.png", chessBoard);

    std::vector<std::vector<cv::Point3f>> objpoints;
    std::vector<std::vector<cv::Point2f>> imgpoints;
    std::vector<cv::Point3f> objp;

    for (int i{0}; i < CHECKERBOARD[0]; i++)
    {
        for (int j{0}; j < CHECKERBOARD[1]; j++)
        {
            objp.push_back(cv::Point3f(static_cast<float>(j), static_cast<float>(i), 0));
        }
    }

    std::vector<cv::Point2f> corner_pts;
    bool success;

    FrameData frame;
    cv::Mat outImg;

    int i = 0;
    int framesSinceLast = -100;

    int picNum = user_config.cameraCalibSamples;

    int image_cols, image_rows;

    while (i < picNum)
    {
        if (!mainThreadRunning || !cameraRunning)
        {
            return;
        }
        CopyFreshCameraImageTo(frame);
        cv::Mat &image = frame.image;
        cv::putText(image, std::to_string(i) + "/" + std::to_string(picNum), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
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

        framesSinceLast++;
        if (framesSinceLast > 50)
        {
            framesSinceLast = 0;
            cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);

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

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        cv::resize(image, outImg, cv::Size(cols, rows));
        gui->UpdatePreview(outImg);

        image_cols = image.cols;
        image_rows = image.rows;
    }

    cv::Mat cameraMatrix, distCoeffs, R, T;

    calibrateCamera(objpoints, imgpoints, cv::Size(image_rows, image_cols), cameraMatrix, distCoeffs, R, T);

    RefPtr<cfg::CameraCalibration> camCalib = calib_config.cameras[0];

    camCalib->cameraMatrix = cameraMatrix;
    camCalib->distortionCoeffs = distCoeffs;
    calib_config.Save();
    mainThreadRunning = false;
    gui->ShowPopup("Calibration complete.", PopupStyle::Info);
}

void Tracker::StartTrackerCalib()
{
    // check that no other process is running on main thread, check that camera is running and calibrated
    if (mainThreadRunning)
    {
        mainThreadRunning = false;
        return;
    }
    if (!cameraRunning)
    {
        gui->ShowPopup(lc.TRACKER_CAMERA_NOTRUNNING, PopupStyle::Error);
        mainThreadRunning = false;
        return;
    }
    if (calib_config.cameras[0]->cameraMatrix.empty())
    {
        gui->ShowPopup(lc.TRACKER_CAMERA_NOTCALIBRATED, PopupStyle::Error);
        mainThreadRunning = false;
        return;
    }

    // start tracker calibration on another thread
    mainThreadRunning = true;
    mainThread = std::thread(&Tracker::CalibrateTracker, this);
    mainThread.detach();

    gui->ShowPrompt(lc.TRACKER_TRACKER_CALIBRATION_INSTRUCTIONS,
                    [&](bool)
                    {
                        mainThreadRunning = false;
                    });
}

void Tracker::StartTrackerRefine()
{
    // check that no other process is running on main thread, check that camera is running and calibrated
    if (mainThreadRunning)
    {
        mainThreadRunning = false;
        return;
    }
    if (!cameraRunning)
    {
        gui->ShowPopup(lc.TRACKER_CAMERA_NOTRUNNING, PopupStyle::Error);
        mainThreadRunning = false;
        return;
    }
    if (calib_config.cameras[0]->cameraMatrix.empty())
    {
        gui->ShowPopup(lc.TRACKER_CAMERA_NOTCALIBRATED, PopupStyle::Error);
        mainThreadRunning = false;
        return;
    }
    if (!trackersCalibrated)
    {
        gui->ShowPopup(lc.TRACKER_TRACKER_NOTCALIBRATED, PopupStyle::Error);
        mainThreadRunning = false;
        return;
    }

    // start tracker calibration on another thread
    mainThreadRunning = true;
    mainThread = std::thread(&Tracker::RefineTracker, this);
    mainThread.detach();

    gui->ShowPrompt(lc.TRACKER_TRACKER_REFINE_INSTRUCTIONS,
                    [&](bool)
                    {
                        mainThreadRunning = false;
                    });
}

void Tracker::StartConnection()
{
    connection->StartConnection();
    gui->SetStatus(true, StatusItem::Driver);
}

void Tracker::HandleConnectionErrors()
{
    using Code = Connection::ErrorCode;
    Code code = connection->GetAndResetErrorState();
    if (code == Code::OK)
        return;

    gui->SetStatus(false, StatusItem::Driver);

    if (code == Code::ALREADY_WAITING)
        gui->ShowPopup("Already waiting for a connection.", PopupStyle::Error);
    else if (code == Code::ALREADY_CONNECTED)
        gui->ShowPopup("Connection closed.", PopupStyle::Info);
    else if (code == Code::CLIENT_ERROR)
        gui->ShowPopup(lc.CONNECT_CLIENT_ERROR + connection->GetErrorMsg(), PopupStyle::Error);
    else if (code == Code::BINDINGS_MISSING)
        gui->ShowPopup(lc.CONNECT_BINDINGS_ERROR, PopupStyle::Error);
    else if (code == Code::DRIVER_ERROR)
        gui->ShowPopup(lc.CONNECT_DRIVER_ERROR, PopupStyle::Error);
    else if (code == Code::DRIVER_MISMATCH)
        gui->ShowPopup(lc.CONNECT_DRIVER_MISSMATCH_1 + connection->GetErrorMsg() +
                           lc.CONNECT_DRIVER_MISSMATCH_2 + utils::GetBridgeDriverVersion().ToString(),
                       PopupStyle::Error);
    else // if (code == Code::SOMETHING_WRONG)
        gui->ShowPopup(lc.CONNECT_SOMETHINGWRONG, PopupStyle::Error);
}

void Tracker::SetWorldTransform(const cfg::ManualCalib::Real& calib)
{
    cv::Matx33d rmat = EulerAnglesToRotationMatrix(calib.angleOffset);
    wtransform = cv::Affine3d(rmat, calib.posOffset);
    wrotation = cv::Quatd::createFromRotMat(rmat).normalize();
    wscale = calib.scale;
}

void Tracker::Start()
{
    // check that no other process is running on main thread, check that camera is running and calibrated, check that trackers are calibrated
    if (mainThreadRunning)
    {
        mainThreadRunning = false;
        gui->SetStatus(false, StatusItem::Tracker);
        return;
    }
    if (!cameraRunning)
    {
        gui->ShowPopup(lc.TRACKER_CAMERA_NOTRUNNING, PopupStyle::Error);
        mainThreadRunning = false;
        return;
    }
    if (calib_config.cameras[0]->cameraMatrix.empty())
    {
        gui->ShowPopup(lc.TRACKER_CAMERA_NOTCALIBRATED, PopupStyle::Error);
        mainThreadRunning = false;
        return;
    }
    if (!trackersCalibrated)
    {
        gui->ShowPopup(lc.TRACKER_TRACKER_NOTCALIBRATED, PopupStyle::Error);
        mainThreadRunning = false;
        return;
    }
    if (connection->status != connection->CONNECTED)
    {
        gui->ShowPopup(lc.TRACKER_STEAMVR_NOTCONNECTED, PopupStyle::Error);
        mainThreadRunning = false;
        return;
    }

    gui->SetStatus(true, StatusItem::Tracker);

    // start detection on another thread
    mainThreadRunning = true;
    mainThread = std::thread(&Tracker::MainLoop, this);
    mainThread.detach();
}

void Tracker::Stop()
{
    mainThreadRunning = false;
    cameraRunning = false;
    /// TODO: join threads instead of sleeping
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

void Tracker::UpdateConfig()
{
    if (connection->status == Connection::CONNECTED)
    {
        connection->Send("settings 120 " + std::to_string(user_config.smoothingFactor) + " " + std::to_string(user_config.additionalSmoothing));
    }
}

void Tracker::CalibrateTracker()
{
    // initialize all parameters needed for tracker calibration

    std::vector<std::vector<int>> boardIds;
    std::vector<std::vector<std::vector<cv::Point3f>>> boardCorners;
    std::vector<bool> boardFound;

    // making a marker model of our markersize for later use
    std::vector<cv::Point3f> modelMarker;
    double markerSize = user_config.markerSize * 0.01; // markerSize is in centimeters, but we need it in meters
    modelMarker.push_back(cv::Point3f(-markerSize / 2.f, markerSize / 2.f, 0));
    modelMarker.push_back(cv::Point3f(markerSize / 2.f, markerSize / 2.f, 0));
    modelMarker.push_back(cv::Point3f(markerSize / 2.f, -markerSize / 2.f, 0));
    modelMarker.push_back(cv::Point3f(-markerSize / 2.f, -markerSize / 2.f, 0));

    AprilTagWrapper april{user_config, aruco_config};

    int markersPerTracker = user_config.markersPerTracker;
    int trackerNum = user_config.trackerNum;

    std::vector<cv::Vec3d> boardRvec, boardTvec;

    // add main marker to every tracker
    for (int i = 0; i < trackerNum; i++)
    {
        std::vector<int> curBoardIds;
        std::vector<std::vector<cv::Point3f>> curBoardCorners;
        curBoardIds.push_back(i * markersPerTracker);
        curBoardCorners.push_back(modelMarker);
        boardIds.push_back(curBoardIds);
        boardCorners.push_back(curBoardCorners);
        boardFound.push_back(false);
        boardRvec.push_back(cv::Vec3d());
        boardTvec.push_back(cv::Vec3d());
    }
    FrameData frame;
    cv::Mat outImg;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

    std::vector<int> idsList;
    std::vector<std::vector<std::vector<cv::Point3f>>> cornersList;

    // reset current tracker calibration data
    // TODO: Don't reset if user clicks cancel
    trackers.clear();

    auto preview = gui->CreatePreviewControl();
    RefPtr<cfg::CameraCalibration> camCalib = calib_config.cameras[0];

    // run loop until we stop it
    while (cameraRunning && mainThreadRunning)
    {
        CopyFreshCameraImageTo(frame);
        cv::Mat &image = frame.image;

        std::chrono::steady_clock::time_point start;
        // clock for timing of detection
        start = std::chrono::steady_clock::now();

        // detect and draw all markers on image
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        std::vector<cv::Point2f> centers;

        april.detectMarkers(image, &corners, &ids, &centers, trackers);
        if (showTimeProfile)
        {
            april.drawTimeProfile(image, cv::Point(10, 60));
        }

        cv::aruco::drawDetectedMarkers(image, corners, cv::noArray(), cv::Scalar(255, 0, 0)); // draw all markers blue. We will overwrite this with other colors for markers that are part of any of the trackers that we use

        // estimate pose of our markers
        std::vector<cv::Vec3d> rvecs, tvecs;
        cv::aruco::estimatePoseSingleMarkers(corners, static_cast<float>(markerSize), camCalib->cameraMatrix, camCalib->distortionCoeffs, rvecs, tvecs);

        float maxDist = static_cast<float>(user_config.trackerCalibDistance);

        for (int i = 0; i < boardIds.size(); i++) // for each of the trackers
        {
            cv::Ptr<cv::aruco::Board> arBoard = cv::aruco::Board::create(boardCorners[i], dictionary, boardIds[i]); // create an aruco board object made out of already added markers to current tracker

            try
            {
                if (cv::aruco::estimatePoseBoard(corners, ids, arBoard, camCalib->cameraMatrix, camCalib->distortionCoeffs, boardRvec[i], boardTvec[i], false) > 0) // try to estimate current trackers pose
                {
                    cv::aruco::drawAxis(image, camCalib->cameraMatrix, camCalib->distortionCoeffs, boardRvec[i], boardTvec[i], 0.1f); // if found, draw axis and mark it found
                    boardFound[i] = true;
                }
                else
                {
                    boardFound[i] = false; // else, if none of the markers for this tracker are visible, mark it not found
                }
            }
            catch (const std::exception& e) // on weird images or calibrations, we get an error. This should usualy only happen on bad camera calibrations, or in very rare cases
            {
                ATT_LOG_ERROR(e.what());
                gui->ShowPopup(lc.TRACKER_CALIBRATION_SOMETHINGWRONG, PopupStyle::Error);
                mainThreadRunning = false;
                return;
            }

            std::string testStr = std::to_string(boardTvec[i][0]) + " " + std::to_string(boardTvec[i][1]) + " " + std::to_string(boardTvec[i][2]);

            bool foundMarkerToCalibrate = false;

            for (int j = 0; j < ids.size(); j++) // check all of the found markers
            {
                if (ids[j] >= i * markersPerTracker && ids[j] < (i + 1) * markersPerTracker) // if marker is part of current tracker (usualy, 0 is 0-44, 1 is 45-89 etc)
                {
                    bool markerInBoard = false;
                    for (int k = 0; k < boardIds[i].size(); k++) // check if marker is already part of the tracker
                    {
                        if (boardIds[i][k] == ids[j])
                        {
                            markerInBoard = true;
                            break;
                        }
                    }
                    if (markerInBoard == true) // if it is, draw it green and continue to next marker
                    {
                        drawMarker(image, corners[j], cv::Scalar(0, 255, 0));
                        continue;
                    }
                    if (boardFound[i]) // if it isnt part of the current tracker, but the tracker was detected, we will attempt to add it
                    {
                        if (sqrt(tvecs[j][0] * tvecs[j][0] + tvecs[j][1] * tvecs[j][1] + tvecs[j][2] * tvecs[j][2]) > maxDist) // if marker is too far away from camera, we just paint it purple as adding it could have too much error
                        {
                            drawMarker(image, corners[j], cv::Scalar(255, 0, 255));
                            continue;
                        }

                        drawMarker(image, corners[j], cv::Scalar(0, 255, 255)); // start adding marker, mark that by painting it yellow

                        if (foundMarkerToCalibrate) // only calibrate one marker at a time, so continue loop if this is the second marker found
                            continue;

                        foundMarkerToCalibrate = true;

                        std::vector<cv::Point3f> marker;
                        transformMarkerSpace(modelMarker, boardRvec[i], boardTvec[i], rvecs[j], tvecs[j], &marker); // transform marker points to the coordinate system of the tracker

                        int listIndex = -1;
                        for (int k = 0; k < idsList.size(); k++) // check whether the idsList and cornersList already contains data for this marker
                        {
                            if (idsList[k] == ids[j])
                            {
                                listIndex = k;
                            }
                        }
                        if (listIndex < 0) // if not, add and initialize it
                        {
                            listIndex = static_cast<int>(idsList.size());
                            idsList.push_back(ids[j]);
                            cornersList.push_back(std::vector<std::vector<cv::Point3f>>());
                        }

                        cornersList[listIndex].push_back(marker); // add the current marker corners to the list
                        if (cornersList[listIndex].size() > 50)   // if we have 50 recorded instances in the list for current marker, we can add it to the tracker
                        {
                            std::vector<cv::Point3f> medianMarker;

                            getMedianMarker(cornersList[listIndex], &medianMarker); // calculate median position of each corner to get rid of outliers

                            boardIds[i].push_back(ids[j]); // add the marker to the tracker
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
        // resize image, then show it
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

        cv::resize(image, outImg, cv::Size(cols, rows));
        gui->UpdatePreview(outImg);
    }

    // when done calibrating, save the trackers to parameters
    for (int i = 0; i < boardIds.size(); i++)
    {
        cv::Ptr<cv::aruco::Board> arBoard = cv::aruco::Board::create(boardCorners[i], dictionary, boardIds[i]);
        trackers.push_back(arBoard);
    }

    calib_config.trackers = trackers;
    calib_config.Save();
    trackersCalibrated = true;

    mainThreadRunning = false;
}

void Tracker::RefineTracker()
{
    int markersPerTracker = user_config.markersPerTracker;
    int trackerNum = user_config.trackerNum;


    AprilTagWrapper april{user_config, aruco_config};

    FrameData frame;
    cv::Mat outImg;

    // setup all variables that need to be stored for each tracker and initialize them
    std::vector<TrackerStatus> trackerStatus = std::vector<TrackerStatus>(trackerNum, TrackerStatus());
    for (int i = 0; i < trackerStatus.size(); i++)
    {
        trackerStatus[i].boardFound = false;
        trackerStatus[i].boardRvec = cv::Vec3d(0, 0, 0);
        trackerStatus[i].boardTvec = cv::Vec3d(0, 0, 0);
        trackerStatus[i].prevLocValues = std::vector<std::vector<double>>(7, std::vector<double>());
        trackerStatus[i].last_update_timestamp = std::chrono::milliseconds(0);
        trackerStatus[i].searchSize = (int)(user_config.videoStreams[0]->searchWindow * calib_config.cameras[0]->cameraMatrix.at<double>(0,0));
    }

    RefPtr<cfg::CameraCalibration> camCalib = calib_config.cameras[0];

    auto preview = gui->CreatePreviewControl();

    typedef struct {
        int point_id;
        int camera_id;
        cv::Point2f observed;
    } Observation;
    typedef struct {
        cv::Vec3d rvec;
        cv::Vec3d tvec;
        cv::Mat cameraMatrix;
        cv::Mat distCoeffs;
    } Camera;

    int num_cameras = 0;
    int num_points = 0;
    int num_observations = 0;
    int num_parameters = 0;

    auto point_index = std::vector<int>();
    auto camera_index = std::vector<int>();
    auto observations = std::vector<double>();
    auto parameters = std::vector<double>();
    auto frameCountPerMarker = std::vector<int>(trackerNum * markersPerTracker, 0);

    int trackerBeingProcessed = -1;
    int frameNumber = 0;

    // run loop until we stop it
    while (cameraRunning && mainThreadRunning)
    {
        CopyFreshCameraImageTo(frame);
        cv::Mat &image = frame.image;
        cv::Mat mask = cv::Mat(image.size(), image.type(), cv::Scalar(1, 1, 1));

        std::chrono::steady_clock::time_point start, end;
        // clock for timing of detection
        start = std::chrono::steady_clock::now();

        // detect and draw all markers on image
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        std::vector<cv::Point2f> centers;


        april.detectMarkers(image, &corners, &ids, &centers, trackers);

        cv::aruco::drawDetectedMarkers(image, corners, cv::noArray(), cv::Scalar(255, 0, 0)); // draw all markers blue. We will overwrite this with other colors for markers that are part of any of the trackers that we use
        for (int i = 0; i < corners.size(); i++)
        {
            std::vector<cv::Point> points;
            for(int j = 0; j < corners[i].size(); j++){
                points.push_back(corners[i].at(j));
            }
            cv::fillConvexPoly(mask, points, cv::Scalar(1, 0, 0), 8, 0);
        }

        bool noTracker = true;
        bool tooFar = true;

        for (int i = 0; i < trackerNum; ++i)
        {

            // estimate the pose of current board
            try
            {
                if (cv::aruco::estimatePoseBoard(corners, ids, trackers[i], camCalib->cameraMatrix, camCalib->distortionCoeffs, trackerStatus[i].boardRvec, trackerStatus[i].boardTvec, trackerStatus[i].boardFound && user_config.usePredictive) <= 0)
                {
                    for (int j = 0; j < 6; j++)
                    {
                        // remove first of the previously saved values
                        if (trackerStatus[i].prevLocValues[j].size() > 0)
                            trackerStatus[i].prevLocValues[j].erase(trackerStatus[i].prevLocValues[j].begin());
                    }
                    trackerStatus[i].boardFound = false;

                    continue;
                }
            }
            catch (const std::exception& e)
            {
                // on rare occasions, detection crashes. Should be very rare and indicate something wrong with camera or tracker calibration
                ATT_LOG_ERROR(e.what());
                gui->ShowPopup(lc.TRACKER_DETECTION_SOMETHINGWRONG, PopupStyle::Error);
                mainThreadRunning = false;
                return;
            }
            trackerStatus[i].boardFound = true;
            noTracker = false;

            // Reject detected positions that are behind the camera
            if (trackerStatus[i].boardTvec[2]  < 0)
            {
                trackerStatus[i].boardFound = false;
                continue;
            }

            // Figure out the camera aspect ratio, XZ and YZ ratio limits
            double aspectRatio = static_cast<double>(image.cols) / static_cast<double>(image.rows);
            double XZratioLimit = 0.5 * static_cast<double>(image.cols) / camCalib->cameraMatrix.at<double>(0,0);
            double YZratioLimit = 0.5 * static_cast<double>(image.rows) / camCalib->cameraMatrix.at<double>(1,1);

            // Figure out whether X or Y dimension is most likely to go outside the camera field of view
            if (std::abs(trackerStatus[i].boardTvec[0] / trackerStatus[i].boardTvec[1]) > aspectRatio)
            {
                // Reject detections when XZ coordinate ratio goes out of camera FOV
                if (std::abs(trackerStatus[i].boardTvec[0] / trackerStatus[i].boardTvec[2]) > XZratioLimit)
                {
                    trackerStatus[i].boardFound = false;
                    continue;
                }
            }
            else
            {
                // Reject detections when YZ coordinate ratio goes out of camera FOV
                if (std::abs(trackerStatus[i].boardTvec[1] / trackerStatus[i].boardTvec[2]) > YZratioLimit)
                {
                    trackerStatus[i].boardFound = false;
                    continue;
                }
            }

            // TODO: Modify function to be able process more than one tracker at the same time
            if ((trackerBeingProcessed != -1) && (i != trackerBeingProcessed))
                continue;


            int tracker_id_begin = i * markersPerTracker;
            int tracker_id_end = (i + 1) * markersPerTracker;

            std::vector<int> trackerIds;
            std::vector<int> trackerIdsYellow;
            std::vector<std::vector<cv::Point2f>> trackerCorners;
            std::vector<std::vector<cv::Point2f>> trackerCornersYellow;
            std::vector<std::vector<cv::Point2f>> trackerCornersGreen;

            // Add get the marker IDs and corners for specific tracker
            for (int j = 0; j < ids.size(); ++j)
            {
                // does marker ID belong to current tracker?
                if ((tracker_id_begin <= ids[j]) && (ids[j] < tracker_id_end))
                {
                    trackerIds.push_back(ids[j]);
                    trackerCorners.push_back(corners[j]);
                    if(frameCountPerMarker[ids[j]] < 10)
                    {
                        trackerIdsYellow.push_back(ids[j]);
                        trackerCornersYellow.push_back(corners[j]);
                    }
                    else
                    {
                        trackerCornersGreen.push_back(corners[j]);
                    }
                }
            }

            if (trackerStatus[i].boardTvec[2] > 0.30)
            {
                cv::aruco::drawDetectedMarkers(image, trackerCorners, cv::noArray(), cv::Scalar(255, 0, 255)); // Purple: Tracker too far away
                for (int i = 0; i < trackerCorners.size(); i++)
                {
                    std::vector<cv::Point> points;
                    for(int j = 0; j < trackerCorners[i].size(); j++){
                        points.push_back(trackerCorners[i].at(j));
                    }
                    cv::fillConvexPoly(mask, points, cv::Scalar(1, 0, 1), 8, 0);
                }
                continue;
            }

            // Yellow: Need more snapshots
            cv::aruco::drawDetectedMarkers(image, trackerCornersYellow, cv::noArray(), cv::Scalar(0, 255, 255));
            for (int i = 0; i < trackerCornersYellow.size(); i++)
            {
                std::vector<cv::Point> points;
                for(int j = 0; j < trackerCornersYellow[i].size(); j++){
                    points.push_back(trackerCornersYellow[i].at(j));
                }
                double fillLevel = (double)frameCountPerMarker[trackerIdsYellow[i]] / 10.0;
                points[1] =  (1 - fillLevel) * points[0] + fillLevel * points[1];
                points[2] =  (1 - fillLevel) * points[3] + fillLevel * points[2];
                cv::fillConvexPoly(mask, points, cv::Scalar(0, 1, 1), 8, 0);
            }

            // Green: Marker done
            cv::aruco::drawDetectedMarkers(image, trackerCornersGreen, cv::noArray(), cv::Scalar(0, 255, 0));
            for (int i = 0; i < trackerCornersGreen.size(); i++)
            {
                std::vector<cv::Point> points;
                for(int j = 0; j < trackerCornersGreen[i].size(); j++){
                    points.push_back(trackerCornersGreen[i].at(j));
                }
                cv::fillConvexPoly(mask, points, cv::Scalar(0, 1, 0), 8, 0);
            }

            tooFar = false;
            trackerBeingProcessed = i;

            cv::aruco::drawAxis(image, camCalib->cameraMatrix, camCalib->distortionCoeffs, trackerStatus[i].boardRvec, trackerStatus[i].boardTvec, 0.1);

            if ((frameNumber++ % (user_config.videoStreams[0]->camera.fps / 3)) != 0) continue;

            // Add camera into parameter block
            parameters.push_back(trackerStatus[i].boardRvec[0]);
            parameters.push_back(trackerStatus[i].boardRvec[1]);
            parameters.push_back(trackerStatus[i].boardRvec[2]);
            parameters.push_back(trackerStatus[i].boardTvec[0]);
            parameters.push_back(trackerStatus[i].boardTvec[1]);
            parameters.push_back(trackerStatus[i].boardTvec[2]);
            parameters.push_back(camCalib->cameraMatrix.at<double>(0,0));
            parameters.push_back(camCalib->cameraMatrix.at<double>(1,1));
            parameters.push_back(camCalib->cameraMatrix.at<double>(0,2));
            parameters.push_back(camCalib->cameraMatrix.at<double>(1,2));
            parameters.push_back(camCalib->distortionCoeffs.at<double>(0,0));
            parameters.push_back(camCalib->distortionCoeffs.at<double>(0,1));
            parameters.push_back(camCalib->distortionCoeffs.at<double>(0,2));
            parameters.push_back(camCalib->distortionCoeffs.at<double>(0,3));
            parameters.push_back(camCalib->distortionCoeffs.at<double>(0,4));
            num_parameters += 15;

            // Add detected points into observations
            for (int j = 0; j < trackerIds.size(); ++j)
            {
                ++frameCountPerMarker[trackerIds[j]];

                for (int k = 0; k < trackerCorners[j].size(); ++k)
                {
                    point_index.push_back((trackerIds[j] - tracker_id_begin) * 4 + k);
                    camera_index.push_back(num_cameras);
                    observations.push_back(trackerCorners[j][k].x);
                    observations.push_back(trackerCorners[j][k].y);
                    num_observations++;
                }
            }

            num_cameras++;
        }

        end = std::chrono::steady_clock::now();
        double frameTime = std::chrono::duration<double>(end - start).count();

        if (gui->IsPreviewVisible())
        {
            // resize image, then show it
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

            cv::multiply(image, mask, image);
            cv::resize(image, outImg, cv::Size(cols, rows));
            cv::putText(outImg, std::to_string(frameTime).substr(0, 5), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
            if (noTracker)
                cv::putText(outImg, std::string("No trackers detected"), cv::Point(10, 70), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
            else if (trackerBeingProcessed == -1)
                cv::putText(outImg, std::string("Tracker too far"), cv::Point(10, 70), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
            else if (tooFar)
                cv::putText(outImg, std::string("Tracker " + std::to_string(trackerBeingProcessed) + " too far"), cv::Point(10, 70), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
            else
                cv::putText(outImg, std::string("Calibrating tracker ") + std::to_string(trackerBeingProcessed), cv::Point(10, 70), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
            cv::putText(outImg, std::string("Frames captured: ") + std::to_string(num_cameras), cv::Point(10, 110), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
            if (showTimeProfile)
            {
                april.drawTimeProfile(outImg, cv::Point(10, 60));
            }
            gui->UpdatePreview(outImg);
        }
        // time of marker detection
    }

    if (gui->IsPreviewVisible())
    {
        outImg = cv::Mat::zeros(outImg.size(), outImg.type());
        cv::putText(outImg, std::string("Processing"), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
        cv::putText(outImg, std::string("Please wait..."), cv::Point(10, 70), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
        gui->UpdatePreview(outImg);
    }

    // Add tracker 3d points into parameter block
    if (trackerBeingProcessed != -1)
    {
        int i = trackerBeingProcessed;

        const std::vector<int>& ids = trackers[i]->ids;
        std::vector<std::vector<cv::Point3f> >& objPoints = trackers[i]->objPoints;

        int first_id = i * markersPerTracker;
        int after_last_id = first_id;

        for (int j = 0; j < ids.size(); ++j)
        {
            if (ids[j] > after_last_id)
                after_last_id = ids[j];
        }

        ++after_last_id;

        for (int j = first_id; j < after_last_id; ++j)
        {
            auto it = std::find(ids.begin(), ids.end(), j);
            auto index = std::distance(ids.begin(), it);

            if (index < objPoints.size())
            {
                for (int k = 0; k < 4; ++k)
                {
                    parameters.push_back(objPoints[index][k].x);
                    parameters.push_back(objPoints[index][k].y);
                    parameters.push_back(objPoints[index][k].z);
                    num_points += 1;
                    num_parameters += 3;
                }
            }
            else
            {
                for (int k = 0; k < 4; ++k)
                {
                    parameters.push_back(0.0);
                    parameters.push_back(0.0);
                    parameters.push_back(0.0);
                    num_points += 1;
                    num_parameters += 3;
                }
            }
        }

        ceres::examples::BALProblem bal_problem(num_cameras, num_points, num_observations, num_parameters, point_index.data(), camera_index.data(), observations.data(), parameters.data());

#ifdef ATT_DEBUG
        bal_problem.WriteToFile("bal_problem.txt");
#endif

        ceres::examples::SolveProblem(bal_problem);

        auto first_point_iter = parameters.begin() + num_cameras * 15;

        for (int j = first_id; j < after_last_id; ++j)
        {
            auto it = std::find(ids.begin(), ids.end(), j);
            auto index = std::distance(ids.begin(), it);

            if (index < objPoints.size())
            {
                for (int k = 0; k < 4; ++k)
                {
                    objPoints[index][k].x = *(first_point_iter + 0 + 3 * (k + 4 * j));
                    objPoints[index][k].y = *(first_point_iter + 1 + 3 * (k + 4 * j));
                    objPoints[index][k].z = *(first_point_iter + 2 + 3 * (k + 4 * j));
                }
            }
        }

        calib_config.Save();
    }

    mainThreadRunning = false;
}

void Tracker::MainLoop()
{

    size_t trackerNum = connection->connectedTrackers.size();
    int numOfPrevValues = user_config.numOfPrevValues;

    SetWorldTransform(user_config.manualCalib.GetAsReal());

    // these variables are used to save detections of apriltags, so we dont define them every frame

    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners;
    std::vector<cv::Point2f> centers;

    FrameData frame;
    cv::Mat drawImg, outImg, ycc, gray, cr;

    cv::Mat prevImg;

    // setup all variables that need to be stored for each tracker and initialize them
    std::vector<TrackerStatus> trackerStatus = std::vector<TrackerStatus>(trackerNum, TrackerStatus());
    for (int i = 0; i < trackerStatus.size(); i++)
    {
        trackerStatus[i].boardFound = false;
        trackerStatus[i].boardRvec = cv::Vec3d(0, 0, 0);
        trackerStatus[i].boardTvec = cv::Vec3d(0, 0, 0);
        trackerStatus[i].prevLocValues = std::vector<std::vector<double>>(7, std::vector<double>());
        trackerStatus[i].last_update_timestamp = std::chrono::milliseconds(0);
        trackerStatus[i].searchSize = (int)(user_config.videoStreams[0]->searchWindow * calib_config.cameras[0]->cameraMatrix.at<double>(0,0));
    }

    // previous values, used for moving median to remove any outliers.
    std::vector<double> prevLocValuesX;

    // the X axis, it is simply numbers 0-10 (or the amount of previous values we have)
    for (int j = 0; j < numOfPrevValues; j++)
    {
        prevLocValuesX.push_back(j);
    }

    AprilTagWrapper april{user_config, aruco_config};

    int framesSinceLastSeen = 0;
    int framesToCheckAll = 20;

    // calculate position of camera from calibration data and send its position to steamvr
    cv::Vec3d stationPos = wtransform.translation();
    CoordTransformOVR(stationPos);

    cv::Quatd stationQ = cv::Quatd(0, 0, 1, 0) * (wrotation * cv::Quatd(1, 0, 0, 0));

    connection->SendStation(0, stationPos[0], stationPos[1], stationPos[2], stationQ[0], stationQ[1], stationQ[2], stationQ[3]);

    // initialize variables for playspace calibration
    bool calibControllerPosActive = false;
    bool calibControllerAngleActive = false;
    auto calibControllerLastPress = std::chrono::steady_clock::now();
    /// offset of controller to virtual camera, set when button is pressed
    cv::Vec3d calibControllerPosOffset = {0, 0, 0};
    /// (pitch, yaw) in radians, 3rd component is distance
    cv::Vec3d calibControllerAngleOffset = {0, 0, 0};

    std::vector<cv::Ptr<cv::aruco::Board>> trackers;
    std::vector<std::vector<int>> boardIds;
    std::vector<std::vector<std::vector<cv::Point3f>>> boardCorners;
    std::vector<cv::Point3f > boardCenters;

    RefPtr<cfg::CameraCalibration> camCalib = calib_config.cameras[0];
    RefPtr<cfg::VideoStream> videoStream = user_config.videoStreams[0];

    for (int i = 0; i < this->trackers.size(); i++)
    {
        boardCorners.push_back(this->trackers[i]->objPoints);
        boardIds.push_back(this->trackers[i]->ids);
        boardCenters.push_back(cv::Point3f(0, 0, 0));

        int numOfCorners = 0;
        for (int j = 0; j < boardCorners[i].size(); j++)
        {
            for (int k = 0; k < boardCorners[i][j].size(); k++)
            {
                boardCenters[i].x += boardCorners[i][j][k].x;
                boardCenters[i].y += boardCorners[i][j][k].y;
                boardCenters[i].z += boardCorners[i][j][k].z;
                numOfCorners++;
            }
        }
        boardCenters[i] /= numOfCorners;
    }

    // by default, trackers have the center at the center of the main marker. If "Use centers of trackers" is checked, we move it to the center of all marker corners.
    if (user_config.trackerCalibCenters)
    {
        for (int i = 0; i < this->trackers.size(); i++)
        {
            for (int j = 0; j < boardCorners[i].size(); j++)
            {
                for (int k = 0; k < boardCorners[i][j].size(); k++)
                {
                    boardCorners[i][j][k] -= boardCenters[i];
                    boardCenters[i] = cv::Point3f(0, 0, 0);
                }
            }

            cv::Ptr<cv::aruco::Board> arBoard = cv::aruco::Board::create(boardCorners[i], cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50), boardIds[i]);
            trackers.push_back(arBoard);
        }
    }
    else
    {
        trackers = this->trackers;
    }

    while (mainThreadRunning && cameraRunning) // run detection until camera is stopped or the start/stop button is pressed again
    {

        CopyFreshCameraImageTo(frame);
        cv::Mat &image = frame.image;
        // shallow copy, gray will be cloned from image and used for detection,
        // so drawing can happen on color image without clone.
        drawImg = image;
        cv::Mat drawImgMasked = cv::Mat::zeros(drawImg.size(), drawImg.type());
        april.convertToSingleChannel(image, gray);

        std::chrono::steady_clock::time_point start, end;
        // for timing our detection
        start = std::chrono::steady_clock::now();

        bool circularWindow = videoStream->circularWindow;

        //last is if pose is valid: 0 is valid, 1 is late (hasnt been updated for more than 0.2 secs), -1 means invalid and is only zeros
        std::vector<int> tracker_pose_valid = std::vector<int>(trackerNum, -1);

        // if any tracker was lost for longer than 20 frames, mark circularWindow as false
        for (int i = 0; i < trackerNum; i++)
        {
            if (!trackerStatus[i].boardFound)
            {
                framesSinceLastSeen++;
                if (framesSinceLastSeen > framesToCheckAll)
                    circularWindow = false;
                break;
            }
        }
        if (!circularWindow)
            framesSinceLastSeen = 0;

        for (int i = 0; i < trackerNum; i++)
        {

            double frameTime = std::chrono::duration<double>(std::chrono::steady_clock::now() - frame.time).count();

            std::string word;
            std::istringstream ret = connection->Send("gettrackerpose " + std::to_string(i) + " " + std::to_string(-frameTime - videoStream->latency));
            ret >> word;
            if (word != "trackerpose")
            {
                continue;
            }

            // first three variables are a position vector
            int idx;
            double a;
            double b;
            double c;

            // second four are rotation quaternion
            double qw;
            double qx;
            double qy;
            double qz;

            // read to our variables
            ret >> idx;
            ret >> a;
            ret >> b;
            ret >> c;
            ret >> qw;
            ret >> qx;
            ret >> qy;
            ret >> qz;
            ret >> tracker_pose_valid[i];

            cv::Vec3d rpos{a, b, c};
            CoordTransformOVR(rpos);

            // transform boards position from steamvr space to camera space based on our calibration data

            rpos = wtransform.inv() * rpos;

            cv::Vec3d rvec;
            cv::Vec3d tvec;

            if (tracker_pose_valid[i] == 0)
            {
                cv::Quatd q{qw, qx, qy, qz};

                /// TODO: What is this?
                q = wrotation.inv(cv::QUAT_ASSUME_UNIT) *
                    cv::Quatd(0, 0, 1, 0).inv(cv::QUAT_ASSUME_UNIT) *
                    q.normalize() *
                    cv::Quatd(0, 0, 1, 0).inv(cv::QUAT_ASSUME_UNIT);

                rvec = q.toRotVec();
                tvec = cv::Vec3d(rpos[0], rpos[1], rpos[2]);
            }
            else {
                tvec = trackerStatus[i].boardTvec;
                rvec = trackerStatus[i].boardRvec;
            }

            std::vector<cv::Point3f> offsetin, offsetout;
            offsetin.push_back(boardCenters[i]);
            offsetFromBoardToCameraSpace(offsetin, rvec, tvec, &offsetout);

            std::vector<cv::Point3d> point;
            point.push_back(cv::Point3d(rpos[0], rpos[1], rpos[2]));
            point.push_back(cv::Point3d(trackerStatus[i].boardTvec));
            point.push_back(cv::Point3d(rpos[0], rpos[1], rpos[2]) + cv::Point3d(offsetout[0]));
            point.push_back(cv::Point3d(trackerStatus[i].boardTvec) + cv::Point3d(offsetout[0]));

            std::vector<cv::Point2d> projected;
            cv::Vec3d identity_rvec, identity_tvec;

            // project point from position of tracker in camera 3d space to 2d camera pixel space, and draw a dot there
            cv::projectPoints(point, identity_rvec, identity_tvec, camCalib->cameraMatrix, camCalib->distortionCoeffs, projected);

            cv::circle(drawImg, projected[0], 5, cv::Scalar(0, 0, 255), 2, 8, 0);
            cv::circle(drawImgMasked, projected[0], 5, cv::Scalar(0, 0, 255), 2, 8, 0);
            cv::circle(drawImg, projected[2], 5, cv::Scalar(0, 255, 255), 2, 8, 0);
            cv::circle(drawImgMasked, projected[2], 5, cv::Scalar(0, 255, 255), 2, 8, 0);
            cv::circle(drawImg, projected[3], 5, cv::Scalar(255, 0, 255), 2, 8, 0);
            cv::circle(drawImgMasked, projected[3], 5, cv::Scalar(255, 0, 255), 2, 8, 0);

            if (tracker_pose_valid[i] == 0) // if the pose from steamvr was valid, save the predicted position and rotation
            {
                if (!trackerStatus[i].boardFound) // if tracker was found in previous frame, we use that position for masking. If not, we use position from driver for masking.
                {
                    trackerStatus[i].maskCenter = projected[2];
                    trackerStatus[i].searchSize = (int)(user_config.videoStreams[0]->searchWindow * calib_config.cameras[0]->cameraMatrix.at<double>(0,0)/point[2].z);
                }
                else
                {
                    trackerStatus[i].maskCenter = projected[3];
                    trackerStatus[i].searchSize = (int)(user_config.videoStreams[0]->searchWindow * calib_config.cameras[0]->cameraMatrix.at<double>(0,0)/point[3].z);
                }

                trackerStatus[i].boardFound = true;
                trackerStatus[i].boardFoundDriver = true;
                trackerStatus[i].boardTvec = tvec;
                trackerStatus[i].boardTvecDriver = tvec;
                trackerStatus[i].boardRvec = rvec;
            }
            else
            {
                if (trackerStatus[i].boardFound) // if pose is not valid, set everything based on previous known position
                {
                    trackerStatus[i].maskCenter = projected[3];
                    trackerStatus[i].searchSize = (int)(user_config.videoStreams[0]->searchWindow * calib_config.cameras[0]->cameraMatrix.at<double>(0,0)/point[3].z);
                }

                trackerStatus[i].boardFoundDriver = false; // do we really need to do this? might be unnecessary
            }
        }

        // define our mask image. We want to create an image where everything but circles around predicted tracker positions will be black to speed up detection.
        cv::Mat mask = cv::Mat::zeros(gray.size(), gray.type());

        cv::Mat dstImage = cv::Mat::zeros(gray.size(), gray.type());

        bool doMasking = false;

        for (int i = 0; i < trackerNum; i++) // calculate the needed masks for every tracker
        {
            if (trackerStatus[i].maskCenter.x <= 0 || trackerStatus[i].maskCenter.y <= 0 || trackerStatus[i].maskCenter.x >= image.cols || trackerStatus[i].maskCenter.y >= image.rows)
            {
                trackerStatus[i].boardFound = false; // if detected tracker is out of view of the camera, we mark it as not found, as either the prediction is wrong or we wont see it anyway
                continue;
            }
            doMasking = true;
            if (circularWindow) // if circular window is set mask a circle around the predicted tracker point
            {
                cv::circle(mask, trackerStatus[i].maskCenter, trackerStatus[i].searchSize, cv::Scalar(255, 0, 0), -1, 8, 0);
                cv::circle(drawImg, trackerStatus[i].maskCenter, trackerStatus[i].searchSize, cv::Scalar(255, 0, 0), 2, 8, 0);
                cv::circle(drawImgMasked, trackerStatus[i].maskCenter, trackerStatus[i].searchSize, cv::Scalar(255, 0, 0), 2, 8, 0);
            }
            else // if not, mask a vertical strip top to bottom. This happens every 20 frames if a tracker is lost.
            {
                int maskCenter = static_cast<int>(std::trunc(trackerStatus[i].maskCenter.x));
                auto lefttop = cv::Point(maskCenter - trackerStatus[i].searchSize, 0);
                auto bottomright = cv::Point(maskCenter + trackerStatus[i].searchSize, image.rows);
                rectangle(mask, lefttop, bottomright, cv::Scalar(255, 0, 0), -1);
                rectangle(drawImg, lefttop, bottomright, cv::Scalar(255, 0, 0), 3);
                rectangle(drawImgMasked, lefttop, bottomright, cv::Scalar(255, 0, 0), 3);
            }
        }

        // using copyTo with masking creates the image where everything but the locations where trackers are predicted to be is black
        if (doMasking)
        {
            gray.copyTo(dstImage, mask);
            gray = dstImage;
        }

        if (manualRecalibrate) // playspace calibration loop
        {
            int inputButton = connection->GetButtonStates();
            cfg::ManualCalib::Real calib = gui->GetManualCalib();

            double timeSincePress = std::chrono::duration<double>(start - calibControllerLastPress).count();
            if (timeSincePress > 60) // we exit playspace calibration after 60 seconds of no input detected, to try to prevent accidentaly ruining calibration
            {
                gui->SetManualCalibVisible(false);
            }

            if (inputButton == 1) // logic for position button
            {
                // to prevent accidental double presses, 0.2 seconds must pass between presses.
                double timeSincePress = std::chrono::duration<double>(start - calibControllerLastPress).count();
                if (timeSincePress >= 0.2)
                {
                    if (!calibControllerPosActive) // if position calibration is inactive, set it to active and calculate offsets between the camera and controller
                    {
                        calibControllerPosActive = true;
                        Pose pose = connection->GetControllerPose();
                        calibControllerPosOffset = pose.position - calib.posOffset;

                        RotateVecByQuat(calibControllerPosOffset, pose.rotation.inv(cv::QUAT_ASSUME_UNIT));
                    }
                    else // else, deactivate it
                    {
                        calibControllerPosActive = false;
                    }
                }
                calibControllerLastPress = std::chrono::steady_clock::now();
            }
            else if (inputButton == 2) // logic for rotation button
            {
                // to prevent accidental double presses, 0.2 seconds must pass between presses.
                double timeSincePress = std::chrono::duration<double>(start - calibControllerLastPress).count();
                if (timeSincePress >= 0.2)
                {
                    if (!calibControllerAngleActive) // if rotation calibration is inactive, set it to active and calculate angle offsets and distance
                    {
                        calibControllerAngleActive = true;
                        Pose pose = connection->GetControllerPose();

                        cv::Vec2d angle = EulerAnglesFromPos(pose.position, calib.posOffset);
                        double distance = Distance(pose.position, calib.posOffset);

                        calibControllerAngleOffset[0] = angle[0] - calib.angleOffset[0];
                        calibControllerAngleOffset[1] = angle[1] - calib.angleOffset[1];
                        calibControllerAngleOffset[2] = distance / calib.scale;
                    }
                    else // else, deactivate it
                    {
                        calibControllerAngleActive = false;
                    }
                }
                calibControllerLastPress = std::chrono::steady_clock::now();
            }

            if (calibControllerPosActive) // while position calibration is active, apply the camera to controller offset to X, Y and Z values
            {
                Pose pose = connection->GetControllerPose();

                RotateVecByQuat(calibControllerPosOffset, pose.rotation);

                calib.posOffset[0] = pose.position[0] - calibControllerPosOffset[0];
                if (!lockHeightCalib)
                {
                    calib.posOffset[1] = pose.position[1] - calibControllerPosOffset[1];
                }
                calib.posOffset[2] = pose.position[2] - calibControllerPosOffset[2];

                RotateVecByQuat(calibControllerPosOffset, pose.rotation.inv(cv::QUAT_ASSUME_UNIT));
            }

            if (calibControllerAngleActive) // while rotation calibration is active, apply the camera to controller angle offsets to A, B, C values, and apply the calibScale based on distance from camera
            {
                Pose pose = connection->GetControllerPose();
                cv::Vec2d angle = EulerAnglesFromPos(pose.position, calib.posOffset);
                double distance = Distance(pose.position, calib.posOffset);

                calib.angleOffset[1] = angle[1] - calibControllerAngleOffset[1];
                if (!lockHeightCalib) // if height is locked, do not calibrate up/down rotation or scale
                {
                    calib.angleOffset[0] = angle[0] - calibControllerAngleOffset[0];
                    calib.scale = distance / calibControllerAngleOffset[2];
                }
            }

            // check that camera is facing correct direction. 90 degrees mean looking straight down, 270 is straight up. This ensures its not upside down.
            if (calib.angleOffset[0] < 91 * DEG_2_RAD)
                calib.angleOffset[0] = 91 * DEG_2_RAD;
            else if (calib.angleOffset[0] > 269 * DEG_2_RAD)
                calib.angleOffset[0] = 269 * DEG_2_RAD;

            if (calibControllerAngleActive || calibControllerPosActive)
            {
                // update the calib in the gui as it wont be modified anymore
                gui->SetManualCalib(calib);
            }
            // update the pre calculated calibration transformations
            SetWorldTransform(gui->GetManualCalib());

            cv::Vec3d stationPos = wtransform.translation();
            CoordTransformOVR(stationPos);
            // rotate y axis by 180 degrees, aka, reflect across xz plane
            cv::Quatd stationQ = cv::Quatd(0, 0, 1, 0) * wrotation;

            // move the camera in steamvr to new calibration
            connection->SendStation(0, stationPos[0], stationPos[1], stationPos[2], stationQ.w, stationQ.x, stationQ.y, stationQ.z);
        }
        else
        {
            calibControllerLastPress = std::chrono::steady_clock::now();
        }
        april.detectMarkers(gray, &corners, &ids, &centers, trackers);
        std::chrono::milliseconds current_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        for (int i = 0; i < trackerNum; ++i)
        {

            // estimate the pose of current board

            try
            {
                trackerStatus[i].boardTvec /= wscale;
                if (cv::aruco::estimatePoseBoard(corners, ids, trackers[connection->connectedTrackers[i].TrackerId], camCalib->cameraMatrix, camCalib->distortionCoeffs, trackerStatus[i].boardRvec, trackerStatus[i].boardTvec, trackerStatus[i].boardFound && user_config.usePredictive) <= 0)
                {
                    for (int j = 0; j < 6; j++)
                    {
                        // remove first of the previously saved values
                        if (trackerStatus[i].prevLocValues[j].size() > 0)
                            trackerStatus[i].prevLocValues[j].erase(trackerStatus[i].prevLocValues[j].begin());
                    }
                    trackerStatus[i].boardFound = false;

                    continue;
                }
                else
                {
                    if ((tracker_pose_valid[i] != 0) || ((current_timestamp.count() - trackerStatus[i].last_update_timestamp.count()) <= 100.0))
                    {
                        trackerStatus[i].last_update_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
                    }

                }
            }
            catch (const std::exception& e)
            {
                // on rare occasions, detection crashes. Should be very rare and indicate something wrong with camera or tracker calibration
                ATT_LOG_ERROR(e.what());
                gui->ShowPopup(lc.TRACKER_DETECTION_SOMETHINGWRONG, PopupStyle::Error);
                mainThreadRunning = false;
                return;
            }
            trackerStatus[i].boardFound = true;

            trackerStatus[i].boardTvec *= wscale;

            if (user_config.depthSmoothing > 0 && trackerStatus[i].boardFoundDriver && !manualRecalibrate)
            {
                // depth estimation is noisy, so try to smooth it more, especialy if using multiple cameras
                // if position is close to the position predicted by the driver, take the depth of the driver.
                // if error is big, take the calculated depth
                // error threshold is defined in the params as depth smoothing

                double distDriver = Length(trackerStatus[i].boardTvecDriver);
                double distPredict = Length(trackerStatus[i].boardTvec);

                cv::Vec3d normPredict = trackerStatus[i].boardTvec / distPredict;

                double dist = std::abs(distPredict - distDriver);

                dist = dist / user_config.depthSmoothing + 0.1;
                if (dist > 1)
                    dist = 1;

                double distFinal = (dist)*distPredict + (1 - dist) * distDriver;

                trackerStatus[i].boardTvec = normPredict * distFinal;

                // trackerStatus[i].boardTvec[2] = (dist) * trackerStatus[i].boardTvec[2] + (1-dist) * trackerStatus[i].boardTvecDriver[2];
            }

            double posValues[6] = {
                trackerStatus[i].boardTvec[0],
                trackerStatus[i].boardTvec[1],
                trackerStatus[i].boardTvec[2],
                trackerStatus[i].boardRvec[0],
                trackerStatus[i].boardRvec[1],
                trackerStatus[i].boardRvec[2]};

            for (int j = 0; j < 6; j++)
            {
                // push new values into previous values list end and remove the one on beggining
                trackerStatus[i].prevLocValues[j].push_back(posValues[j]);
                if (trackerStatus[i].prevLocValues[j].size() > numOfPrevValues)
                {
                    trackerStatus[i].prevLocValues[j].erase(trackerStatus[i].prevLocValues[j].begin());
                }

                std::vector<double> valArray(trackerStatus[i].prevLocValues[j]);
                // sort(valArray.begin(), valArray.end());

                // posValues[j] = valArray[valArray.size() / 2];
                posValues[j] = valArray[valArray.size() - 1];
            }
            // save fitted values back to our variables
            trackerStatus[i].boardTvec[0] = posValues[0];
            trackerStatus[i].boardTvec[1] = posValues[1];
            trackerStatus[i].boardTvec[2] = posValues[2];
            trackerStatus[i].boardRvec[0] = posValues[3];
            trackerStatus[i].boardRvec[1] = posValues[4];
            trackerStatus[i].boardRvec[2] = posValues[5];

            cv::Vec3d rpos = trackerStatus[i].boardTvec;

            // transform boards position based on our calibration data
            rpos = wtransform * rpos;
            CoordTransformOVR(rpos);

            // convert rodriguez rotation to quaternion
            cv::Quatd q = cv::Quatd::createFromRvec(trackerStatus[i].boardRvec);

            cv::aruco::drawAxis(drawImg, camCalib->cameraMatrix, camCalib->distortionCoeffs, trackerStatus[i].boardRvec, trackerStatus[i].boardTvec, 0.1);
            cv::aruco::drawAxis(drawImgMasked, camCalib->cameraMatrix, camCalib->distortionCoeffs, trackerStatus[i].boardRvec, trackerStatus[i].boardTvec, 0.1);

            q = cv::Quatd{0, 0, 1, 0} * (wrotation * q) * cv::Quatd{0, 0, 1, 0};

            double factor = user_config.smoothingFactor;

            if (factor < 0)
                factor = 0;
            else if (factor >= 1)
                factor = 0.99;

            end = std::chrono::steady_clock::now();
            double frameTime = std::chrono::duration<double>(end - frame.time).count();

            // Reject detected positions that are behind the camera
            if (trackerStatus[i].boardTvec[2]  < 0)
            {
                trackerStatus[i].boardFound = false;
                continue;
            }

            // Figure out the camera aspect ratio, XZ and YZ ratio limits
            double aspectRatio = static_cast<double>(image.cols) / static_cast<double>(image.rows);
            double XZratioLimit = 0.5 * static_cast<double>(image.cols) / camCalib->cameraMatrix.at<double>(0,0);
            double YZratioLimit = 0.5 * static_cast<double>(image.rows) / camCalib->cameraMatrix.at<double>(1,1);

            // Figure out whether X or Y dimension is most likely to go outside the camera field of view
            if (std::abs(trackerStatus[i].boardTvec[0] / trackerStatus[i].boardTvec[1]) > aspectRatio)
            {
                // Reject detections when XZ coordinate ratio goes out of camera FOV
                if (std::abs(trackerStatus[i].boardTvec[0] / trackerStatus[i].boardTvec[2]) > XZratioLimit)
                {
                    trackerStatus[i].boardFound = false;
                    continue;
                }
            }
            else
            {
                // Reject detections when YZ coordinate ratio goes out of camera FOV
                if (std::abs(trackerStatus[i].boardTvec[1] / trackerStatus[i].boardTvec[2]) > YZratioLimit)
                {
                    trackerStatus[i].boardFound = false;
                    continue;
                }
            }

            // Don't send tracker if more than 100 miliseconds have passed since tracker was last detected
            if ((current_timestamp.count() - trackerStatus[i].last_update_timestamp.count()) > 100.0)
                continue;

            // send all the values
            // frame time is how much time passed since frame was acquired.
            if (!multicamAutocalib)
                connection->SendTracker(connection->connectedTrackers[i].DriverId, rpos[0], rpos[1], rpos[2], q.w, q.x, q.y, q.z, -frameTime - videoStream->latency, factor);
            else if (trackerStatus[i].boardFoundDriver)
            {
                // if calibration refinement with multiple cameras is active, do not send calculated poses to driver.
                // instead, refine the calibration data with gradient descent
                // the error is the diffrence of the detected trackers position to the estimated trackers position
                // numerical derivatives are then calculated to see how X,Y,Z, A,B, scale data affects the error in position
                // calibration values are then slightly changed in the estimated direction in order to reduce error.
                // after a couple of seconds, the calibration data should converge

                cv::Vec3d pos = trackerStatus[i].boardTvec;
                cv::Vec2d angles = EulerAnglesFromPos(pos);
                double length = Length(pos);

                cv::Vec3d driverPos = trackerStatus[i].boardTvecDriver;
                cv::Vec2d driverAngles = EulerAnglesFromPos(driverPos);
                double driverLength = Length(driverPos);

                pos = wtransform * pos;
                driverPos = wtransform * driverPos;

                CoordTransformOVR(pos);
                CoordTransformOVR(driverPos);

                double dX = (pos[0] - driverPos[0]) * 0.1;
                double dY = -(pos[1] - driverPos[1]) * 0.1;
                double dZ = (pos[2] - driverPos[2]) * 0.1;

                if (abs(dX) > 0.01)
                    dX = 0.1 * (dX / abs(dX));
                if (abs(dY) > 0.1)
                    dY = 0.1 * (dY / abs(dY));
                if (abs(dZ) > 0.1)
                    dZ = 0.1 * (dZ / abs(dZ));

                cfg::ManualCalib::Real calib = gui->GetManualCalib();

                calib.posOffset += cv::Vec3d(dX, dY, dZ);
                calib.angleOffset += cv::Vec3d(angles[0] - driverAngles[0], angles[1] - driverAngles[1]) * 0.1;
                calib.scale -= (1 - (driverLength / length)) * 0.1;

                gui->SetManualCalib(calib);
                SetWorldTransform(calib);
            }
        }

        // Copy only the inner white squares of detected markers into drawImgMasked
        if (corners.size() > 0) {
            cv::Mat mask = cv::Mat::zeros(image.size(), image.type());

            for (int i = 0; i < corners.size(); i++)
            {
                std::vector<cv::Point> points;
                for(int j = 0; j < corners[i].size(); j++){
                    points.push_back(corners[i].at(j));
                }
                cv::fillConvexPoly(mask, points, cv::Scalar(255, 255, 255), 8, 0);
            }

            drawImg.copyTo(drawImgMasked, mask);
        }

        // draw and display the detections
        if (ids.size() > 0) {
            cv::aruco::drawDetectedMarkers(drawImg, corners, ids);
            cv::aruco::drawDetectedMarkers(drawImgMasked, corners, ids);
        }

        end = std::chrono::steady_clock::now();
        double frameTime = std::chrono::duration<double>(end - start).count();

        if (gui->IsPreviewVisible())
        {
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

            if (privacyMode)
                cv::resize(drawImgMasked, outImg, cv::Size(cols, rows));
            else
                cv::resize(drawImg, outImg, cv::Size(cols, rows));
            cv::putText(outImg, std::to_string(frameTime).substr(0, 5), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
            if (showTimeProfile)
            {
                april.drawTimeProfile(outImg, cv::Point(10, 60));
            }
            gui->UpdatePreview(outImg);
        }
        // time of marker detection
    }
}
