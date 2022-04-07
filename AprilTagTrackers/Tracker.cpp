#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <vector>

#pragma warning(push)
#pragma warning(disable:4996)
#include <wx/wx.h>
#pragma warning(pop)

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <thread>

#include "AprilTagWrapper.h"
#include "Connection.h"
#include "GUI.h"
#include "Helpers.h"
#include "MessageDialog.h"
#include "Parameters.h"
#include "Tracker.h"

namespace {

// Create a grid in front of the camera for visualization purposes.
std::vector<std::vector<cv::Point3f>> createXyGridLines(
    const int gridSizeX, // Number of units from leftmost to rightmost line.
    const int gridSizeY, // Number of units from top to bottom line.
    const int gridSubdivision, // Number of segments per line.
    const float z) // Z-coord of grid.
{
    std::vector<std::vector<cv::Point3f>> gridLines(gridSizeX + gridSizeY + 2);
    for (int i = 0; i <= gridSizeX; ++i)
    {
        auto& verticalLine = gridLines[i];
        verticalLine.reserve(gridSizeY * gridSubdivision + 1);
        const float x = float(i) - float(gridSizeX) * 0.5f;
        for (int j = 0; j <= gridSizeY * gridSubdivision; ++j)
        {
            const float y = float(j) / float(gridSubdivision) - float(gridSizeY) * 0.5f;
            verticalLine.push_back(cv::Point3f(x, y, z));
        }
    }
    for (int i = 0; i <= gridSizeY; ++i)
    {
        auto& horizontalLine = gridLines[gridSizeX + 1 + i];
        horizontalLine.reserve(gridSizeX * gridSubdivision + 1);
        const float y = float(i) - float(gridSizeY) * 0.5f;
        for (int j = 0; j <= gridSizeX * gridSubdivision; ++j)
        {
            const float x = float(j) / float(gridSubdivision) - float(gridSizeX) * 0.5f;
            horizontalLine.push_back(cv::Point3f(x, y, z));
        }
    }
    return gridLines;
}

void previewCalibration(
    cv::Mat& drawImg,
    const cv::Mat1d& cameraMatrix,
    const cv::Mat1d& distCoeffs,
    const cv::Mat1d& stdDeviationsIntrinsics,
    const std::vector<double>& perViewErrors,
    const std::vector<std::vector<cv::Point2f>>& allCharucoCorners,
    const std::vector<std::vector<int>>& allCharucoIds)
{
    if (!cameraMatrix.empty())
    {
        const float gridZ = 10.0f;
        const float width = drawImg.cols;
        const float height = drawImg.rows;
        const float fx = cameraMatrix(0, 0);
        const float fy = cameraMatrix(1, 1);
        const int gridSizeX = std::round(gridZ * width / fx);
        const int gridSizeY = std::round(gridZ * height / fy);
        const std::vector<std::vector<cv::Point3f>> gridLinesInCamera = createXyGridLines(gridSizeX, gridSizeY, 10, gridZ);
        std::vector<cv::Point2f> gridLineInImage; // Will be populated by cv::projectPoints.

        // The generator is static to avoid starting over with the same seed every time.
        static std::default_random_engine generator;
        std::normal_distribution<double> unitGaussianDistribution(0.0, 1.0);

        cv::Mat1d sampleCameraMatrix = cameraMatrix.clone();
        cv::Mat1d sampleDistCoeffs = distCoeffs.clone();
        if (!stdDeviationsIntrinsics.empty())
        {
            assert(sampleDistCoeffs.total() + 4 <= stdDeviationsIntrinsics.total());
            sampleCameraMatrix(0, 0) += unitGaussianDistribution(generator) * stdDeviationsIntrinsics(0);
            sampleCameraMatrix(1, 1) += unitGaussianDistribution(generator) * stdDeviationsIntrinsics(1);
            sampleCameraMatrix(0, 2) += unitGaussianDistribution(generator) * stdDeviationsIntrinsics(2);
            sampleCameraMatrix(1, 2) += unitGaussianDistribution(generator) * stdDeviationsIntrinsics(3);
            for (int i = 0; i < sampleDistCoeffs.total(); ++i)
            {
                sampleDistCoeffs(i) += unitGaussianDistribution(generator) * stdDeviationsIntrinsics(i + 4);
            }
        }

        for (const auto& gridLineInCamera : gridLinesInCamera)
        {
            cv::projectPoints(gridLineInCamera, cv::Vec3f::zeros(), cv::Vec3f::zeros(), sampleCameraMatrix, sampleDistCoeffs, gridLineInImage);
            for (size_t j = 1; j < gridLineInImage.size(); ++j)
            {
                const auto p1 = gridLineInImage[j - 1];
                const auto p2 = gridLineInImage[j];
                cv::line(drawImg, p1, p2, cv::Scalar(127, 127, 127));
            }
        }
    }

    if (allCharucoCorners.size() > 0)
    {
        // Draw all corners that we have so far
        cv::Mat colorsFromErrors;
        if (!perViewErrors.empty())
        {
            cv::Mat(perViewErrors).convertTo(colorsFromErrors, CV_8UC1, 255.0, 0.0);
            cv::applyColorMap(colorsFromErrors, colorsFromErrors, cv::COLORMAP_VIRIDIS);
        }
        for (int i = 0; i < allCharucoCorners.size(); ++i)
        {
            const auto& charucoCorners = allCharucoCorners[i];
            cv::Scalar color(200, 100, 0);
            if (colorsFromErrors.total() > i)
            {
                color = colorsFromErrors.at<cv::Vec3b>(i);
            }
            for (const auto& point : charucoCorners)
            {
                cv::circle(drawImg, point, 2, color, cv::FILLED);
            }
        }
    }
}

void previewCalibration(cv::Mat& drawImg, Parameters* parameters)
{
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;
    cv::Mat1d stdDeviationsIntrinsics;
    parameters->camMat.copyTo(cameraMatrix);
    parameters->distCoeffs.copyTo(distCoeffs);
    parameters->stdDeviationsIntrinsics.copyTo(stdDeviationsIntrinsics);
    std::vector<double> perViewErrors = parameters->perViewErrors;
    std::vector<std::vector<cv::Point2f>> allCharucoCorners = parameters->allCharucoCorners;
    std::vector<std::vector<int>> allCharucoIds = parameters->allCharucoIds;

    previewCalibration(
        drawImg,
        cameraMatrix,
        distCoeffs,
        stdDeviationsIntrinsics,
        perViewErrors,
        allCharucoCorners,
        allCharucoIds);
}

} // namespace

Tracker::Tracker(Parameters* params, Connection* conn, MyApp* app)
{
    parameters = params;
    connection = conn;
    parentApp = app;
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
    calibScale = parameters->calibScale;
}

void Tracker::StartCamera(std::string id, int apiPreference)
{
    if (cameraRunning)
    {
        cameraRunning = false;
        mainThreadRunning = false;
        //cameraThread.join();
        sleep_millis(1000);
        return;
    }
    if (id.length() <= 2)		//if camera address is a single character, try to open webcam
    {
        int i = std::stoi(id);	//convert to int
#if OS_LINUX
        // On Linux cv::VideoCapture does not work when GStreamer backend is used and
        // camera is set to MJPG pixel format. As a work around we manually setup the
        // GStreamer pipeline with suitable decoding before feeding the stream into
        // application.
        if ((apiPreference == cv::CAP_ANY) || (apiPreference == cv::CAP_GSTREAMER))
        {
            std::stringstream ss;
            ss << "v4l2src device=/dev/video" << id << " ! image/jpeg";
            if (parameters->camWidth != 0)
                ss << ",width=" << parameters->camWidth;
            if (parameters->camHeight != 0)
                ss << ",height=" << parameters->camHeight;
            ss << ",framerate=" << parameters->camFps << "/1";
            ss << " ! jpegdec ! video/x-raw,format=I420 ! videoconvert ! appsink";
            cap = cv::VideoCapture(ss.str(), apiPreference);
        }
        else
        {
            cap = cv::VideoCapture(i, apiPreference);
        }
#else
        
        //API preference of 2300 is defined in pseyecapi
        if (apiPreference == 2300)
            cap = PSEyeVideoCapture(i);
        else
            cap = cv::VideoCapture(i, apiPreference);
#endif
    }
    else
    {			//if address is longer, we try to open it as an ip address
        cap = cv::VideoCapture(id, apiPreference);
    }

    if (!cap.isOpened())
    {
        wxMessageDialog dial(NULL,
            parameters->language.TRACKER_CAMERA_START_ERROR, wxT("Error"), wxOK | wxICON_ERROR);
        dial.ShowModal();
        return;
    }
    //Sleep(1000);

    // On Linux and when GStreamer backend is used we already setup the camera pixel format,
    // width, height and FPS above when the GStreamer pipeline was created.
#if OS_LINUX
    if ((apiPreference != cv::CAP_ANY) && (apiPreference != cv::CAP_GSTREAMER))
#endif
    {
        //cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('m', 'j', 'p', 'g'));
        //cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));

        if(parameters->camWidth != 0)
            cap.set(cv::CAP_PROP_FRAME_WIDTH, parameters->camWidth);
        if (parameters->camHeight != 0)
            cap.set(cv::CAP_PROP_FRAME_HEIGHT, parameters->camHeight);
        cap.set(cv::CAP_PROP_FPS, parameters->camFps);
    }
    if(parameters->cameraSettings)
        cap.set(cv::CAP_PROP_SETTINGS, 1);
    if (parameters->settingsParameters)
    {
        cap.set(cv::CAP_PROP_AUTOFOCUS, 0);
        cap.set(cv::CAP_PROP_AUTO_EXPOSURE, parameters->cameraAutoexposure);
        cap.set(cv::CAP_PROP_EXPOSURE, parameters->cameraExposure);
        cap.set(cv::CAP_PROP_GAIN, parameters->cameraGain);
    }

    double codec = 0x47504A4D; //code by FPaul. Should use MJPEG codec to enable fast framerates.
    cap.set(cv::CAP_PROP_FOURCC, codec);

    cameraRunning = true;
    cameraThread = std::thread(&Tracker::CameraLoop, this);
    cameraThread.detach();
}

void Tracker::CameraLoop()
{
    bool previewShown = false;
    bool rotate = false;
    int rotateFlag = -1;
    if (parameters->rotateCl && parameters->rotateCounterCl)
    {
        rotate = true;
        rotateFlag = cv::ROTATE_180;
    }
    else if (parameters->rotateCl)
    {
        rotate = true;
        rotateFlag = cv::ROTATE_90_CLOCKWISE;
    }
    else if (parameters->rotateCounterCl)
    {
        rotate = true;
        rotateFlag = cv::ROTATE_90_COUNTERCLOCKWISE;
    }
    cv::Mat img;
    cv::Mat drawImg;
    double fps = 0;
    clock_t last_preview_time = clock();
    last_frame_time = clock();
    bool frame_visible = false;
    while (cameraRunning)
    {
        if (!cap.read(img))
        {
            gui->CallAfter([parameters=parameters] ()
                           {
                           wxMessageDialog dial(NULL,
                               parameters->language.TRACKER_CAMERA_ERROR, wxT("Error"), wxOK | wxICON_ERROR);
                           dial.ShowModal();
                           });
            cameraRunning = false;
            break;
        }
        clock_t curtime = clock();
        fps = 0.95*fps + 0.05/(double(curtime - last_frame_time) / double(CLOCKS_PER_SEC));
        last_frame_time = curtime;        
        if (rotate)
        {
            cv::rotate(img, img, rotateFlag);
        }
        std::string resolution = std::to_string(img.cols) + "x" + std::to_string(img.rows);
        double timeSinceLast = (double(curtime - last_preview_time) / double(CLOCKS_PER_SEC));  //ensure that preview isnt shown more than 60 times per second, otherwise the CallAfter function gets overloaded
        if (timeSinceLast > 0.015)
        {
            if ((previewCamera || previewCameraCalibration))
            {
                last_preview_time = clock();
                img.copyTo(drawImg);
                cv::putText(drawImg, std::to_string((int)(fps + (0.5))), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0));
                cv::putText(drawImg, resolution, cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0));
                if (previewCameraCalibration)
                {
                    cv::Mat* outImg = new cv::Mat();
                    drawImg.copyTo(*outImg);
                    previewCalibration(*outImg, parameters);
                    gui->CallAfter([outImg, &win_OTU = parameters->octiuSah]()
                        {
                            cv::imshow(win_OTU + " Preview", *outImg);
                            cv::waitKey(1);
                            delete(outImg);
                        });
                    previewShown = true;
                }
                else
                {
                    cv::Mat* outImg = new cv::Mat();
                    drawImg.copyTo(*outImg);
                    gui->CallAfter([outImg, &win_OTU = parameters->octiuSah]()
                        {
                            cv::imshow(win_OTU + " Preview", *outImg);
                            cv::waitKey(1);
                            delete(outImg);
                        });
                    previewShown = true;
                }
                frame_visible = true;
            }
            else if (previewShown)
            {
                gui->CallAfter([&win_OTU = parameters->octiuSah]()
                    {
                        cv::destroyWindow(win_OTU + " Preview");
                    });
                previewShown = false;
            }
        }
        {
            std::lock_guard<std::mutex> lock(cameraImageMutex);
            // Swap avoids copying the pixel buffer. It only swaps pointers and metadata.
            // The pixel buffer from cameraImage can be reused if the size and format matches.
            cv::swap(img, cameraImage);
            if (img.size() != cameraImage.size() || img.flags != cameraImage.flags)
            {
                img.release();
            }
            imageReady = true;
        }

        if (!disableOpenVrApi)
        {
            //process events. BETA TEST ONLY, MOVE TO CONNECTION LATER
            if (connection->status == connection->CONNECTED)
            {
                vr::VREvent_t event;
                while (connection->openvr_handle->PollNextEvent(&event, sizeof(event)))
                {
                    if (event.eventType == vr::VREvent_Quit)
                    {
                        connection->openvr_handle->AcknowledgeQuit_Exiting();       //close connection to steamvr without closing att
                        connection->status = connection->DISCONNECTED;
                        vr::VR_Shutdown();
                        mainThreadRunning = false;
                        break;
                    }
                }
            }
        }
    }
    gui->CallAfter([] ()
                   {
                   cv::destroyAllWindows();
                   });
    cap.release();
}

void Tracker::CopyFreshCameraImageTo(cv::Mat& image)
{
    // Sleep happens between each iteration when the mutex is not locked.
    for (;;sleep_millis(1))
    {
        std::lock_guard<std::mutex> lock(cameraImageMutex);
        if (imageReady)
        {
            imageReady = false;
            // Swap metadata and pointers to pixel buffers.
            cv::swap(image, cameraImage);
            // We don't want to overwrite shared data so release the image unless we are the only user of it.
            if (!(cameraImage.u && cameraImage.u->refcount == 1))
            {
                cameraImage.release();
            }
            return;
        }
    }
}

void DialogOk(void *data)
{
    Tracker *tracker = static_cast<Tracker *>(data);
    tracker->messageDialogResponse = wxID_OK;
    tracker->mainThreadRunning = false;
}

void DialogCancel(void *data)
{
    Tracker *tracker = static_cast<Tracker *>(data);
    tracker->messageDialogResponse = wxID_CANCEL;
    tracker->mainThreadRunning = false;
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
        bool *mtr = &mainThreadRunning;
        wxString e = parameters->language.TRACKER_CAMERA_NOTRUNNING;
        gui->CallAfter([e, mtr] ()
                        {
                            wxMessageDialog dial(NULL, e, wxT("Error"), wxOK | wxICON_ERROR);
                            dial.ShowModal();
                            *mtr = false;
                        });
        return;
    }

    mainThreadRunning = true;
    if(!parameters->chessboardCalib)
    {
        mainThread = std::thread(&Tracker::CalibrateCameraCharuco, this);
    }
    else
    {
        mainThread = std::thread(&Tracker::CalibrateCamera, this);
    }
    mainThread.detach();
}

void Tracker::CalibrateCameraCharuco()
{
    //function to calibrate our camera

    cv::Mat image;
    cv::Mat gray;
    cv::Mat drawImg;

    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
    cv::Ptr<cv::aruco::DetectorParameters> params = cv::aruco::DetectorParameters::create();

    //generate and show our charuco board that will be used for calibration
    cv::Ptr<cv::aruco::CharucoBoard> board = cv::aruco::CharucoBoard::create(8, 7, 0.04f, 0.02f, dictionary);
    cv::Mat boardImage;

    //set our detectors marker border bits to 1 since thats what charuco uses
    params->markerBorderBits = 1;

    //int framesSinceLast = -2 * parameters->camFps;
    clock_t timeOfLast = clock();

    int messageDialogResponse = wxID_CANCEL;
    std::thread th{ [this, &messageDialogResponse]() {
        wxString e = parameters->language.TRACKER_CAMERA_CALIBRATION_INSTRUCTIONS;
        int *mdr = &messageDialogResponse;
        bool *mtr = &mainThreadRunning;
        
#if OS_LINUX                                        //temporary fix, as showmodal inside a callafter doesnt work on windows, causing the out window to only update when you move your mouse
        gui->CallAfter([e, mdr, mtr] ()
            {
#endif
            wxMessageDialog dial(NULL, e, wxT("Message"), wxOK | wxCANCEL);
            *mdr = dial.ShowModal();
            *mtr = false;
#if OS_LINUX
            });
#endif
        
    } };

    th.detach();

    cv::Mat cameraMatrix, distCoeffs, R, T;
    cv::Mat1d stdDeviationsIntrinsics, stdDeviationsExtrinsics;
    std::vector<double> perViewErrors;
    std::vector<std::vector<cv::Point2f>> allCharucoCorners;
    std::vector<std::vector<int>> allCharucoIds;

    int picsTaken = 0;
    while(mainThreadRunning && cameraRunning)
    {
        CopyFreshCameraImageTo(image);
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

        previewCalibration(
            drawImg,
            cameraMatrix,
            distCoeffs,
            stdDeviationsIntrinsics,
            perViewErrors,
            allCharucoCorners,
            allCharucoIds);

        //check the highest per view error and remove it if its higher than 1px.

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

        cv::Mat *outImg = new cv::Mat();
        cv::resize(drawImg, *outImg, cv::Size(cols, rows));
        char key = -1;
        gui->CallAfter([outImg, &key, &win_OTU = parameters->octiuSah] ()
                        {
                        cv::imshow(win_OTU + " Calibration", *outImg);
                        //key = (char)cv::waitKey(1);
                        cv::waitKey(1);
                        delete(outImg);
                        });

        //if more than one second has passed since last calibration image, add current frame to calibration images
        //framesSinceLast++;
        if (key != -1 || double(clock() - timeOfLast) / double(CLOCKS_PER_SEC) > 1)
        {
            //framesSinceLast = 0;
            timeOfLast = clock();
            //if any button was pressed
            cvtColor(image, gray, cv::COLOR_BGR2GRAY);

            std::vector<int> markerIds;
            std::vector<std::vector<cv::Point2f>> markerCorners;
            std::vector<std::vector<cv::Point2f>> rejectedCorners;

            //detect our markers
            cv::aruco::detectMarkers(gray, dictionary, markerCorners, markerIds, params, rejectedCorners);
            cv::aruco::refineDetectedMarkers(gray, board, markerCorners, markerIds, rejectedCorners);

            if (markerIds.size() > 0)
            {
                //if markers were found, try to add calibration data
                std::vector<cv::Point2f> charucoCorners;
                std::vector<int> charucoIds;
                //using data from aruco detection we refine the search of chessboard corners for higher accuracy
                cv::aruco::interpolateCornersCharuco(markerCorners, markerIds, gray, board, charucoCorners, charucoIds);
                if (charucoIds.size() > 15)
                {
                    //if corners were found, we draw them
                    cv::aruco::drawDetectedCornersCharuco(drawImg, charucoCorners, charucoIds);
                    //we then add our corners to the array
                    allCharucoCorners.push_back(charucoCorners);
                    allCharucoIds.push_back(charucoIds);
                    picsTaken++;

                    cv::Mat *outImg = new cv::Mat();
                    cv::resize(drawImg, *outImg, cv::Size(cols, rows));
                    gui->CallAfter([outImg, &win_OTU = parameters->octiuSah] ()
                                    {
                                    cv::imshow(win_OTU + " Calibration", *outImg);
                                    cv::waitKey(1);
                                    delete(outImg);
                                    });
                    
                    if (picsTaken >= 3)
                    {
                        try
                        {
                            // Calibrate camera using our data
                            cv::aruco::calibrateCameraCharuco(allCharucoCorners, allCharucoIds, board, cv::Size(image.rows, image.cols),
                                cameraMatrix, distCoeffs, R, T, stdDeviationsIntrinsics, stdDeviationsExtrinsics, perViewErrors,
                                cv::CALIB_USE_LU);
                        }
                        catch(const cv::Exception& e)
                        {
                            std::cerr << "Failed to calibrate: " << e.what();
                        }

                        int curI = perViewErrors.size();

                    }
                }
            }
        }
    }

    gui->CallAfter([&win_OTU = parameters->octiuSah] ()
                   {
                   cv::destroyWindow(win_OTU + " Calibration");
                   });
    mainThreadRunning = false;
    if (messageDialogResponse == wxID_OK)
    {
        if (cameraMatrix.empty())
        {
            wxString e = parameters->language.TRACKER_CAMERA_CALIBRATION_NOTDONE;
            gui->CallAfter([e] ()
                           {
                           wxMessageDialog dial(NULL, e, wxT("Info"), wxOK | wxICON_ERROR);
                           dial.ShowModal();
                           });

        }
        else
        {

            //some checks of the camera calibration values. The thresholds should be adjusted to prevent any false  negatives
            // 
            //this was before bad frames were being removed, and should no longer be necessary and is commented out since it caused too many false negatives

            double avgPerViewError = 0;
            double maxPerViewError = 0;

            for (int i = 0; i < perViewErrors.size(); i++)
            {
                avgPerViewError += perViewErrors[i];
                if (perViewErrors[i] > maxPerViewError)
                    maxPerViewError = perViewErrors[i];
            }

            avgPerViewError /= perViewErrors.size();

            
            /*
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
            parameters->camMat = cameraMatrix;
            parameters->distCoeffs = distCoeffs;
            parameters->stdDeviationsIntrinsics = stdDeviationsIntrinsics;
            parameters->perViewErrors = perViewErrors;
            parameters->allCharucoCorners = allCharucoCorners;
            parameters->allCharucoIds = allCharucoIds;
            parameters->Save();
            wxString e = parameters->language.TRACKER_CAMERA_CALIBRATION_COMPLETE;
            gui->CallAfter([e] ()
                           {
                           wxMessageDialog dial(NULL, e, wxT("Info"), wxOK);
                           dial.ShowModal();
                           });
        }
    }
}

void Tracker::CalibrateCamera()
{
    //old calibration function, only still here for legacy reasons.


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
            gui->CallAfter([] ()
                           {
                           cv::destroyAllWindows();
                           });
            return;
        }
        CopyFreshCameraImageTo(image);
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
        {
            cv::Mat *outImg = new cv::Mat();
            cv::resize(image, *outImg, cv::Size(cols,rows));
            gui->CallAfter([outImg, &win_OTU = parameters->octiuSah] ()
                           {
                           cv::imshow(win_OTU + " Calibration", *outImg);
                           cv::waitKey(1);
                           delete(outImg);
                           });
        }
        framesSinceLast++;
        if (framesSinceLast > 50)
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

            {
                cv::Mat *outImg = new cv::Mat();
                cv::resize(image, *outImg, cv::Size(cols,rows));
                gui->CallAfter([outImg, &win_OTU = parameters->octiuSah] ()
                               {
                               cv::imshow(win_OTU + " Calibration", *outImg);
                               cv::waitKey(1);
                               delete(outImg);
                               });
            }
            sleep_millis(1000);
        }
    }

    cv::Mat cameraMatrix, distCoeffs, R, T;

    calibrateCamera(objpoints, imgpoints, cv::Size(image.rows, image.cols), cameraMatrix, distCoeffs, R, T);

    parameters->camMat = cameraMatrix;
    parameters->distCoeffs = distCoeffs;
    parameters->Save();
    mainThreadRunning = false;
    gui->CallAfter([] ()
                   {
                   cv::destroyAllWindows();
                   wxMessageDialog dial(NULL,
                       wxT("Calibration complete."), wxT("Info"), wxOK);
                   dial.ShowModal();
                   });
}

void Tracker::StartTrackerCalib()
{
    //check that no other process is running on main thread, check that camera is running and calibrated
    if (mainThreadRunning)
    {
        mainThreadRunning = false;
        return;
    }
    if (!cameraRunning)
    {
        wxString e = parameters->language.TRACKER_CAMERA_NOTRUNNING;
        bool *mtr = &mainThreadRunning;
        gui->CallAfter([e, mtr] ()
                       {
                       wxMessageDialog dial(NULL, e, wxT("Error"), wxOK | wxICON_ERROR);
                       dial.ShowModal();
                       *mtr = false;
                       });
        return;
    }
    if (parameters->camMat.empty())
    {
        wxString e = parameters->language.TRACKER_CAMERA_NOTCALIBRATED;
        bool *mtr = &mainThreadRunning;
        gui->CallAfter([e, mtr] ()
                       {
                       wxMessageDialog dial(NULL, e, wxT("Error"), wxOK | wxICON_ERROR);
                       dial.ShowModal();
                       *mtr = false;
                       });
        return;
    }

    //start tracker calibration on another thread
    mainThreadRunning = true;
    mainThread = std::thread(&Tracker::CalibrateTracker, this);
    mainThread.detach();


    //make a new thread with message box, and stop main thread when we press OK
    std::thread th{ [=]() {
        wxString e = parameters->language.TRACKER_TRACKER_CALIBRATION_INSTRUCTIONS;
        bool *mtr = &mainThreadRunning;

#if OS_LINUX                                        //temporary fix, as showmodal inside a callafter doesnt work on windows, causing the out window to only update when you move your mouse
        gui->CallAfter([e, mtr]()
            {
#endif
                        wxMessageDialog dial(NULL, e, wxT("Message"), wxOK);
                        dial.ShowModal();
                        *mtr = false;
#if OS_LINUX 
                        });
#endif
    } };

    th.detach();
}

void Tracker::Start()
{
    //check that no other process is running on main thread, check that camera is running and calibrated, check that trackers are calibrated
    if (mainThreadRunning)
    {
        mainThreadRunning = false;
        return;
    }
    if (!cameraRunning)
    {
        wxMessageDialog dial(NULL,
            parameters->language.TRACKER_CAMERA_NOTRUNNING, wxT("Error"), wxOK | wxICON_ERROR);
        dial.ShowModal();
        mainThreadRunning = false;
        return;
    }
    if (parameters->camMat.empty())
    {
        wxMessageDialog dial(NULL,
            parameters->language.TRACKER_CAMERA_NOTCALIBRATED, wxT("Error"), wxOK | wxICON_ERROR);
        dial.ShowModal();
        mainThreadRunning = false;
        return;
    }
    if (!trackersCalibrated)
    {
        wxMessageDialog dial(NULL,
            parameters->language.TRACKER_TRACKER_NOTCALIBRATED, wxT("Error"), wxOK | wxICON_ERROR);
        dial.ShowModal();
        mainThreadRunning = false;
        return;
    }
    if (connection->status != connection->CONNECTED)
    {
        wxMessageDialog dial(NULL,
            parameters->language.TRACKER_STEAMVR_NOTCONNECTED, wxT("Error"), wxOK | wxICON_ERROR);
        dial.ShowModal();
        mainThreadRunning = false;
        return;
    }

    //start detection on another thread
    mainThreadRunning = true;
    mainThread = std::thread(&Tracker::MainLoop, this);
    mainThread.detach();
}

void Tracker::CalibrateTracker()
{
    //initialize all parameters needed for tracker calibration

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

    AprilTagWrapper april{parameters};

    int markersPerTracker = parameters->markersPerTracker;
    int trackerNum = parameters->trackerNum;

    std::vector<cv::Vec3d> boardRvec, boardTvec;

    //add main marker to every tracker
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

    //reset current tracker calibration data
    trackers.clear();

    //run loop until we stop it
    while (cameraRunning && mainThreadRunning)
    {
        CopyFreshCameraImageTo(image);

        clock_t start;
        //clock for timing of detection
        start = clock();

        //detect and draw all markers on image
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f> > corners;
        std::vector<cv::Point2f> centers;

        april.detectMarkers(image, &corners, &ids, &centers,trackers);
        if (showTimeProfile)
        {
            april.drawTimeProfile(image, cv::Point(10, 60));
        }

        cv::aruco::drawDetectedMarkers(image, corners, cv::noArray(), cv::Scalar(255, 0, 0));       //draw all markers blue. We will overwrite this with other colors for markers that are part of any of the trackers that we use

        //estimate pose of our markers
        std::vector<cv::Vec3d> rvecs, tvecs;
        cv::aruco::estimatePoseSingleMarkers(corners, markerSize, parameters->camMat, parameters->distCoeffs, rvecs, tvecs);

        float maxDist = parameters->trackerCalibDistance;

        for (int i = 0; i < boardIds.size(); i++)           //for each of the trackers
        {
            cv::Ptr<cv::aruco::Board> arBoard = cv::aruco::Board::create(boardCorners[i], dictionary, boardIds[i]);         //create an aruco board object made out of already added markers to current tracker

            try
            {
                if (cv::aruco::estimatePoseBoard(corners, ids, arBoard, parameters->camMat, parameters->distCoeffs, boardRvec[i], boardTvec[i], false) > 0)         //try to estimate current trackers pose
                {
                    cv::aruco::drawAxis(image, parameters->camMat, parameters->distCoeffs, boardRvec[i], boardTvec[i], 0.1f);       //if found, draw axis and mark it found
                    boardFound[i] = true;
                }
                else
                {
                    boardFound[i] = false;          //else, if none of the markers for this tracker are visible, mark it not found
                }
            }
            catch (std::exception&)             //on weird images or calibrations, we get an error. This should usualy only happen on bad camera calibrations, or in very rare cases
            {
                wxString e = parameters->language.TRACKER_CALIBRATION_SOMETHINGWRONG;
                bool *mtr = &mainThreadRunning;
                gui->CallAfter([e, mtr, &win_OTU = parameters->octiuSah] ()
                                {
                                wxMessageDialog dial(NULL, e, wxT("Error"), wxOK | wxICON_ERROR);
                                dial.ShowModal();
                                cv::destroyWindow("Calibrating Tracker [" + win_OTU + "]");
                                *mtr = false;
                                });
                return;
                
            }

            std::string testStr = std::to_string(boardTvec[i][0]) + " " + std::to_string(boardTvec[i][1]) + " " + std::to_string(boardTvec[i][2]);

            bool foundMarkerToCalibrate = false;

            for (int j = 0; j < ids.size(); j++)        //check all of the found markers
            {
                if (ids[j] >= i * markersPerTracker && ids[j] < (i + 1) * markersPerTracker)            //if marker is part of current tracker (usualy, 0 is 0-44, 1 is 45-89 etc)
                {
                    bool markerInBoard = false;
                    for (int k = 0; k < boardIds[i].size(); k++)        //check if marker is already part of the tracker
                    {
                        if (boardIds[i][k] == ids[j])          
                        {
                            markerInBoard = true;
                            break;
                        }
                    }
                    if (markerInBoard == true)          //if it is, draw it green and continue to next marker
                    {
                        drawMarker(image, corners[j], cv::Scalar(0, 255, 0));
                        continue;
                    }
                    if (boardFound[i])                  //if it isnt part of the current tracker, but the tracker was detected, we will attempt to add it
                    {
                        if (sqrt(tvecs[j][0] * tvecs[j][0] + tvecs[j][1] * tvecs[j][1] + tvecs[j][2] * tvecs[j][2]) > maxDist)          //if marker is too far away from camera, we just paint it purple as adding it could have too much error
                        {
                            drawMarker(image, corners[j], cv::Scalar(255, 0, 255));
                            continue;
                        }

                        drawMarker(image, corners[j], cv::Scalar(0, 255, 255));         //start adding marker, mark that by painting it yellow
 
                        if (foundMarkerToCalibrate)                     //only calibrate one marker at a time, so continue loop if this is the second marker found
                            continue;

                        foundMarkerToCalibrate = true;


                        std::vector<cv::Point3f> marker;
                        transformMarkerSpace(modelMarker, boardRvec[i], boardTvec[i], rvecs[j], tvecs[j], &marker);         //transform marker points to the coordinate system of the tracker

                        int listIndex = -1;
                        for (int k = 0; k < idsList.size(); k++)            //check whether the idsList and cornersList already contains data for this marker
                        {
                            if (idsList[k] == ids[j])
                            {
                                listIndex = k;
                            }
                        }
                        if (listIndex < 0)                  //if not, add and initialize it
                        {
                            listIndex = idsList.size();
                            idsList.push_back(ids[j]);
                            cornersList.push_back(std::vector<std::vector<cv::Point3f>>());
                        }

                        cornersList[listIndex].push_back(marker);       //add the current marker corners to the list
                        if (cornersList[listIndex].size() > 50)         //if we have 50 recorded instances in the list for current marker, we can add it to the tracker
                        {
                            std::vector<cv::Point3f> medianMarker;

                            getMedianMarker(cornersList[listIndex], &medianMarker);         //calculate median position of each corner to get rid of outliers

                            boardIds[i].push_back(ids[j]);                                  //add the marker to the tracker
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
        //resize image, then show it
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
        cv::Mat *outImg = new cv::Mat();
        cv::resize(image, *outImg, cv::Size(cols,rows));
        gui->CallAfter([outImg, &win_OTU = parameters->octiuSah] ()
                        {
                        cv::imshow("Calibrating Tracker [" + win_OTU + "]", *outImg);
                        cv::waitKey(1);
                        delete(outImg);
                        });
    }

    //when done calibrating, save the trackers to parameters
    for (int i = 0; i < boardIds.size(); i++)
    {
        cv::Ptr<cv::aruco::Board> arBoard = cv::aruco::Board::create(boardCorners[i], dictionary, boardIds[i]);
        trackers.push_back(arBoard);
    }

    parameters->trackers = trackers;
    parameters->Save();
    trackersCalibrated = true;

    //close preview window
    bool *mtr = &mainThreadRunning;
    gui->CallAfter([mtr, &win_OTU = parameters->octiuSah] ()
                   {
                   cv::destroyWindow("Calibrating Tracker [" + win_OTU + "]");
                   *mtr = false;
                   });
}

void Tracker::MainLoop()
{

    int trackerNum = connection->connectedTrackers.size();
    int numOfPrevValues = parameters->numOfPrevValues;

    //these variables are used to save detections of apriltags, so we dont define them every frame

    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f> > corners;
    std::vector<cv::Point2f> centers;


    cv::Mat image, drawImg, ycc, gray, cr;

    cv::Mat  prevImg;


    //setup all variables that need to be stored for each tracker and initialize them
    std::vector<TrackerStatus> trackerStatus = std::vector<TrackerStatus>(trackerNum, TrackerStatus());
    for (int i = 0; i < trackerStatus.size(); i++)
    {
        trackerStatus[i].boardFound = false;
        trackerStatus[i].boardRvec = cv::Vec3d(0, 0, 0);
        trackerStatus[i].boardTvec = cv::Vec3d(0, 0, 0);
        trackerStatus[i].prevLocValues = std::vector<std::vector<double>>(7, std::vector<double>());
    }

    //previous values, used for moving median to remove any outliers.
    std::vector<double> prevLocValuesX;

    //the X axis, it is simply numbers 0-10 (or the amount of previous values we have)
    for (int j = 0; j < numOfPrevValues; j++)
    {
        prevLocValuesX.push_back(j);
    }


    AprilTagWrapper april{parameters};

    int framesSinceLastSeen = 0;
    int framesToCheckAll = 20;

    //calculate position of camera from calibration data and send its position to steamvr
    cv::Mat stationPos = (cv::Mat_<double>(4, 1) << 0, 0, 0, 1);
    stationPos = wtranslation * stationPos;

    Quaternion<double> stationQ = Quaternion<double>(0, 0, 1, 0) * (wrotation * Quaternion<double>(1, 0, 0, 0));

    double a = -stationPos.at<double>(0, 0);
    double b = stationPos.at<double>(1, 0);
    double c = -stationPos.at<double>(2, 0);

    connection->SendStation(0, a, b, c, stationQ.w, stationQ.x, stationQ.y, stationQ.z);

    //initialize variables for playspace calibration
    bool calibControllerPosActive = false;
    bool calibControllerAngleActive = false;
    clock_t calibControllerLastPress = clock();
    double calibControllerPosOffset[] = { 0,0,0 };
    double calibControllerAngleOffset[] = { 0,0,0 };

    std::vector<cv::Ptr<cv::aruco::Board>> trackers;
    std::vector<std::vector<int>> boardIds;
    std::vector<std::vector < std::vector<cv::Point3f >>> boardCorners;

    //by default, trackers have the center at the center of the main marker. If "Use centers of trackers" is checked, we move it to the center of all marker corners.
    if (parameters->trackerCalibCenters)
    {
        for (int i = 0; i < this->trackers.size(); i++)
        {
            boardCorners.push_back(this->trackers[i]->objPoints);
            boardIds.push_back(this->trackers[i]->ids);

            cv::Point3f boardCenter = cv::Point3f(0, 0, 0);
            int numOfCorners = 0;
            for (int j = 0; j < boardCorners[i].size(); j++)
            {
                for (int k = 0; k < boardCorners[i][j].size(); k++)
                {
                    boardCenter.x += boardCorners[i][j][k].x;
                    boardCenter.y += boardCorners[i][j][k].y;
                    boardCenter.z += boardCorners[i][j][k].z;
                    numOfCorners++;
                }
            }
            boardCenter /= numOfCorners;

            for (int j = 0; j < boardCorners[i].size(); j++)
            {
                for (int k = 0; k < boardCorners[i][j].size(); k++)
                {
                    boardCorners[i][j][k] -= boardCenter;
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


    //initialize CV out window
    gui->CallAfter([&win_OTU = parameters->octiuSah] ()
                   {
                   cv::namedWindow("Running on... " + win_OTU);
                   });

    while (mainThreadRunning && cameraRunning)      //run detection until camera is stopped or the start/stop button is pressed again
    {

        CopyFreshCameraImageTo(image);

        drawImg = image;
        april.convertToSingleChannel(image, gray);

        clock_t start, end;
        //for timing our detection
        start = clock();

        bool circularWindow = parameters->circularWindow;

        //if any tracker was lost for longer than 20 frames, mark circularWindow as false
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

            double frameTime = double(clock() - last_frame_time) / double(CLOCKS_PER_SEC);

            std::string word;
            std::istringstream ret = connection->Send("gettrackerpose " + std::to_string(i) + " " + std::to_string(-frameTime - parameters->camLatency));
            ret >> word;
            if (word != "trackerpose")
            {
                continue;
            }

            //first three variables are a position vector
            int idx; double a; double b; double c;

            //second four are rotation quaternion
            double qw; double qx; double qy; double qz;

            //last is if pose is valid: 0 is valid, 1 is late (hasnt been updated for more than 0.2 secs), -1 means invalid and is only zeros
            int tracker_pose_valid;

            //read to our variables
            ret >> idx; ret >> a; ret >> b; ret >> c; ret >> qw; ret >> qx; ret >> qy; ret >> qz; ret >> tracker_pose_valid;

            cv::Mat rpos = (cv::Mat_<double>(4, 1) << -a, b, -c, 1);

            //transform boards position from steamvr space to camera space based on our calibration data

            rpos.at<double>(3, 0) = 1;
            rpos = wtranslation.inv() * rpos;

            std::vector<cv::Point3d> point;
            point.push_back(cv::Point3d(rpos.at<double>(0, 0), rpos.at<double>(1, 0), rpos.at<double>(2, 0)));
            point.push_back(cv::Point3d(trackerStatus[i].boardTvec));

            std::vector<cv::Point2d> projected;
            cv::Vec3d rvec, tvec;

            //project point from position of tracker in camera 3d space to 2d camera pixel space, and draw a dot there
            cv::projectPoints(point, rvec, tvec, parameters->camMat, parameters->distCoeffs, projected);

            cv::circle(drawImg, projected[0], 5, cv::Scalar(0, 0, 255), 2, 8, 0);

            if (tracker_pose_valid == 0)        //if the pose from steamvr was valid, save the predicted position and rotation
            {

                Quaternion<double> q = Quaternion<double>(qw, qx, qy, qz);
                q = q.UnitQuaternion();

                q = wrotation.inverse() * Quaternion<double>(0, 0, 1, 0).inverse() * q * Quaternion<double>(0, 0, 1, 0).inverse();

                cv::Vec3d rvec = quat2rodr(q.w, q.x, q.y, q.z);
                cv::Vec3d tvec;
                tvec[0] = rpos.at<double>(0, 0);
                tvec[1] = rpos.at<double>(1, 0);
                tvec[2] = rpos.at<double>(2, 0);

                cv::aruco::drawAxis(drawImg, parameters->camMat, parameters->distCoeffs, rvec, tvec, 0.10);

                if (!trackerStatus[i].boardFound)       //if tracker was found in previous frame, we use that position for masking. If not, we use position from driver for masking.
                {
                    trackerStatus[i].maskCenter = projected[0];
                }
                else
                {
                    trackerStatus[i].maskCenter = projected[1];
                }

                trackerStatus[i].boardFound = true;
                trackerStatus[i].boardFoundDriver = true;
                trackerStatus[i].boardTvec = tvec;
                trackerStatus[i].boardTvecDriver = tvec;
                trackerStatus[i].boardRvec = rvec;

            }
            else
            {
                if (trackerStatus[i].boardFound)        //if pose is not valid, set everything based on previous known position
                {
                    trackerStatus[i].maskCenter = projected[1];
                }

                trackerStatus[i].boardFoundDriver = false;        //do we really need to do this? might be unnecessary
            }

        }

        //define our mask image. We want to create an image where everything but circles around predicted tracker positions will be black to speed up detection.
        cv::Mat mask = cv::Mat::zeros(gray.size(), gray.type());

        cv::Mat dstImage = cv::Mat::zeros(gray.size(), gray.type());

        int size = gray.rows * parameters->searchWindow;

        bool doMasking = false;

        for (int i = 0; i < trackerNum; i++)        //calculate the needed masks for every tracker
        {
            if (trackerStatus[i].maskCenter.x <= 0 || trackerStatus[i].maskCenter.y <= 0 || trackerStatus[i].maskCenter.x >= image.cols || trackerStatus[i].maskCenter.y >= image.rows)
            {
                trackerStatus[i].boardFound = false;    //if detected tracker is out of view of the camera, we mark it as not found, as either the prediction is wrong or we wont see it anyway
                continue;
            }
            doMasking = true;
            if (circularWindow)     //if circular window is set mask a circle around the predicted tracker point
            {
                cv::circle(mask, trackerStatus[i].maskCenter, size, cv::Scalar(255, 0, 0), -1, 8, 0);
                cv::circle(drawImg, trackerStatus[i].maskCenter, size, cv::Scalar(255, 0, 0), 2, 8, 0);
            }
            else            //if not, mask a vertical strip top to bottom. This happens every 20 frames if a tracker is lost.
            {
                rectangle(mask, cv::Point(trackerStatus[i].maskCenter.x - size, 0), cv::Point(trackerStatus[i].maskCenter.x + size, image.rows), cv::Scalar(255, 0, 0), -1);
                rectangle(drawImg, cv::Point(trackerStatus[i].maskCenter.x - size, 0), cv::Point(trackerStatus[i].maskCenter.x + size, image.rows), cv::Scalar(255, 0, 0), 3);
            }
        }

        //using copyTo with masking creates the image where everything but the locations where trackers are predicted to be is black
        if (doMasking)
        {
            gray.copyTo(dstImage, mask);
            gray = dstImage;
        }

        if (manualRecalibrate)          //playspace calibration loop
        {
            int inputButton = 0;
            inputButton = connection->GetButtonStates();

            double timeSincePress = double(start - calibControllerLastPress) / double(CLOCKS_PER_SEC);
            if (timeSincePress > 60)                                                                        //we exit playspace calibration after 60 seconds of no input detected, to try to prevent accidentaly ruining calibration
            {
                gui->cb3->SetValue(false);
                wxCommandEvent event(wxEVT_COMMAND_CHECKBOX_CLICKED, gui->MANUAL_CALIB_CHECKBOX);
                parentApp->ButtonPressedSpaceCalib(event);
            }

            if (inputButton == 1)       //logic for position button
            {
                double timeSincePress = double(start - calibControllerLastPress) / double(CLOCKS_PER_SEC);      //to prevent accidental double presses, 0.2 seconds must pass between presses.
                if (timeSincePress >= 0.2)
                {
                    if (!calibControllerPosActive)          //if position calibration is inactive, set it to active and calculate offsets between the camera and controller
                    {
                        calibControllerPosActive = true;
                        double pose[7];
                        connection->GetControllerPose(pose);

                        calibControllerPosOffset[0] = 100 * pose[0] - gui->manualCalibX->value;
                        calibControllerPosOffset[1] = 100 * pose[1] - gui->manualCalibY->value;
                        calibControllerPosOffset[2] = 100 * pose[2] - gui->manualCalibZ->value;

                        Quaternion<double> quat(pose[3], pose[4], pose[5], pose[6]);
                        quat.x = -quat.x;
                        quat.z = -quat.z;

                        quat.inverse().QuatRotation(calibControllerPosOffset);

                        calibControllerLastPress = clock();

                    }
                    else       //else, deactivate it
                    {
                        calibControllerPosActive = false;
                    }
                }
                calibControllerLastPress = clock();
            }
            if (inputButton == 2)       //logic for rotation button
            {
                double timeSincePress = double(start - calibControllerLastPress) / double(CLOCKS_PER_SEC);      //to prevent accidental double presses, 0.2 seconds must pass between presses.
                if (timeSincePress >= 0.2)
                {
                    if (!calibControllerAngleActive)          //if rotation calibration is inactive, set it to active and calculate angle offsets and distance
                    {
                        calibControllerAngleActive = true;
                        double pose[7];
                        connection->GetControllerPose(pose);

                        double xzLen = sqrt(pow(100 * pose[0] - gui->manualCalibX->value, 2) + pow(100 * pose[2] - gui->manualCalibZ->value, 2));
                        double angleA = atan2(100 * pose[1] - gui->manualCalibY->value, xzLen) * 57.3;
                        double angleB = atan2(100 * pose[0] - gui->manualCalibX->value, 100 * pose[2] - gui->manualCalibZ->value) * 57.3;
                        double xyzLen = sqrt(pow(100 * pose[0] - gui->manualCalibX->value, 2) + pow(100 * pose[1] - gui->manualCalibY->value, 2) + pow(100 * pose[2] - gui->manualCalibZ->value, 2));

                        calibControllerAngleOffset[0] = angleA - gui->manualCalibA->value;
                        calibControllerAngleOffset[1] = angleB - gui->manualCalibB->value;
                        calibControllerAngleOffset[2] = xyzLen;

                        calibControllerLastPress = clock();

                    }
                    else       //else, deactivate it
                    {
                        calibControllerAngleActive = false;
                    }
                }
                calibControllerLastPress = clock();
            }

            if (calibControllerPosActive)       //while position calibration is active, apply the camera to controller offset to X, Y and Z values
            {
                double pose[7];
                connection->GetControllerPose(pose);

                Quaternion<double> quat(pose[3], pose[4], pose[5], pose[6]);
                quat.x = -quat.x;
                quat.z = -quat.z;
                quat.QuatRotation(calibControllerPosOffset);

                gui->manualCalibX->SetValue(100 * pose[0] - calibControllerPosOffset[0]);

                if (!lockHeightCalib)       //if height is locked, dont change it
                {
                    gui->manualCalibY->SetValue(100 * pose[1] - calibControllerPosOffset[1]);
                }

                gui->manualCalibZ->SetValue(100 * pose[2] - calibControllerPosOffset[2]);

                quat.inverse().QuatRotation(calibControllerPosOffset);
            }

            if (calibControllerAngleActive)     //while rotation calibration is active, apply the camera to controller angle offsets to A, B, C values, and apply the calibScale based on distance from camera
            {
                double pose[7];
                connection->GetControllerPose(pose);

                double xzLen = sqrt(pow(100 * pose[0] - gui->manualCalibX->value, 2) + pow(100 * pose[2] - gui->manualCalibZ->value, 2));
                double angleA = atan2(100 * pose[1] - gui->manualCalibY->value, xzLen) * 57.3;
                double angleB = atan2(100 * pose[0] - gui->manualCalibX->value, 100 * pose[2] - gui->manualCalibZ->value) * 57.3;
                double xyzLen = sqrt(pow(100 * pose[0] - gui->manualCalibX->value, 2) + pow(100 * pose[1] - gui->manualCalibY->value, 2) + pow(100 * pose[2] - gui->manualCalibZ->value, 2));

                gui->manualCalibB->SetValue(angleB - calibControllerAngleOffset[1]);
                if (!lockHeightCalib)                                                   //if height is locked, do not calibrate up/down rotation or scale
                {
                    gui->manualCalibA->SetValue(angleA - calibControllerAngleOffset[0]);
                    calibScale = xyzLen / calibControllerAngleOffset[2];
                    if (calibScale > 1.2)
                        calibScale = 1.2;
                    else if (calibScale < 0.8)
                        calibScale = 0.8;
                }
            }

            //check that camera is facing correct direction. 90 degrees mean looking straight down, 270 is straight up. This ensures its not upside down.
            if (gui->manualCalibA->value < 90)
                gui->manualCalibA->SetValue(90);
            else if (gui->manualCalibA->value > 270)
                gui->manualCalibA->SetValue(270);

            //from the calculated position and angular values, calculate the playspace calibration transformation matrix and rotation quaternion

            cv::Vec3d calibRot(gui->manualCalibA->value * 0.01745, gui->manualCalibB->value * 0.01745, gui->manualCalibC->value * 0.01745);
            cv::Vec3d calibPos(gui->manualCalibX->value / 100, gui->manualCalibY->value / 100, gui->manualCalibZ->value / 100);
            cv::Vec3d calibRodr(cos(calibRot[0]) * cos(calibRot[1]) * 3.14, sin(calibRot[1]) * 3.14, sin(calibRot[0]) * cos(calibRot[1]) * 3.14);

            wtranslation = getSpaceCalibEuler(calibRot, cv::Vec3d(0, 0, 0), calibPos(0), calibPos(1), calibPos(2));

            wrotation = mRot2Quat(eulerAnglesToRotationMatrix(cv::Vec3f(calibRot)));

            cv::Mat stationPos = (cv::Mat_<double>(4, 1) << 0, 0, 0, 1);
            stationPos = wtranslation * stationPos;

            Quaternion<double> stationQ = Quaternion<double>(0, 0, 1, 0) * (wrotation * Quaternion<double>(1, 0, 0, 0));

            double a = -stationPos.at<double>(0, 0);
            double b = stationPos.at<double>(1, 0);
            double c = -stationPos.at<double>(2, 0);

            //move the camera in steamvr to new calibration
            connection->SendStation(0, a, b, c, stationQ.w, stationQ.x, stationQ.y, stationQ.z);
        }
        else
        {
            calibControllerLastPress = clock();
        }
        april.detectMarkers(gray, &corners, &ids, &centers, trackers);
        for (int i = 0; i < trackerNum; ++i) {

            //estimate the pose of current board

            try
            {
                trackerStatus[i].boardTvec /= calibScale;
                if (cv::aruco::estimatePoseBoard(corners, ids, trackers[connection->connectedTrackers[i].TrackerId], parameters->camMat, parameters->distCoeffs, trackerStatus[i].boardRvec, trackerStatus[i].boardTvec, trackerStatus[i].boardFound && parameters->usePredictive) <= 0)
                {
                    for (int j = 0; j < 6; j++)
                    {
                        //remove first of the previously saved values
                        if (trackerStatus[i].prevLocValues[j].size() > 0)
                            trackerStatus[i].prevLocValues[j].erase(trackerStatus[i].prevLocValues[j].begin());
                    }
                    trackerStatus[i].boardFound = false;

                    continue;
                }
            }
            catch (std::exception&)
            {

                wxString e = parameters->language.TRACKER_DETECTION_SOMETHINGWRONG; // on rare occasions, detection crashes. Should be very rare and indicate something wrong with camera or tracker calibration
                gui->CallAfter([e, &win_OTU = parameters->octiuSah] ()

                               {
                               wxMessageDialog dial(NULL, e, wxT("Error"), wxOK | wxICON_ERROR);
                                dial.ShowModal();
                               cv::destroyWindow("Running on... " + win_OTU);
                               });
                //apriltag_detector_destroy(td);
                mainThreadRunning = false;
                return;
            }
            trackerStatus[i].boardFound = true;

            trackerStatus[i].boardTvec *= calibScale;

            if (parameters->depthSmoothing > 0 && trackerStatus[i].boardFoundDriver && !manualRecalibrate)
            {
                //depth estimation is noisy, so try to smooth it more, especialy if using multiple cameras
                //if position is close to the position predicted by the driver, take the depth of the driver.
                //if error is big, take the calculated depth
                //error threshold is defined in the params as depth smoothing

                double distDriver = sqrt(pow(trackerStatus[i].boardTvecDriver[0], 2)
                    + pow(trackerStatus[i].boardTvecDriver[1], 2)
                    + pow(trackerStatus[i].boardTvecDriver[2], 2));

                double distPredict = sqrt(pow(trackerStatus[i].boardTvec[0], 2)
                    + pow(trackerStatus[i].boardTvec[1], 2)
                    + pow(trackerStatus[i].boardTvec[2], 2));

                cv::Vec3d normPredict = trackerStatus[i].boardTvec / distPredict;

                double dist = abs(distPredict - distDriver);

                dist = dist / parameters->depthSmoothing + 0.1;
                if (dist > 1)
                    dist = 1;

                double distFinal = (dist)*distPredict + (1 - dist) * distDriver;

                trackerStatus[i].boardTvec = normPredict * distFinal;

                //trackerStatus[i].boardTvec[2] = (dist) * trackerStatus[i].boardTvec[2] + (1-dist) * trackerStatus[i].boardTvecDriver[2];
            }

            double posValues[6] = {
                trackerStatus[i].boardTvec[0],
                trackerStatus[i].boardTvec[1],
                trackerStatus[i].boardTvec[2],
                trackerStatus[i].boardRvec[0],
                trackerStatus[i].boardRvec[1],
                trackerStatus[i].boardRvec[2] };

            for (int j = 0; j < 6; j++)
            {
                //push new values into previous values list end and remove the one on beggining
                trackerStatus[i].prevLocValues[j].push_back(posValues[j]);
                if (trackerStatus[i].prevLocValues[j].size() > numOfPrevValues)
                {
                    trackerStatus[i].prevLocValues[j].erase(trackerStatus[i].prevLocValues[j].begin());
                }

                std::vector<double> valArray(trackerStatus[i].prevLocValues[j]);
                //sort(valArray.begin(), valArray.end());

                //posValues[j] = valArray[valArray.size() / 2];
                posValues[j] = valArray[valArray.size() - 1];

            }
            //save fitted values back to our variables
            trackerStatus[i].boardTvec[0] = posValues[0];
            trackerStatus[i].boardTvec[1] = posValues[1];
            trackerStatus[i].boardTvec[2] = posValues[2];
            trackerStatus[i].boardRvec[0] = posValues[3];
            trackerStatus[i].boardRvec[1] = posValues[4];
            trackerStatus[i].boardRvec[2] = posValues[5];

            cv::Mat rpos = cv::Mat_<double>(4, 1);

            //transform boards position based on our calibration data

            for (int x = 0; x < 3; x++)
            {
                rpos.at<double>(x, 0) = trackerStatus[i].boardTvec[x];
            }
            rpos.at<double>(3, 0) = 1;
            rpos = wtranslation * rpos;

            //convert rodriguez rotation to quaternion
            Quaternion<double> q = rodr2quat(trackerStatus[i].boardRvec[0], trackerStatus[i].boardRvec[1], trackerStatus[i].boardRvec[2]);

            //cv::aruco::drawAxis(drawImg, parameters->camMat, parameters->distCoeffs, boardRvec[i], boardTvec[i], 0.05);

            q = Quaternion<double>(0, 0, 1, 0) * (wrotation * q) * Quaternion<double>(0, 0, 1, 0);

            double a = -rpos.at<double>(0, 0);
            double b = rpos.at<double>(1, 0);
            double c = -rpos.at<double>(2, 0);

            double factor;
            factor = parameters->smoothingFactor;

            if (factor < 0)
                factor = 0;
            else if (factor >= 1)
                factor = 0.99;

            end = clock();
            double frameTime = double(end - last_frame_time) / double(CLOCKS_PER_SEC);


            //send all the values
            //frame time is how much time passed since frame was acquired.
            if (!multicamAutocalib)
                connection->SendTracker(connection->connectedTrackers[i].DriverId, a, b, c, q.w, q.x, q.y, q.z, -frameTime - parameters->camLatency, factor);
            else if (trackerStatus[i].boardFoundDriver)                                                                                                             
            {
                //if calibration refinement with multiple cameras is active, do not send calculated poses to driver.
                //instead, refine the calibration data with gradient descent
                //the error is the diffrence of the detected trackers position to the estimated trackers position
                //numerical derivatives are then calculated to see how X,Y,Z, A,B, scale data affects the error in position
                //calibration values are then slightly changed in the estimated direction in order to reduce error.
                //after a couple of seconds, the calibration data should converge

                cv::Vec3d pose;
                pose = trackerStatus[i].boardTvec;
                double xzLen = sqrt(pow(100 * pose[0], 2) + pow(100 * pose[2], 2));
                double angleA = atan2(100 * pose[1], xzLen) * 57.3;
                double angleB = atan2(100 * pose[0], 100 * pose[2]) * 57.3;
                double xyzLen = sqrt(pow(100 * pose[0], 2) + pow(100 * pose[1], 2) + pow(100 * pose[2], 2));

                pose = trackerStatus[i].boardTvecDriver;
                double xzLenDriver = sqrt(pow(100 * pose[0], 2) + pow(100 * pose[2], 2));
                double angleADriver = atan2(100 * pose[1], xzLenDriver) * 57.3;
                double angleBDriver = atan2(100 * pose[0], 100 * pose[2]) * 57.3;
                double xyzLenDriver = sqrt(pow(100 * pose[0], 2) + pow(100 * pose[1], 2) + pow(100 * pose[2], 2));

                cv::Mat rpos1 = cv::Mat_<double>(4, 1);
                cv::Mat rpos2 = cv::Mat_<double>(4, 1);

                for (int x = 0; x < 3; x++)
                {
                    rpos1.at<double>(x, 0) = trackerStatus[i].boardTvec[x];
                }
                rpos1.at<double>(3, 0) = 1;
                rpos1 = wtranslation * rpos1;
                rpos1.at<double>(0, 0) = -rpos1.at<double>(0, 0);
                rpos1.at<double>(2, 0) = -rpos1.at<double>(2, 0);

                for (int x = 0; x < 3; x++)
                {
                    rpos2.at<double>(x, 0) = trackerStatus[i].boardTvecDriver[x];
                }
                rpos2.at<double>(3, 0) = 1;
                rpos2 = wtranslation * rpos2;
                rpos2.at<double>(0, 0) = -rpos2.at<double>(0, 0);
                rpos2.at<double>(2, 0) = -rpos2.at<double>(2, 0);

                double dX = (rpos1.at<double>(0) - rpos2.at<double>(0)) * 0.1;
                double dY = -(rpos1.at<double>(1) - rpos2.at<double>(1)) * 0.1;
                double dZ = (rpos1.at<double>(2) - rpos2.at<double>(2)) * 0.1;

                if (abs(dX) > 0.01)
                    dX = 0.1 * (dX / abs(dX));
                if (abs(dY) > 0.1)
                    dY = 0.1 * (dY / abs(dY));
                if (abs(dZ) > 0.1)
                    dZ = 0.1 * (dZ / abs(dZ));

                gui->manualCalibX->SetValue(gui->manualCalibX->value + dX);
                gui->manualCalibY->SetValue(gui->manualCalibY->value + dY);
                gui->manualCalibZ->SetValue(gui->manualCalibZ->value + dZ);

                gui->manualCalibA->SetValue(gui->manualCalibA->value + 0.1 * (angleA - angleADriver));
                gui->manualCalibB->SetValue(gui->manualCalibB->value + 0.1 * (angleB - angleBDriver));
                calibScale = calibScale - 0.1 * (1 - (xyzLenDriver / xyzLen));

                if (calibScale > 1.2)
                    calibScale = 1.2;
                else if (calibScale < 0.8)
                    calibScale = 0.8;

            }


        }

        //draw and display the detections
        if (ids.size() > 0)
            cv::aruco::drawDetectedMarkers(drawImg, corners, ids);

        end = clock();
        double frameTime = double(end - start) / double(CLOCKS_PER_SEC);

        if (!disableOut)
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
            cv::Mat *outImg = new cv::Mat();
            cv::resize(drawImg, *outImg, cv::Size(cols, rows));
            cv::putText(*outImg, std::to_string(frameTime).substr(0, 5), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
            if (showTimeProfile)
            {
                april.drawTimeProfile(*outImg, cv::Point(10, 60));
            }            
            
            gui->CallAfter([outImg, &win_OTU = parameters->octiuSah] ()
                           {
                           cv::imshow("Running on... " + win_OTU, *outImg);
                           cv::waitKey(1);
                           delete(outImg);
                           });
        }
        //time of marker detection
    }
    gui->CallAfter([&win_OTU = parameters->octiuSah] ()
                   {
                   cv::destroyWindow("Running on... " + win_OTU);
                   });
}
