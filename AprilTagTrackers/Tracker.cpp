#include "Tracker.hpp"

#include "AprilTagWrapper.hpp"
#include "config/TrackerUnit.hpp"
#include "Helpers.hpp"
#include "ImageDrawing.hpp"
#include "math/CVHelpers.hpp"
#include "tracker/TrackerUnit.hpp"
#include "utils/Assert.hpp"
#include "utils/LogBatch.hpp"
#include "utils/SteadyTimer.hpp"
#include "utils/Types.hpp"

#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>
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
    : mCapture(&_userConfig.videoStreams[0]->camera),
      user_config(_userConfig), calib_config(_calibConfig), aruco_config(_arucoConfig), lc(_lc)
{
    SetTrackerUnitsFromConfig();
    SetWorldTransform(user_config.manualCalib.GetAsReal());
}

void Tracker::StartCamera(RefPtr<cfg::Camera> cam)
{
    if (cameraRunning)
    {
        cameraRunning = false;
        mainThreadRunning = false;
        cameraThread.join();
        return;
    }

    if (!mCapture.TryOpen())
    {
        gui->ShowPopup(lc.TRACKER_CAMERA_START_ERROR, PopupStyle::Error);
        return;
    }

    // ensure joined before creating new thread
    if (cameraThread.joinable())
    {
        cameraThread.join();
    }

    cameraRunning = true;
    cameraThread = std::thread(&Tracker::CameraLoop, this);
}

void Tracker::StartCamera()
{
    StartCamera(&user_config.videoStreams[0]->camera);
}

void Tracker::CameraLoop()
{
    const RefPtr<cfg::Camera> cam = &user_config.videoStreams[0]->camera;

    cv::Mat img;
    cv::Mat drawImg;

    utils::SteadyTimer fpsTimer{};
    int frameCount = 0;
    int fps = 0;

    utils::SteadyTimer previewTimer{};
    gui->SetStatus(true, StatusItem::Camera);

    while (cameraRunning)
    {
        const auto stampBeforeCap = utils::SteadyTimer::Now();
        const utils::NanoS frameTime = mFrameTimer.Get(stampBeforeCap);
        mFrameTimer.Restart(stampBeforeCap);

        if (!mCapture.TryReadFrame(img))
        {
            gui->ShowPopup(lc.TRACKER_CAMERA_ERROR, PopupStyle::Error);
            cameraRunning = false;
            break;
        }
        const auto stampAfterCap = utils::SteadyTimer::Now();

        // framerate limiter
        // constexpr int minSafeFps = 300;
        // constexpr auto minSafeCaptureTime = duration_cast<utils::NanoS>(utils::PerSecond(utils::Seconds(minSafeFps)));
        // if (frameTime < minSafeCaptureTime)
        // {
        //     std::this_thread::sleep_for(utils::MilliS(10));
        // }

        // fps counter
        if (fpsTimer.Get(stampAfterCap) < utils::Seconds(1))
        {
            ++frameCount;
        }
        else
        {
            fpsTimer.Restart(stampAfterCap);
            fps = frameCount;
            frameCount = 0;
        }

        // fps = (0.95 * fps) + (0.05 * utils::PerSecond(frameTime).count());
        const utils::NanoS captureTime = mFrameTimer.Get(stampAfterCap);

        // Ensure that preview isnt shown more than 60 times per second.
        // In some cases opencv will return a solid color image without any blocking delay (unlike a camera locked to a framerate),
        // and we would just be drawing fps text on nothing.
        if ((previewTimer.Get(stampAfterCap) * 60) > utils::Seconds(1))
        {
            if (gui->IsPreviewVisible(PreviewId::Camera))
            {
                previewTimer.Restart(stampAfterCap);
                img.copyTo(drawImg);
                cv::putText(drawImg, std::to_string(fps),
                            cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 255, 0), 2);
                const std::string resolution = std::to_string(img.cols) + "x" + std::to_string(img.rows);
                cv::putText(drawImg, resolution, cv::Point(10, 120), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 255, 0), 2);
                if (previewCameraCalibration) drawCalibration(drawImg, *calib_config.cameras[0]);
                gui->UpdatePreview(drawImg, PreviewId::Camera);
            }
        }
        { // lock scope
            std::lock_guard lock{mCameraImageMutex};
            // Swap avoids copying the pixel buffer. It only swaps pointers and metadata.
            // The pixel buffer from cameraImage can be reused if the size and format matches.
            cv::swap(img, mCameraFrame.image);
            mCameraFrame.timestamp = stampAfterCap;
            // TODO: does opencv really care if img.read is called on the wrong size of matrix?
            // if (img.size() != cameraImage.size() || img.flags != cameraImage.flags)
            // {
            //     img.release();
            // }
            mIsImageReady = true;
        }
        mImageReadyCond.notify_one();

        if (mVRClient && mVRClient->IsInit())
        {
            mVRClient->PollEvents();
        }
    }
    mCapture.Close();
    gui->SetStatus(false, StatusItem::Camera);
}

void Tracker::CopyFreshCameraImageTo(CapturedFrame& frame)
{
    std::unique_lock lock{mCameraImageMutex};
    mImageReadyCond.wait(lock, [&] { return mIsImageReady; });

    mIsImageReady = false;
    cv::swap(frame.image, mCameraFrame.image);
    frame.timestamp = mCameraFrame.timestamp;
}

void Tracker::StartCameraCalib()
{
    if (mainThreadRunning)
    {
        mainThreadRunning = false;
        mainThread.join();
        return;
    }
    if (!cameraRunning)
    {
        gui->ShowPopup(lc.TRACKER_CAMERA_NOTRUNNING, PopupStyle::Error);
        mainThreadRunning = false;
        return;
    }

    // ensure joined before creating new thread
    if (mainThread.joinable())
    {
        mainThread.join();
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
}

/// function to calibrate our camera
void Tracker::CalibrateCameraCharuco()
{
    CapturedFrame frame;
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
                    [&](bool pressedOk) {
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
        const cv::Size2i drawSize = math::ConstrainSize(math::GetMatSize(frame.image), DRAW_IMG_SIZE);
        frame.image.copyTo(drawImg);
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
                cv::aruco::calibrateCameraCharuco(allCharucoCorners, allCharucoIds, board, math::GetMatSize(frame.image),
                                                  cameraMatrix, distCoeffs, R, T, stdDeviationsIntrinsics, stdDeviationsExtrinsics, perViewErrors,
                                                  cv::CALIB_USE_LU);

                picsTaken--;
            }
        }

        cvtColor(frame.image, gray, cv::COLOR_BGR2GRAY);
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

        preview.Update(outImg, DRAW_IMG_SIZE);

        // if more than one second has passed since last calibration image, add current frame to calibration images
        // framesSinceLast++;
        if (std::chrono::duration<double>(std::chrono::steady_clock::now() - timeOfLast).count() > 1)
        {
            // framesSinceLast = 0;
            timeOfLast = std::chrono::steady_clock::now();
            // if any button was pressed

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
                            cv::aruco::calibrateCameraCharuco(allCharucoCorners, allCharucoIds, board, math::GetMatSize(frame.image),
                                                              cameraMatrix, distCoeffs, R, T, stdDeviationsIntrinsics, stdDeviationsExtrinsics, perViewErrors,
                                                              cv::CALIB_USE_LU);
                        }
                        catch (const cv::Exception& e)
                        {
                            ATT_LOG_ERROR(e.what());
                        }

                        std::size_t curI = perViewErrors.size();
                    }
                }
            }
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
            RefPtr<cfg::CameraCalib> camCalib = calib_config.cameras[0];
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

    CapturedFrame frame;
    cv::Mat outImg;

    int i = 0;
    int framesSinceLast = -100;

    int picNum = user_config.cameraCalibSamples;

    cv::Size2i imageSize;

    while (i < picNum)
    {
        if (!mainThreadRunning || !cameraRunning)
        {
            return;
        }
        CopyFreshCameraImageTo(frame);
        cv::Mat& image = frame.image;
        cv::putText(image, std::to_string(i) + "/" + std::to_string(picNum), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
        const cv::Size2i drawSize = math::ConstrainSize(math::GetMatSize(image), DRAW_IMG_SIZE);

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

        cv::resize(image, outImg, drawSize);
        gui->UpdatePreview(outImg);

        imageSize = math::GetMatSize(frame.image);
    }

    cv::Mat cameraMatrix, distCoeffs, R, T;

    calibrateCamera(objpoints, imgpoints, imageSize, cameraMatrix, distCoeffs, R, T);

    RefPtr<cfg::CameraCalib> camCalib = calib_config.cameras[0];

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
        mainThread.join();
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

    // ensure joined before creating new thread
    if (mainThread.joinable())
    {
        mainThread.join();
    }
    // start tracker calibration on another thread
    mainThreadRunning = true;
    mainThread = std::thread(&Tracker::CalibrateTracker, this);
}

void Tracker::StartConnection()
{
    mVRDriver = tracker::VRDriver{user_config.trackers};
    if (!user_config.disableOpenVrApi)
    {
        mVRClient = std::make_unique<tracker::OpenVRClient>();
    }
    else
    {
        mVRClient = std::make_unique<tracker::MockOpenVRClient>();
    }
    if (!mVRClient->CanInit())
    {
        gui->ShowPopup("Unable to initialize steamvr client, is your hmd connected?", PopupStyle::Error);
        return;
    }
    mVRClient->Init();
    gui->SetStatus(true, StatusItem::Driver);
}

// void Tracker::HandleConnectionErrors()
// {
//     using Code = Connection::ErrorCode;
//     Code code = connection->GetAndResetErrorState();
//     if (code == Code::OK)
//         return;

//     gui->SetStatus(false, StatusItem::Driver);

//     if (code == Code::ALREADY_WAITING)
//         gui->ShowPopup("Already waiting for a connection.", PopupStyle::Error);
//     else if (code == Code::ALREADY_CONNECTED)
//         gui->ShowPopup("Connection closed.", PopupStyle::Info);
//     else if (code == Code::CLIENT_ERROR)
//         gui->ShowPopup(lc.CONNECT_CLIENT_ERROR + connection->GetErrorMsg(), PopupStyle::Error);
//     else if (code == Code::BINDINGS_MISSING)
//         gui->ShowPopup(lc.CONNECT_BINDINGS_ERROR, PopupStyle::Error);
//     else if (code == Code::DRIVER_ERROR)
//         gui->ShowPopup(lc.CONNECT_DRIVER_ERROR, PopupStyle::Error);
//     else if (code == Code::DRIVER_MISMATCH)
//         gui->ShowPopup(lc.CONNECT_DRIVER_MISSMATCH_1 + connection->GetErrorMsg() +
//                            lc.CONNECT_DRIVER_MISSMATCH_2 + utils::GetBridgeDriverVersion().ToString(),
//                        PopupStyle::Error);
//     else // if (code == Code::SOMETHING_WRONG)
//         gui->ShowPopup(lc.CONNECT_SOMETHINGWRONG, PopupStyle::Error);
// }
// TODO: rename to SetPlayspaceCalib
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
        mainThread.join();
        return;
    }
    if (!cameraRunning)
    {
        gui->ShowPopup(lc.TRACKER_CAMERA_NOTRUNNING, PopupStyle::Error);
        mainThreadRunning = false;
        mainThread.join();
        return;
    }
    if (calib_config.cameras[0]->cameraMatrix.empty())
    {
        gui->ShowPopup(lc.TRACKER_CAMERA_NOTCALIBRATED, PopupStyle::Error);
        mainThreadRunning = false;
        mainThread.join();
        return;
    }
    if (!IsTrackerUnitsCalibrated())
    {
        gui->ShowPopup(lc.TRACKER_TRACKER_NOTCALIBRATED, PopupStyle::Error);
        mainThreadRunning = false;
        mainThread.join();
        return;
    }
    if (!mVRClient->IsInit() || !mVRDriver)
    {
        gui->ShowPopup(lc.TRACKER_STEAMVR_NOTCONNECTED, PopupStyle::Error);
        mainThreadRunning = false;
        mainThread.join();
        return;
    }

    gui->SetStatus(true, StatusItem::Tracker);

    // ensure joined before creating new thread
    if (mainThread.joinable())
    {
        mainThread.join();
    }

    // start detection on another thread
    mainThreadRunning = true;
    mainThread = std::thread(&Tracker::MainLoop, this);
}

void Tracker::Stop()
{
    mainThreadRunning = false;
    cameraRunning = false;

    if (cameraThread.joinable()) cameraThread.join();
    if (mainThread.joinable()) mainThread.join();
}

void Tracker::UpdateConfig()
{
    if (mVRDriver)
    {
        mVRDriver->SetSmoothing(user_config.smoothingFactor, user_config.additionalSmoothing);
    }
}

void Tracker::CalibrateTracker()
{
    utils::RegisterThisThreadName("Calibrate Tracker");

    bool promptSaveCalib = false;
    gui->ShowPrompt(lc.TRACKER_TRACKER_CALIBRATION_INSTRUCTIONS,
                    [&](bool pressedOk) {
                        promptSaveCalib = pressedOk;
                        mainThreadRunning = false;
                    });

    // initialize all parameters needed for tracker calibration
    std::vector<tracker::TrackerUnit> trackerUnits;

    AprilTagWrapper april{AprilTagWrapper::ConvertFamily(user_config.markerLibrary), user_config.videoStreams[0]->quadDecimate, 4};
    MarkerDetectionList dets{};

    const Index trackerNum = user_config.trackerNum;
    const int markersPerTracker = user_config.markersPerTracker;
    const double markerSize = user_config.markerSize * 0.01; // centimeters to meters

    const MarkerCorners3f modelMarker = tracker::TrackerUnit::CreateModelMarker(markerSize);
    constexpr int numCachedToAdd = 50;
    /// maps marker id to many copies of its corners
    std::unordered_map<int, std::vector<MarkerCorners3f>> markersCache;

    // add main marker for every tracker
    for (int i = 0; i < trackerNum; i++)
    {
        tracker::TrackerUnit unit;
        // TODO: dynamically pick the main marker, based on the first seen? need some gui to help as multiple marker tend to get detected in the background while calibrating.
        // might be helpful to draw the id of the marker on each detected, and then some gui to select which detected marker is the main, and which should be added to this one.
        // it should be easy to detect if two markers are moving together, and separate from one not moving in the background
        const int id = i * user_config.markersPerTracker;
        unit.AddMarker(id, modelMarker);
        trackerUnits.push_back(std::move(unit));
    }

    CapturedFrame frame;
    cv::Mat grayImage;

    math::EstimatePoseSingleMarkersResult markerPoses;
    const RefPtr<cfg::CameraCalib> camCalib = calib_config.cameras[0];
    auto preview = gui->CreatePreviewControl();

    // TODO: temporary make code easier by allowing returns and handling exceptions properly within loop
    // will be refactored to another class, but easier than pulling out to another function due to amount of state
    const auto doStep = [&] {
        CopyFreshCameraImageTo(frame);
        // detect and draw all markers on image
        AprilTagWrapper::ConvertGrayscale(frame.image, grayImage);
        april.DetectMarkers(grayImage, dets);
        if (showTimeProfile)
        {
            april.DrawTimeProfile(frame.image, cv::Point(10, 60));
        }
        // draw all markers blue. We will overwrite this with other colors for markers that are part of any of the trackers that we use
        cv::aruco::drawDetectedMarkers(frame.image, dets.corners, dets.ids, COLOR_MARKER_DETECTED);

        math::EstimatePoseSingleMarkers(dets.corners, markerSize, *camCalib, markerPoses);
        ATT_ASSERT(markerPoses.positions.size() == dets.ids.size());
        ATT_ASSERT(markerPoses.rotations.size() == dets.ids.size());
        const double maxDist = user_config.trackerCalibDistance;

        // TODO: stop using hardcoded tracker roles
        /// 0 = waist, 1 = left foot, 2 = right foot
        for (int trackerIndex = 0; trackerIndex < trackerNum; ++trackerIndex)
        {
            auto& unit = trackerUnits[trackerIndex];
            // on weird images or calibrations, throws exception. This should usually only happen on bad camera calibrations, or in very rare cases
            auto [boardPose, estimated] = math::EstimatePoseTracker(dets.corners, dets.ids, unit.GetArucoBoard(), *camCalib);
            if (estimated == 0) continue; // no existing markers in this tracker were detected, can't add new ones to it

            cv::drawFrameAxes(frame.image, camCalib->cameraMatrix, camCalib->distortionCoeffs, boardPose.rotation.value, boardPose.position, 0.1F);

            bool foundMarkerToCalibrate = false;
            for (Index detIndex = 0; detIndex < static_cast<Index>(dets.ids.size()); ++detIndex)
            {
                const int detId = dets.ids[detIndex];
                const MarkerCorners2f& detCorners = dets.corners[detIndex];
                const RodrPose detMarkerPose{markerPoses.positions[detIndex], math::RodriguesVec3d(markerPoses.rotations[detIndex])};

                // if marker is part of current tracker (usualy, 0 is 0-44, 1 is 45-89 etc), if not, continue to next detection
                if (detId < (trackerIndex * markersPerTracker) || detId >= ((trackerIndex + 1) * markersPerTracker))
                {
                    continue;
                }

                // the main markers are already added above
                if (unit.HasMarkerId(detId)) // already added to a tracker, draw it green and continue to next detection
                {
                    DrawMarker(frame.image, detCorners, COLOR_MARKER_ADDED);
                    continue;
                }
                ATT_ASSERT(detId % markersPerTracker != 0, "main marker already added");

                // if marker is too far away from camera, paint it purple, as adding it could have too much error
                if (Length(detMarkerPose.position) > maxDist)
                {
                    DrawMarker(frame.image, detCorners, COLOR_MARKER_FAR);
                    continue;
                }

                DrawMarker(frame.image, detCorners, COLOR_MARKER_ADDING);

                if (foundMarkerToCalibrate) continue;
                foundMarkerToCalibrate = true;

                // append this detection to cornersList for this id
                auto& cornersList = markersCache[detId]; // add or get
                MarkerCorners3f& corners = cornersList.emplace_back(math::NUM_CORNERS);
                TransformMarkerSpace(modelMarker, boardPose, detMarkerPose, corners);

                if (cornersList.size() >= numCachedToAdd)
                {
                    MarkerCorners3f outMedianMarker;
                    FindMedianMarker(cornersList, outMedianMarker);
                    unit.AddMarker(detId, std::move(outMedianMarker));
                }
            }
        }

        if (preview.IsVisible()) preview.Update(frame.image, DRAW_IMG_SIZE);
    };

    // run loop until we stop it
    while (cameraRunning && mainThreadRunning)
    {
        try
        {
            doStep();
        }
        catch (const std::exception& e)
        {
            ATT_LOG_ERROR(e.what());
            gui->ShowPopup(lc.TRACKER_CALIBRATION_SOMETHINGWRONG, PopupStyle::Error);
            mainThreadRunning = false;
            return;
        }
    }
    mainThreadRunning = false;

    if (promptSaveCalib)
    {
        SaveTrackerUnitsToCalib(trackerUnits);
        SetTrackerUnitsFromConfig();
    }
}

void Tracker::UpdatePlayspaceCalibrator(bool& posActive, bool& angleActive, cv::Vec3d& posOffset, cv::Vec3d& angleOffset, utils::SteadyTimer& timer)
{
    ATT_ASSERT(mVRClient->IsInit());
    mVRClient->UpdateInputActions();
    const tracker::ButtonAction inputButton = mVRClient->GetButtonAction();
    cfg::ManualCalib::Real calib = gui->GetManualCalib();

    const auto now = utils::SteadyTimer::Now();

    if (timer.Get(now) > utils::Seconds(60)) // we exit playspace calibration after 60 seconds of no input detected, to try to prevent accidentaly ruining calibration
    {
        ATT_LOG_INFO("auto timeout of playspace calibration");
        gui->SetManualCalibVisible(false);
    }

    if (inputButton == tracker::ButtonAction::GrabCamera) // logic for position button
    {
        // to prevent accidental double presses, 0.2 seconds must pass between presses.
        if (timer.Get(now) > utils::MilliS(200))
        {
            if (!posActive) // if position calibration is inactive, set it to active and calculate offsets between the camera and controller
            {
                posActive = true;
                const Pose pose = mVRClient->GetControllerPoseAction();
                posOffset = cv::Vec3d(pose.position) - calib.posOffset;

                RotateVecByQuat(posOffset, pose.rotation.inv(cv::QUAT_ASSUME_UNIT));
            }
            else // else, deactivate it
            {
                posActive = false;
            }
            ATT_LOG_DEBUG("pos button pressed: ", posActive);
        }
        timer.Restart(now);
    }
    else if (inputButton == tracker::ButtonAction::GrabTrackers) // logic for rotation button
    {
        // to prevent accidental double presses, 0.2 seconds must pass between presses.
        if (timer.Get(now) > utils::MilliS(200))
        {
            if (!angleActive) // if rotation calibration is inactive, set it to active and calculate angle offsets and distance
            {
                angleActive = true;
                const Pose pose = mVRClient->GetControllerPoseAction();

                cv::Vec2d angle = EulerAnglesFromPos(cv::Vec3d(pose.position), calib.posOffset);
                const double distance = Distance(cv::Vec3d(pose.position), calib.posOffset);

                angleOffset[0] = angle[0] - calib.angleOffset[0];
                angleOffset[1] = angle[1] - calib.angleOffset[1];
                angleOffset[2] = distance / calib.scale;
            }
            else // else, deactivate it
            {
                angleActive = false;
            }
            ATT_LOG_DEBUG("angle button pressed: ", angleActive);
        }
        timer.Restart(now);
    }

    if (posActive) // while position calibration is active, apply the camera to controller offset to X, Y and Z values
    {
        const Pose pose = mVRClient->GetControllerPoseAction();

        RotateVecByQuat(posOffset, pose.rotation);

        calib.posOffset[0] = pose.position.x - posOffset[0];
        if (!lockHeightCalib)
        {
            calib.posOffset[1] = pose.position.y - posOffset[1];
        }
        calib.posOffset[2] = pose.position.z - posOffset[2];

        RotateVecByQuat(posOffset, pose.rotation.inv(cv::QUAT_ASSUME_UNIT));
    }

    if (angleActive) // while rotation calibration is active, apply the camera to controller angle offsets to A, B, C values, and apply the calibScale based on distance from camera
    {
        const Pose pose = mVRClient->GetControllerPoseAction();
        cv::Vec2d angle = EulerAnglesFromPos(cv::Vec3d(pose.position), calib.posOffset);
        const double distance = Distance(cv::Vec3d(pose.position), calib.posOffset);

        calib.angleOffset[1] = angle[1] - angleOffset[1];
        if (!lockHeightCalib) // if height is locked, do not calibrate up/down rotation or scale
        {
            calib.angleOffset[0] = angle[0] - angleOffset[0];
            calib.scale = distance / angleOffset[2];
        }
    }

    // check that camera is facing correct direction. 90 degrees mean looking straight down, 270 is straight up. This ensures its not upside down.
    calib.angleOffset[0] = std::clamp(calib.angleOffset[0], 91 * DEG_2_RAD, 269 * DEG_2_RAD);

    if (angleActive || posActive)
    {
        // update the calib in the gui as it wont be modified anymore
        gui->SetManualCalib(calib);
    }
    // update the pre calculated calibration transformations
    SetWorldTransform(gui->GetManualCalib());

    cv::Vec3d stationPos = wtransform.translation();
    CoordTransformOVR(stationPos);
    // rotate y axis by 180 degrees, aka, reflect across xz plane
    const cv::Quatd stationQ = cv::Quatd(0, 0, 1, 0) * wrotation;

    // move the camera in steamvr to new calibration
    mVRDriver->UpdateStation(Pose(stationPos, stationQ));
}

void Tracker::UpdateMulticamPlayspaceCalibrator(const tracker::TrackerUnit& unit)
{
    // if calibration refinement with multiple cameras is active, do not send calculated poses to driver.
    // instead, refine the calibration data with gradient descent
    // the error is the diffrence of the detected trackers position to the estimated trackers position
    // numerical derivatives are then calculated to see how X,Y,Z, A,B, scale data affects the error in position
    // calibration values are then slightly changed in the estimated direction in order to reduce error.
    // after a couple of seconds, the calibration data should converge

    cv::Vec3d pos = unit.GetEstimatedPose().position;
    cv::Vec2d angles = EulerAnglesFromPos(pos);
    const double length = Length(pos);

    cv::Vec3d driverPos = unit.GetPoseFromDriver().position;
    cv::Vec2d driverAngles = EulerAnglesFromPos(driverPos);
    const double driverLength = Length(driverPos);

    pos = wtransform * pos;
    driverPos = wtransform * driverPos;

    CoordTransformOVR(pos);
    CoordTransformOVR(driverPos);

    double dX = (pos[0] - driverPos[0]) * 0.1;
    double dY = -(pos[1] - driverPos[1]) * 0.1;
    double dZ = (pos[2] - driverPos[2]) * 0.1;

    if (std::abs(dX) > 0.01)
    {
        dX = 0.1 * (dX / std::fabs(dX));
    }
    if (std::abs(dY) > 0.1)
    {
        dY = 0.1 * (dY / std::abs(dY));
    }
    if (std::abs(dZ) > 0.1)
    {
        dZ = 0.1 * (dZ / std::abs(dZ));
    }

    cfg::ManualCalib::Real calib = gui->GetManualCalib();

    calib.posOffset += cv::Vec3d(dX, dY, dZ);
    calib.angleOffset += cv::Vec3d(angles[0] - driverAngles[0], angles[1] - driverAngles[1]) * 0.1;
    calib.scale -= (1 - (driverLength / length)) * 0.1;

    gui->SetManualCalib(calib);
    SetWorldTransform(calib);
}

void Tracker::MainLoop()
{
    const Index trackerNum = mTrackerUnits.size();
    SetWorldTransform(user_config.manualCalib.GetAsReal());

    // these variables are used to save detections of apriltags, so we dont define them every frame

    MarkerDetectionList dets{};

    CapturedFrame frame;
    cv::Mat drawImg;
    cv::Mat outImg;
    cv::Mat grayAprilImg;
    cv::Mat maskSearchImg;
    cv::Mat tempGrayMaskedImg;
    // setup all variables that need to be stored for each tracker and initialize them

    int framesSinceLastSeen = 0;
    constexpr int framesToCheckAll = 20;

    // calculate position of camera from calibration data and send its position to steamvr
    {
        cv::Vec3d stationPos = wtransform.translation();
        CoordTransformOVR(stationPos);
        const cv::Quatd stationQ = cv::Quatd(0, 0, 1, 0) * (wrotation * cv::Quatd(1, 0, 0, 0));
        mVRDriver->UpdateStation(Pose(stationPos, stationQ));
    }

    // initialize variables for playspace calibration
    bool calibControllerPosActive = false;
    bool calibControllerAngleActive = false;
    /// offset of controller to virtual camera, set when button is pressed
    cv::Vec3d calibControllerPosOffset = {0, 0, 0};
    /// (pitch, yaw) in radians, 3rd component is distance
    cv::Vec3d calibControllerAngleOffset = {0, 0, 0};

    utils::SteadyTimer calibControllerTimer{};
    utils::SteadyTimer detectionTimer{};

    const RefPtr<cfg::CameraCalib> camCalib = calib_config.cameras[0];
    const RefPtr<cfg::VideoStream> videoStream = user_config.videoStreams[0];
    // TODO: support other marker families
    AprilTagWrapper april{MarkerFamily::Standard41h12, videoStream->quadDecimate, 4};

    // TODO: temporary lambda to make refactor easier
    const auto doStep = [&] {
        CopyFreshCameraImageTo(frame);
        // shallow copy, gray will be cloned from image and used for detection,
        // so drawing can happen on color image without clone.
        drawImg = frame.image;
        AprilTagWrapper::ConvertGrayscale(frame.image, grayAprilImg);
        const bool previewIsVisible = gui->IsPreviewVisible();

        const auto stampBeforeDetect = utils::SteadyTimer::Now();
        detectionTimer.Restart(stampBeforeDetect);

        bool circularWindow = videoStream->circularWindow;

        // if any tracker was lost for longer than 20 frames, mark circularWindow as false
        for (const auto& unit : mTrackerUnits)
        {
            if (!unit.WasVisibleLastFrame())
            {
                ++framesSinceLastSeen;
                if (framesSinceLastSeen > framesToCheckAll) circularWindow = false;
                break;
            }
        }
        if (!circularWindow) framesSinceLastSeen = 0;

        // define our mask image. We want to create an image where everything but circles around predicted tracker positions will be black to speed up detection.
        if (GetMatSize(maskSearchImg) != GetMatSize(grayAprilImg))
        {
            maskSearchImg.create(GetMatSize(grayAprilImg), CV_8U);
        }
        maskSearchImg = cv::Scalar(0); // fill with empty pixels
        const int searchRadius = static_cast<int>(static_cast<double>(grayAprilImg.rows) * videoStream->searchWindow);
        bool atleastOneTrackerVisible = false;

        const double frameTimeBeforeDetect = duration_cast<utils::FSeconds>(stampBeforeDetect - frame.timestamp).count();
        for (int i = 0; i < trackerNum; i++)
        {
            auto& unit = mTrackerUnits[i];
            auto [pose, isValid] = mVRDriver->GetTracker(i, -frameTimeBeforeDetect - videoStream->latency);
            CoordTransformOVR(pose.position);
            // transform boards position from steamvr space to camera space based on our calibration data
            pose.position = wtransform.inv() * pose.position;

            std::array<cv::Point2d, 2> projected;
            {
                const cv::Vec3d unusedRVec{}; // used to perform change of basis
                const cv::Vec3d unusedTVec{};
                const std::array<cv::Point3d, 2> points{pose.position, unit.GetEstimatedPose().position};
                cv::projectPoints(points, unusedRVec, unusedTVec, camCalib->cameraMatrix, camCalib->distortionCoeffs, projected);
            }
            const auto& [driverCenter, previousCenter] = projected;

            // project point from position of tracker in camera 3d space to 2d camera pixel space, and draw a dot there
            if (previewIsVisible) cv::circle(drawImg, driverCenter, 5, cv::Scalar(0, 0, 255), 2, 8, 0);

            unit.SetWasVisibleToDriverLastFrame(isValid);
            cv::Point2d maskCenter;
            if (isValid) // if the pose from steamvr was valid, save the predicted position and rotation
            {
                /// TODO: What is this?
                pose.rotation = wrotation.inv(cv::QUAT_ASSUME_UNIT) *
                                cv::Quatd(0, 0, 1, 0).inv(cv::QUAT_ASSUME_UNIT) *
                                pose.rotation.normalize() *
                                cv::Quatd(0, 0, 1, 0).inv(cv::QUAT_ASSUME_UNIT);

                if (previewIsVisible) cv::aruco::drawAxis(drawImg, camCalib->cameraMatrix, camCalib->distortionCoeffs, pose.rotation.toRotVec(), math::ToVec(pose.position), 0.10F);

                if (!unit.WasVisibleLastFrame()) // if tracker was found in previous frame, we use that position for masking. If not, we use position from driver for masking.
                {
                    maskCenter = driverCenter;
                }
                else
                {
                    maskCenter = previousCenter;
                }
                unit.SetWasVisibleLastFrame(true);
                unit.SetPoseFromDriver(RodrPose(pose));
            }
            else
            {
                if (unit.WasVisibleLastFrame())
                {
                    maskCenter = previousCenter; // if pose is not valid, set everything based on previous known position
                }
            }

            if (maskCenter.inside(cv::Rect2d(0, 0, frame.image.cols, frame.image.rows)))
            {
                atleastOneTrackerVisible = true;
                if (circularWindow) // if circular window is set mask a circle around the predicted tracker point
                {
                    cv::circle(maskSearchImg, maskCenter, searchRadius, cv::Scalar(255), -1, 8, 0);
                    if (previewIsVisible) cv::circle(drawImg, maskCenter, searchRadius, COLOR_MASK, 2, 8, 0);
                }
                else // if not, mask a vertical strip top to bottom. This happens every 20 frames if a tracker is lost.
                {
                    const int maskX = static_cast<int>(maskCenter.x);
                    const cv::Rect2i maskRect{cv::Point(maskX - searchRadius, 0), cv::Point2i(maskX + searchRadius, frame.image.rows)};
                    cv::rectangle(maskSearchImg, maskRect, cv::Scalar(255), -1);
                    if (previewIsVisible) cv::rectangle(drawImg, maskRect, COLOR_MASK, 3);
                }
            }
            else
            {
                unit.SetWasVisibleLastFrame(false); // if detected tracker is out of view of the camera, we mark it as not found, as either the prediction is wrong or we wont see it anyway
            }
        }

        // using copyTo with masking creates the image where everything but the locations where trackers are predicted to be is black
        if (atleastOneTrackerVisible)
        {
            grayAprilImg.copyTo(tempGrayMaskedImg, maskSearchImg);
            grayAprilImg = tempGrayMaskedImg;
        }

        if (manualRecalibrate) // playspace calibration loop
        {
            UpdatePlayspaceCalibrator(calibControllerPosActive, calibControllerAngleActive, calibControllerPosOffset, calibControllerAngleOffset, calibControllerTimer);
        }
        else
        {
            calibControllerTimer.Restart();
        }

        april.DetectMarkers(grayAprilImg, dets);
        // frame time is how much time passed since frame was acquired.
        const double frameTimeAfterDetect = duration_cast<utils::FSeconds>(utils::SteadyTimer::Now() - frame.timestamp).count();
        for (int index = 0; index < mTrackerUnits.size(); ++index)
        {
            auto& unit = mTrackerUnits[index];
            // estimate the pose of current board
            const RodrPose scaledPoseFromDriver{unit.GetPoseFromDriver().position / wscale, unit.GetPoseFromDriver().rotation};
            // on rare occasions, detection crashes. Should be very rare and indicate something wrong with camera or tracker calibration
            auto [estimatedPose, numEstimated] = math::EstimatePoseTracker(
                dets.corners, dets.ids, unit.GetArucoBoard(), *camCalib,
                unit.WasVisibleLastFrame() && user_config.usePredictive,
                scaledPoseFromDriver);
            estimatedPose.position *= wscale; // unscale returned estimation;
            unit.SetEstimatedPose(estimatedPose);

            ATT_ASSERT(!std::isnan(estimatedPose.position[X]));

            if (numEstimated <= 0)
            {
                unit.SetWasVisibleLastFrame(false);
                continue;
            }
            unit.SetWasVisibleLastFrame(true);

            if (user_config.depthSmoothing > 0 && unit.WasVisibleToDriverLastFrame() && !manualRecalibrate)
            {
                // depth estimation is noisy, so try to smooth it more, especialy if using multiple cameras
                // if position is close to the position predicted by the driver, take the depth of the driver.
                // if error is big, take the calculated depth
                // error threshold is defined in the params as depth smoothing
                RodrPose pose = unit.GetEstimatedPose();

                const double distDriver = Length(unit.GetPoseFromDriver().position);
                const double distPredict = Length(pose.position);

                const cv::Vec3d normPredict = pose.position / distPredict;

                double dist = std::abs(distPredict - distDriver);
                dist = (dist / static_cast<double>(user_config.depthSmoothing)) + 0.1;
                dist = std::clamp(dist, 0.0, 1.0);

                const double distFinal = (dist * distPredict) + (1 - dist) * distDriver;

                pose.position = normPredict * distFinal;
                unit.SetEstimatedPose(pose);
            }

            {
                const cv::Point3d position = unit.GetEstimatedPose().position;

                // Reject detected positions that are behind the camera
                if (position.z < 0)
                {
                    unit.SetWasVisibleLastFrame(false);
                    continue;
                }

                // Figure out the camera aspect ratio, XZ and YZ ratio limits
                const double aspectRatio = GetMatSize(frame.image).aspectRatio();
                const double xzRatioLimit = 0.5 * static_cast<double>(frame.image.cols) / camCalib->cameraMatrix.at<double>(0, 0);
                const double yzRatioLimit = 0.5 * static_cast<double>(frame.image.rows) / camCalib->cameraMatrix.at<double>(1, 1);

                // Figure out whether X or Y dimension is most likely to go outside the camera field of view
                if (std::abs(position.x / position.y) > aspectRatio)
                {
                    // Reject detections when XZ coordinate ratio goes out of camera FOV
                    if (std::abs(position.x / position.z) > xzRatioLimit)
                    {
                        unit.SetWasVisibleLastFrame(false);
                        continue;
                    }
                }
                else
                {
                    // Reject detections when YZ coordinate ratio goes out of camera FOV
                    if (std::abs(position.y / position.z) > yzRatioLimit)
                    {
                        unit.SetWasVisibleLastFrame(false);
                        continue;
                    }
                }
            }

            if (multicamAutocalib && unit.WasVisibleToDriverLastFrame())
            {
                UpdateMulticamPlayspaceCalibrator(unit);
                continue; // skip sending to driver
            }

            // transform boards position based on our calibration data
            Pose poseToSend{unit.GetEstimatedPose()};
            poseToSend.position = wtransform * poseToSend.position;
            poseToSend.rotation = wrotation * poseToSend.rotation;
            CoordTransformOVR(poseToSend.position);
            CoordTransformOVR(poseToSend.rotation);

            // send all the values
            mVRDriver->UpdateTracker(index, poseToSend, -frameTimeAfterDetect - videoStream->latency, user_config.smoothingFactor);
        }

        if (gui->IsPreviewVisible())
        {
            // draw and display the detections
            if (!dets.ids.empty()) cv::aruco::drawDetectedMarkers(drawImg, dets.corners, dets.ids);
            const cv::Size2i drawSize = ConstrainSize(GetMatSize(frame.image), DRAW_IMG_SIZE);
            cv::resize(drawImg, outImg, drawSize);
            cv::putText(outImg, std::to_string(frameTimeAfterDetect).substr(0, 5), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
            if (showTimeProfile)
            {
                april.DrawTimeProfile(outImg, cv::Point(10, 60));
            }
            gui->UpdatePreview(outImg);
        }
        // time of marker detection
    };

    // run detection until camera is stopped or the start/stop button is pressed again
    while (mainThreadRunning && cameraRunning)
    {
        try
        {
            doStep();
        }
        catch (const std::exception& e)
        {
            ATT_LOG_ERROR(e.what());
            mainThreadRunning = false;
            gui->ShowPopup(lc.TRACKER_DETECTION_SOMETHINGWRONG, PopupStyle::Error);
        }
    }
    mainThreadRunning = false;
}

namespace
{

void EnsureTrackersConfigSize(UserConfig& userConfig, CalibrationConfig& calibConfig)
{
    const Index expected = std::max(userConfig.trackers.GetSize(), calibConfig.trackers.GetSize());
    userConfig.trackers.Resize(expected);
    calibConfig.trackers.Resize(expected);
}

} // namespace

void Tracker::SetTrackerUnitsFromConfig()
{
    EnsureTrackersConfigSize(user_config, calib_config);
    if (user_config.trackers.GetSize() == 0) return; // not calibrated yet

    mTrackerUnits.resize(user_config.trackers.GetSize());
    for (Index i = 0; i < static_cast<Index>(mTrackerUnits.size()); ++i)
    {
        const auto config = user_config.trackers[i];
        const auto calib = calib_config.trackers[i];
        auto& unit = mTrackerUnits[i];
        unit.SetMarkers(calib->ids, calib->corners);
        if (user_config.trackerCalibCenters) unit.RecenterMarkers();
        unit.SetRole(config->role);
    }
}

void Tracker::SaveTrackerUnitsToCalib(const std::vector<tracker::TrackerUnit>& trackerUnits)
{
    calib_config.trackers.Resize(trackerUnits.size());
    for (Index i = 0; i < static_cast<Index>(trackerUnits.size()); ++i)
    {
        calib_config.trackers[i]->ids = trackerUnits[i].GetIds();
        calib_config.trackers[i]->corners = trackerUnits[i].GetMarkers();
    }
    calib_config.Save();
}
