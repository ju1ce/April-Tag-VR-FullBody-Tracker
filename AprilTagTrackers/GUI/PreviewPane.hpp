#pragma once

#include "Config.hpp"
#include "RefPtr.hpp"
#include "utils/Assert.hpp"
#include "utils/Env.hpp"
#include "wxHelpers.hpp"

#include <opencv2/core.hpp>
#include <wx/frame.h>
#include <wx/graphics.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/timer.h>
#include <wx/window.h>

#include <memory>
#include <mutex>

class PreviewPane
{
    /// minimum size of panel to attempt to draw anything
    static constexpr int MIN_DRAW_SIZE = 20;
    /// render loop updates per second
    static constexpr int RENDER_UPS = 30;
    /// can only be created in a paint event
    static inline wxGraphicsBrush backgroundBrush = wxNullGraphicsBrush;

    /// render loop without blocking gui thread, handled by wx event loop
    class RenderLoop : private wxTimer
    {
    public:
        RenderLoop(PreviewPane& _parentPane) : parentPane(_parentPane) {}
        /// calls Notify at updates per second
        void StartLoop(int ups) { wxTimer::Start(1000 / ups); }
        void StopLoop() { wxTimer::Stop(); }

    private:
        /// render loop body
        void Notify() final { parentPane.Repaint(); }
        PreviewPane& parentPane;
    };

public:
    PreviewPane() : renderLoop(std::make_unique<RenderLoop>(*this)) {}

    /// @return call someParent->SetSizer(result.release())
    /// or someSizer->Add(result.release()) to place the returned panel
    std::unique_ptr<wxBoxSizer> Create(RefPtr<wxWindow> _parent, wxSize size);
    void Destroy();

    void Show();
    void Hide();

    void UpdateImage(const cv::Mat& newImage);
    bool IsVisible() const { return isVisible; }

private:
    static void ClearBackground(RefPtr<wxGraphicsContext> context, wxSize drawArea);

    void Repaint();

    /// @return new aspect ratio if changed, otherwise negative
    float SetImage(const cv::Mat& newImage);

    /// @return whether the image was swapped
    bool SwapReadWriteImage();

    /// sets the aspect ratio of panel for wxSHAPED
    void SetRatioUnsafe(float aspectRatio);

    void SetRatio(float aspectRatio);

    /// panel.Refresh invalidates area to be painted later
    /// panel.Update triggers a repaint now, on any invalidated area
    /// repeatedly called by RenderLoop
    void OnPanelPaint(wxPaintEvent& evt);
    void OnPanelShow(bool isShown) { isVisible = isShown; }

    std::unique_ptr<RenderLoop> renderLoop;

    /// parent of previewPanel, owns memory
    RefPtr<wxWindow> parent;
    /// reserve ids to link events for panel
    wxWindowIDRef panelID{};
    /// panel with paint event, gets drawn on
    OptRefPtr<wxPanel> panel{};

    /// is the panel visible in some form
    bool isVisible = false;

    /// image set externally, swapped when other is needed
    cv::Mat writeImage{};
    /// usually when image will get swapped
    bool writeImageUpdated = false;
    /// image used internally and displayed
    cv::Mat readImage{};
    /// either a reference of or a buffer that the readImage will be copied to
    /// translating the OpenCV image to something wxWidgets can use
    wxBitmap readImageBitmap{};

    /// protect swapping writeImage and readImage
    std::mutex imageSwapMutex{};
};

class PreviewFrame
{
public:
    PreviewFrame(const wxString& _title)
        : title(_title), frame(nullptr, FrameDeleter) {}

    void Create();
    void CreateClosable();
    void Destroy();

    void UpdateImage(const cv::Mat& newImage)
    {
        previewPane.UpdateImage(newImage);
    }

    bool IsVisible() const { return isVisible; }

private:
    bool IsVisibleUnsafe() const
    {
        ATT_ASSERT(utils::IsMainThread());
        ATT_ASSERT(IsVisible() == static_cast<bool>(frame));
        return static_cast<bool>(frame);
    }

    void CreateFrame(wxPoint pos, wxSize size, int style);

    wxString title;
    FramePtr frame;

    wxWindowIDRef frameID{};
    PreviewPane previewPane{};

    /// sync with frame not-null
    bool isVisible = false;
};
