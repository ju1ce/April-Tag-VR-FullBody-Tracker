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

class IVRClient
{
public:
    virtual ~IVRClient() = default;
    virtual bool CanInit() const { return true; } /// fast check if init will succeed
    virtual void Init() = 0;
    virtual void PollEvents() = 0;                    /// events are able to shutdown vr
    virtual void UpdateInputActions() const = 0;      /// call before GetXAction for this frame
    virtual ButtonAction GetButtonAction() const = 0; /// buttons are only active during the first frame they are pressed
    virtual Pose GetControllerPoseAction() const = 0;
    virtual bool IsInit() const = 0;
};

class OpenVRClient final : public IVRClient
{
    static constexpr auto VRContextDeleter = [](vr::IVRSystem*)
    { vr::VR_Shutdown(); };
    using VRContextHandle = std::unique_ptr<vr::IVRSystem, decltype(VRContextDeleter)>;

public:
    bool CanInit() const final;
    void Init() final;
    void Shutdown(); /// gets called in destructor
    void PollEvents() final;

    void UpdateInputActions() const final;
    ButtonAction GetButtonAction() const final;
    Pose GetControllerPoseAction() const final;

    bool IsInit() const final { return static_cast<bool>(mContext); }

private:
    VRContextHandle mContext{nullptr, VRContextDeleter};
    vr::VRActionHandle_t mActionGrabCamera = 0;
    vr::VRActionHandle_t mActionGrabTrackers = 0;
    vr::VRActionHandle_t mActionLeftControllerPose = 0;
    vr::VRActionSetHandle_t mActionSet = 0;
};

class MockOpenVRClient final : public IVRClient
{
public:
    bool CanInit() const final { return true; }
    void Init() final
    {
        ATT_ASSERT(!mIsInit);
        mIsInit = true;
    }
    void PollEvents() final { ATT_ASSERT(mIsInit); }

    void UpdateInputActions() const final { ATT_ASSERT(mIsInit); }
    ButtonAction GetButtonAction() const final
    {
        ATT_ASSERT(mIsInit);
        return ButtonAction::None;
    }
    Pose GetControllerPoseAction() const final
    {
        ATT_ASSERT(mIsInit);
        return Pose::Ident();
    }
    bool IsInit() const final { return mIsInit; }

private:
    bool mIsInit = false;
};

} // namespace tracker
