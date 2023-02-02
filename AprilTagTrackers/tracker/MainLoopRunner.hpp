#pragma once

#include "AprilTagWrapper.hpp"
#include "GUI.hpp"
#include "OpenVRClient.hpp"
#include "PlayspaceCalib.hpp"
#include "RefPtr.hpp"
#include "TrackerUnit.hpp"
#include "VideoCapture.hpp"
#include "VRDriver.hpp"

namespace tracker
{

class MainLoopRunner
{
    static constexpr int DRAW_IMG_SIZE = 480; // TODO: make configurable (preview image scaler)
    static inline const cv::Scalar COLOR_MASK{255, 0, 0}; /// red

public:
    explicit MainLoopRunner(RefPtr<UserConfig> config,
                            RefPtr<const CalibrationConfig> calibConfig,
                            RefPtr<PlayspaceCalib> playspace,
                            RefPtr<VRDriver> vrDriver)
        : mConfig(config),
          camCalib(calibConfig->cameras[0]),
          videoStream(mConfig->videoStreams[0]),
          april(AprilTagWrapper::ConvertFamily(mConfig->markerLibrary), videoStream->quadDecimate, mConfig->apriltagThreadCount),
          trackerNum(mConfig->trackerNum),
          mPlayspace(playspace),
          mVRDriver(vrDriver)
    {
        mPlayspace->Set(mConfig->manualCalib.GetAsReal());
        // calculate position of camera from calibration data and send its position to steamvr
        mVRDriver->UpdateStation(mPlayspace->GetStationPoseOVR());
    }

    void Update(RefPtr<AwaitedFrame> cameraFrame,
                RefPtr<GUI> gui,
                RefPtr<std::vector<TrackerUnit>> trackerUnits,
                RefPtr<IVRClient> vrClient,
                RefPtr<const ITrackerControl> trackerCtrl)
    {
        cameraFrame->Get(frame);
        // shallow copy, gray will be cloned from image and used for detection,
        // so drawing can happen on color image without clone.
        drawImg = frame.image;
        AprilTagWrapper::ConvertGrayscale(frame.image, grayAprilImg);
        const bool previewIsVisible = gui->IsPreviewVisible();

        const auto stampBeforeDetect = utils::SteadyTimer::Now();
        detectionTimer.Restart(stampBeforeDetect);

        bool circularWindow = videoStream->circularWindow;

        // if any tracker was lost for longer than 20 frames, mark circularWindow as false
        for (const auto& unit : *trackerUnits)
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
            auto& unit = (*trackerUnits)[i];
            auto [pose, isValid] = mVRDriver->GetTracker(i, -frameTimeBeforeDetect - videoStream->latency);

            if(isValid)
                pose = mPlayspace->InvTransformFromOVR(pose);

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
                if (previewIsVisible) cv::drawFrameAxes(drawImg, camCalib->cameraMatrix, camCalib->distortionCoeffs, pose.rotation.toRotVec(), math::ToVec(pose.position), 0.10F);

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

        mCalibrator.Update(vrClient, mVRDriver, gui, mPlayspace, trackerCtrl->lockHeightCalib, trackerCtrl->manualRecalibrate);

        april.DetectMarkers(grayAprilImg, dets);
        // frame time is how much time passed since frame was acquired.
        const double frameTimeAfterDetect = duration_cast<utils::FSeconds>(utils::SteadyTimer::Now() - frame.timestamp).count();
        for (int index = 0; index < trackerUnits->size(); ++index)
        {
            auto& unit = (*trackerUnits)[index];
            // estimate the pose of current board
            const RodrPose scaledPoseFromDriver{unit.GetPoseFromDriver().position / mPlayspace->GetScale(), unit.GetPoseFromDriver().rotation};
            // on rare occasions, detection crashes. Should be very rare and indicate something wrong with camera or tracker calibration
            auto [estimatedPose, numEstimated] = math::EstimatePoseTracker(
                dets.corners, dets.ids, unit.GetArucoBoard(), *camCalib,
                unit.WasVisibleLastFrame() && mConfig->usePredictive,
                scaledPoseFromDriver);
            estimatedPose.position *= mPlayspace->GetScale(); // unscale returned estimation;
            unit.SetEstimatedPose(estimatedPose);

            ATT_ASSERT(!std::isnan(estimatedPose.position[X]));

            if (numEstimated <= 0)
            {
                unit.SetWasVisibleLastFrame(false);
                continue;
            }
            unit.SetWasVisibleLastFrame(true);

            if (mConfig->depthSmoothing > 0 && unit.WasVisibleToDriverLastFrame() && !trackerCtrl->manualRecalibrate)
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
                dist = (dist / static_cast<double>(mConfig->depthSmoothing)) + 0.1;
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

            if (trackerCtrl->multicamAutocalib && unit.WasVisibleToDriverLastFrame())
            {
                tracker::PlayspaceCalibrator::UpdateMulticam(gui, mPlayspace, unit);
                continue; // skip sending to driver
            }

            // transform boards position based on our calibration data
            Pose poseToSend = mPlayspace->TransformToOVR(Pose(unit.GetEstimatedPose()));

            // send all the values
            mVRDriver->UpdateTracker(index, poseToSend, -frameTimeAfterDetect - videoStream->latency, mConfig->smoothingFactor);
        }

        if (gui->IsPreviewVisible())
        {
            // draw and display the detections
            if (!dets.ids.empty()) cv::aruco::drawDetectedMarkers(drawImg, dets.corners, dets.ids);
            const cv::Size2i drawSize = ConstrainSize(GetMatSize(frame.image), DRAW_IMG_SIZE);
            cv::resize(drawImg, outImg, drawSize);
            cv::putText(outImg, std::to_string(frameTimeAfterDetect).substr(0, 5), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));
            if (false) // TODO: tracker->showTimeProfile (is this even needed?)
            {
                april.DrawTimeProfile(outImg, cv::Point(10, 60));
            }
            gui->UpdatePreview(outImg);
        }
        // time of marker detection
    }

private:
    RefPtr<UserConfig> mConfig;
    RefPtr<const cfg::CameraCalib> camCalib;
    RefPtr<const cfg::VideoStream> videoStream;
    AprilTagWrapper april;
    Index trackerNum;
    RefPtr<PlayspaceCalib> mPlayspace;
    RefPtr<VRDriver> mVRDriver;

    MarkerDetectionList dets{};

    tracker::CapturedFrame frame{};
    cv::Mat drawImg{};
    cv::Mat outImg{};
    cv::Mat grayAprilImg{};
    cv::Mat maskSearchImg{};
    cv::Mat tempGrayMaskedImg{};

    int framesSinceLastSeen = 0;
    static constexpr int framesToCheckAll = 20;
    PlayspaceCalibrator mCalibrator{};

    utils::SteadyTimer detectionTimer{};
};

} // namespace tracker
