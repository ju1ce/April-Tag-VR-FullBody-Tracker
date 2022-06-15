#pragma once

#include "RefPtr.h"

#include <opencv2/core.hpp>
#include <wx/frame.h>
#include <wx/graphics.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/timer.h>
#include <wx/window.h>

#include <atomic>
#include <memory>
#include <mutex>

class PreviewRenderLoop;

class PreviewPane : public wxPanel
{
public:
    friend class PreviewRenderLoop;
    PreviewPane(RefPtr<wxWindow> parent, wxWindowID _id);

    /// thread safe
    void UpdateImage(const cv::Mat& image);
    void ShowPane();
    void HidePane();

private:
    /// render loop updates per second
    static constexpr int RENDER_UPS = 30;
    void OnPaintEvent(wxPaintEvent& evt);

    wxWindowIDRef id;
    const std::unique_ptr<PreviewRenderLoop> renderLoop;
    /// Stores the image data that wxImage points to.
    /// DrawImage will copy to it during rgb conversion, no need to reallocate.
    cv::Mat image, swapImage;
    bool newImageReady = false;
    std::mutex imageMutex;

    wxGraphicsBitmap bitmap = wxNullGraphicsBitmap;
    wxGraphicsBrush backgroundBrush = wxNullGraphicsBrush;
};

class PreviewRenderLoop : private wxTimer
{
public:
    PreviewRenderLoop(PreviewPane& pane);

    /// If updatesPerSecond is positive it starts the render loop with that update speed,
    /// if its -1 it starts the loop with the previously set update speed.
    void StartLoop(int updatesPerSecond = -1);
    void StopLoop() { wxTimer::Stop(); };
    bool IsRunning() const { return wxTimer::IsRunning(); }

private:
    void Notify() override;
    PreviewPane& pane;
};

class PreviewFrame
{
public:
    PreviewFrame(const wxString& _title);

    enum class CloseButton
    {
        Show,
        Hide,
    };

    void UpdateImage(const cv::Mat& image);
    void Show(CloseButton closeButton = CloseButton::Show);
    void Hide();
    bool IsVisible();

private:
    wxString title;
    wxWindowIDRef id;

    static constexpr auto FrameDestroyer = [](wxFrame* f)
    {
        f->Destroy();
    };
    std::unique_ptr<wxFrame, decltype(FrameDestroyer)> frame;
    std::mutex frameLock;
    /// owned by frame, so exists if frame is not null
    RefPtr<PreviewPane> pane;
};
