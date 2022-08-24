#include "GUI.hpp"

#include "Debug.hpp"
#include "GUI/MainFrame.hpp"

namespace
{
// The arguments will have references stripped and then get moved/copied to the lambda.
// Reference args on a MainFrame function will then point to lambdas copy captures,
// and then destroyed by wxWidgets after the lambda is used.
// Because the CallAfter could happen after the lifetime of the callsites stack.
// Use a RefPtr for the arg if the lifetime will exist for a few frames,
// move a std::unique_ptr if the function will handle its lifetime,
// or a std::shared_ptr if the lifetime is unknown.
// Many of opencv's types are internally reference counted (like cv::Mat) and can be passed by value.

/// Wrap a MainFrame function in CallAfter, to allow it to be called from any thread
template <typename... ArgTs>
inline void ForwardToMainThread(
    RefPtr<GUI::MainFrame> impl,
    void (GUI::MainFrame::*func)(ArgTs...),
    std::remove_reference_t<ArgTs>&&... args)
{
    // func is a pointer, args are moved to a tuple.
    // impl is the first element in the tuple like with std::invoke(), the this pointer
    impl->CallOnMainThread(
        [func, args = std::make_tuple(impl.Get(), std::move(args)...)]
        {
            std::apply(func, std::move(args));
        });
}
} // namespace

GUI::GUI(RefPtr<ITrackerControl> _tracker, const Localization& _lc, UserConfig& _config)
    : impl(new MainFrame(_tracker, _lc, _config))
{
    _tracker->SetGUI(this);
}

GUI::~GUI() = default;

void GUI::ShowPrompt(U8String msg, std::function<void(bool)> onClose)
{
    ForwardToMainThread(impl, &MainFrame::ShowPrompt, std::move(msg), std::move(onClose));
}

void GUI::ShowPopup(U8String msg, PopupStyle style)
{
    ForwardToMainThread(impl, &MainFrame::ShowPopup, std::move(msg), std::move(style));
}

void GUI::SetStatus(bool status, StatusItem item)
{
    ForwardToMainThread(impl, &MainFrame::SetStatus, std::move(status), std::move(item));
}

PreviewControl GUI::CreatePreviewControl(PreviewId id)
{
    return PreviewControl{this, id};
}

void GUI::SetPreviewVisible(bool visible, PreviewId id, bool userCanDestroy)
{
    ForwardToMainThread(impl, &MainFrame::SetPreviewVisible, std::move(visible), std::move(id), std::move(userCanDestroy));
}

void GUI::UpdatePreview(const cv::Mat& image, PreviewId id)
{
    impl->UpdatePreview(image, id);
}

bool GUI::IsPreviewVisible(PreviewId id)
{
    return impl->IsPreviewVisible(id);
}

ManualCalib::Real GUI::GetManualCalib()
{
    return impl->GetManualCalib();
}

void GUI::SetManualCalib(const ManualCalib::Real& calib)
{
    impl->SetManualCalib(calib);
}

void GUI::SetManualCalibVisible(bool visible)
{
    ForwardToMainThread(impl, &MainFrame::SetManualCalibVisible, std::move(visible));
}

PreviewControl::PreviewControl(RefPtr<GUI> _gui, PreviewId _id)
    : gui(_gui), id(_id)
{
    gui->SetPreviewVisible(true, id, false);
}

PreviewControl::~PreviewControl()
{
    gui->SetPreviewVisible(false, id);
}

void PreviewControl::Update(const cv::Mat& image)
{
    gui->UpdatePreview(image, id);
}

bool PreviewControl::IsVisible()
{
    return gui->IsPreviewVisible(id);
}
