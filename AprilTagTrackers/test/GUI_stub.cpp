#include "GUI.hpp"
#include "utils/Assert.hpp"

class GUI::MainFrame
{
public:
    MainFrame(RefPtr<ITrackerControl> _tracker, const Localization& _lc, UserConfig& _config) {}
};

GUI::GUI(RefPtr<ITrackerControl> _tracker, const Localization& _lc, UserConfig& _config)
    : impl(new MainFrame(_tracker, _lc, _config))
{
    _tracker->SetGUI(this);
}

GUI::~GUI() = default;

void GUI::ShowPrompt(U8String msg, std::function<void(bool)> onClose)
{
}

void GUI::ShowPopup(U8String msg, PopupStyle style)
{
}

void GUI::SetStatus(bool status, StatusItem item)
{
}

PreviewControl GUI::CreatePreviewControl(PreviewId id)
{
    return PreviewControl{this, id};
}

void GUI::SetPreviewVisible(bool visible, PreviewId id, bool userCanDestroy)
{
}

void GUI::UpdatePreview(const cv::Mat& image, PreviewId id)
{
}

bool GUI::IsPreviewVisible(PreviewId id)
{
    return true;
}

cfg::ManualCalib::Real GUI::GetManualCalib()
{
    return cfg::ManualCalib::Real{cv::Vec3d{}, cv::Vec3d{}, 0};
}

void GUI::SetManualCalib(const cfg::ManualCalib::Real& calib)
{
}

void GUI::SetManualCalibVisible(bool visible)
{
}

PreviewControl::PreviewControl(RefPtr<GUI> _gui, PreviewId _id)
    : gui(_gui), id(_id)
{
}

PreviewControl::~PreviewControl()
{
}

void PreviewControl::Update(const cv::Mat& image)
{
}

bool PreviewControl::IsVisible()
{
    return true;
}
