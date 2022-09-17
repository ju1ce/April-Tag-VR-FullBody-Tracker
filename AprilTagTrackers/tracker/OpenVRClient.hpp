#pragma once

#include "Helpers.hpp"

#include <opencv2/core/types.hpp>
#include <openvr.h>

namespace tracker
{

enum class ButtonAction
{
    None,
    GrabCamera,
    GrabTrackers
};

class OpenVRClient
{
    static constexpr auto VRContextDeleter = [](vr::IVRSystem*)
    { vr::VR_Shutdown(); };
    using VRContextHandle = std::unique_ptr<vr::IVRSystem, decltype(VRContextDeleter)>;

public:
    /// fast check if init will fail
    bool CanInitializeOpenVR() const;

    void Init();
    /// gets called in destructor
    void Shutdown();
    /// can shutdown openvr
    void PollEvents();

    /// call before GetXAction for this frame
    void UpdateInputActions() const;
    /// buttons are only active during the first frame they are pressed
    ButtonAction GetButtonAction() const;
    Pose GetControllerPoseAction() const;

    bool IsInitialized() const { return static_cast<bool>(mContext); }

private:
    VRContextHandle mContext{nullptr, VRContextDeleter};
    vr::VRActionHandle_t mActionGrabCamera = 0;
    vr::VRActionHandle_t mActionGrabTrackers = 0;
    vr::VRActionHandle_t mActionLeftControllerPose = 0;
    vr::VRActionSetHandle_t mActionSet = 0;
};

} // namespace tracker
