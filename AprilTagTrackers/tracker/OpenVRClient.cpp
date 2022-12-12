#include "OpenVRClient.hpp"

#include "Helpers.hpp"
#include "utils/Env.hpp"

#include <optional>
#include <stdexcept>
#include <string>

namespace
{

inline void ThrowVRError(vr::EVRInitError ec)
{
    throw std::runtime_error(vr::VR_GetVRInitErrorAsEnglishDescription(ec));
}
inline void ThrowVRError(vr::EVRInputError ec)
{
    throw std::runtime_error("VRInputError: " + std::to_string(ec));
}
template <typename T>
inline void ThrowIfError(T errorCode)
{
    if (static_cast<int>(errorCode) == 0) return;
    ThrowVRError(errorCode);
}

/// update actions for this frame
inline void UpdateActionSetState(vr::VRActionSetHandle_t actionSet)
{
    vr::VRActiveActionSet_t activeSet{};
    activeSet.ulActionSet = actionSet;
    vr::VRInput()->UpdateActionState(&activeSet, sizeof(activeSet), 1);
}

/// call after UpdateActionSetState
inline bool GetDigitalState(vr::VRActionHandle_t action)
{
    vr::InputDigitalActionData_t actionData{};
    ThrowIfError(vr::VRInput()->GetDigitalActionData(action, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle));
    return actionData.bActive && actionData.bState;
}

std::optional<vr::HmdMatrix34_t> GetPoseForNextFrame(vr::VRActionHandle_t action)
{
    vr::InputPoseActionData_t actionData{};
    constexpr vr::ETrackingUniverseOrigin origin = vr::TrackingUniverseRawAndUncalibrated;
    ThrowIfError(vr::VRInput()->GetPoseActionDataForNextFrame(action, origin, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle));
    if (!actionData.bActive || !actionData.pose.bPoseIsValid) return std::nullopt;
    return actionData.pose.mDeviceToAbsoluteTracking;
}

Pose OVRPoseMatrixToPose(vr::HmdMatrix34_t matrix)
{
    const cv::Matx33d rotMat(
        matrix.m[0][0], matrix.m[0][1], matrix.m[0][2],
        matrix.m[1][0], matrix.m[1][1], matrix.m[1][2],
        matrix.m[2][0], matrix.m[2][1], matrix.m[2][2]);
    cv::Quatd rot = cv::Quatd::createFromRotMat(rotMat).normalize();

    cv::Point3d pos{matrix.m[0][3], matrix.m[1][3], matrix.m[2][3]};

    // openvr is +x right, +y up, -z forward
    // opencv is +x right, -y up, +z forward
    // So negate y and z of pos and rot to convert
    // Except thats not true?
    // transform to -x right, +y up, +z forward
    // which is apparently what ATT uses sometimes
    CoordTransformOVR(pos);
    CoordTransformOVR(rot);

    return Pose{pos, rot};
}

} // namespace

namespace tracker
{

bool OpenVRClient::CanInit() const
{
    if (IsInit()) return false;
    return vr::VR_IsHmdPresent();
}

void OpenVRClient::Init()
{
    if (mContext) throw std::runtime_error("openvr already initialized");

    vr::EVRInitError initError = vr::VRInitError_None;
    vr::IVRSystem* context = vr::VR_Init(&initError, vr::VRApplication_Overlay);
    if (initError != 0 || context == nullptr) ThrowVRError(initError);
    mContext.reset(context);
    ATT_LOG_INFO("initialized openvr client");

    const auto manifestPath = utils::GetBindingsDir() / "att_actions.json";
    if (!std::filesystem::exists(manifestPath)) throw std::runtime_error("action manifest not found");

    ThrowIfError(vr::VRInput()->SetActionManifestPath(manifestPath.generic_string().c_str()));
    ThrowIfError(vr::VRInput()->GetActionHandle("/actions/demo/in/grab_camera", &mActionGrabCamera));
    ThrowIfError(vr::VRInput()->GetActionHandle("/actions/demo/in/grab_trackers", &mActionGrabTrackers));
    ThrowIfError(vr::VRInput()->GetActionHandle("/actions/demo/in/Hand_Left", &mActionLeftControllerPose));
    ThrowIfError(vr::VRInput()->GetActionSetHandle("/actions/demo", &mActionSet));
    ATT_LOG_INFO("openvr action manifest: ", manifestPath);
}

void OpenVRClient::Shutdown()
{
    if (!mContext) throw std::runtime_error("openvr not initialized");
    mContext.reset();
}

void OpenVRClient::PollEvents()
{
    if (!mContext) throw std::runtime_error("openvr not initialized");
    vr::VREvent_t event{};
    while (mContext->PollNextEvent(&event, sizeof(event)))
    {
        if (event.eventType == vr::VREvent_Quit)
        {
            // close connection to steamvr without closing att
            mContext->AcknowledgeQuit_Exiting();
            mContext.reset();
            return;
        }
    }
}

void OpenVRClient::UpdateInputActions() const
{
    if (!mContext) throw std::runtime_error("openvr not initialized");
    UpdateActionSetState(mActionSet);
}

ButtonAction OpenVRClient::GetButtonAction() const
{
    if (!mContext) throw std::runtime_error("openvr not initialized");
    if (GetDigitalState(mActionGrabCamera)) return ButtonAction::GrabCamera;
    if (GetDigitalState(mActionGrabTrackers)) return ButtonAction::GrabTrackers;
    return ButtonAction::None;
}

Pose OpenVRClient::GetControllerPoseAction() const
{
    if (!mContext) throw std::runtime_error("openvr not initialized");
    if (const auto pose = GetPoseForNextFrame(mActionLeftControllerPose))
    {
        return OVRPoseMatrixToPose(*pose);
    }
    return Pose::Ident();
}

} // namespace tracker
