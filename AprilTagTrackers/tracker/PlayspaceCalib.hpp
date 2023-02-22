#pragma once

#include "config/ManualCalib.hpp"
#include "GUI.hpp"
#include "OpenVRClient.hpp"
#include "RefPtr.hpp"
#include "TrackerUnit.hpp"
#include "utils/SteadyTimer.hpp"
#include "VRDriver.hpp"

namespace tracker
{

class PlayspaceCalib
{
public:
    void Set(const cv::Vec3d& posOffset, const cv::Vec3d& angleOffset, double scale)
    {
        cv::Matx33d rotMat = EulerAnglesToRotationMatrix(angleOffset);
        mTransform = cv::Affine3d(rotMat, posOffset);
        mRotation = cv::Quatd::createFromRotMat(rotMat).normalize();
        mInvTransform = mTransform.inv();
        mInvRotation = mRotation.inv();
        mScale = scale;
    }
    void Set(const cfg::ManualCalib::Real& calib)
    {
        Set(calib.posOffset, calib.angleOffset, calib.scale);
    }

    //NOTE: should also apply scale?
    Pose Transform(const Pose& pose) const
    {
        return {mTransform * pose.position, mRotation * pose.rotation};
    }
    Pose InvTransform(const Pose& pose) const
    {
        return {mInvTransform * pose.position, mInvRotation * pose.rotation};
    }
    cv::Point3d Transform(const cv::Point3d& pos) const { return mTransform * pos; }

    Pose TransformToOVR(Pose pose) const
    {
        CoordTransformOVR(pose.position);
        CoordTransformOVR(pose.rotation);
        return Transform(pose);
    }
    Pose InvTransformFromOVR(const Pose& pose) const
    {
        Pose p = InvTransform(pose);
        CoordTransformOVR(p.position);
        CoordTransformOVR(p.rotation);
        return p;
    }

    Pose GetStationPose() const { return {mTransform.translation(), mRotation}; }
    Pose GetStationPoseOVR() const
    {
        Pose pose = GetStationPose();
        CoordTransformOVR(pose.position);
        CoordTransformOVR(pose.rotation);
        return pose;
    }
    double GetScale() const { return mScale; }

private:
    cv::Affine3d mTransform{};
    cv::Quatd mRotation{};
    cv::Affine3d mInvTransform{};
    cv::Quatd mInvRotation{};
    double mScale = 0;
};

class PlayspaceCalibrator
{
public:
    void Update(RefPtr<tracker::IVRClient> mVRClient,
                RefPtr<tracker::VRDriver> mVRDriver,
                RefPtr<GUI> gui,
                RefPtr<PlayspaceCalib> playspace,
                bool lockHeightCalib,
                bool enabled)
    {
        if (!enabled)
        {
            timer.Restart();
            return;
        }

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
        playspace->Set(gui->GetManualCalib());
        mVRDriver->UpdateStation(playspace->GetStationPoseOVR());
    }

    static void UpdateMulticam(RefPtr<GUI> gui,
                               RefPtr<PlayspaceCalib> playspace,
                               TrackerUnit& unit)
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

        pos = playspace->Transform(pos);
        driverPos = playspace->Transform(driverPos);

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
        playspace->Set(calib);
    }

private:
    bool posActive = false;
    bool angleActive = false;
    /// offset of controller to virtual camera, set when button is pressed
    cv::Vec3d posOffset{};
    /// (pitch, yaw) in radians, 3rd component is distance
    cv::Vec3d angleOffset{};
    utils::SteadyTimer timer{};
};

} // namespace tracker
