#include "PreviewPane.h"

#include "Debug.h"
#include "wxHelpers.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <wx/dcclient.h>
#include <wx/graphics.h>
#include <wx/image.h>
#include <wx/rawbmp.h>
#include <wx/sizer.h>

PreviewPane::PreviewPane(RefPtr<wxWindow> parent, wxWindowID _id)
    : wxPanel(parent, _id, wxDefaultPosition, wxSize(360, 640)), id(_id),
      renderLoop(std::make_unique<PreviewRenderLoop>(*this))
{
    Hide();
    SetBackgroundStyle(wxBackgroundStyle::wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &PreviewPane::OnPaintEvent, this, id);
}

void PreviewPane::OnPaintEvent(wxPaintEvent& evt)
{
    {
        const auto lock = std::lock_guard(imageMutex);
        if (!newImageReady || image.empty()) return;
        newImageReady = false;
        cv::swap(image, swapImage);
    }
    wxSize drawArea = GetSize();
    if (drawArea.x < 10 || drawArea.y < 10) return;
    // Sets up a reference to the pixel buffer that wxBitmap can use
    constexpr bool isStaticData = true;
    // expects data to be RGB 8U
    wxImage imageRef(swapImage.cols, swapImage.rows, swapImage.data, isStaticData);
    wxPaintDC dc(this);
    // Uses a more efficient backend for drawing
    const auto gc = std::unique_ptr<wxGraphicsContext>(wxGraphicsContext::Create(dc));
    // This copies the wxImage data to some native format
    auto bitmap = gc->CreateBitmapFromImage(imageRef);

    // gc->SetInterpolationQuality(wxInterpolationQuality::wxINTERPOLATION_FAST);
    gc->DrawBitmap(bitmap, 0, 0, drawArea.x, drawArea.y);

    evt.Skip();
}

void PreviewPane::UpdateImage(const cv::Mat& newImage)
{
    const auto lock = std::lock_guard(imageMutex);
    bool sizeChanged = image.cols != newImage.cols || image.rows != newImage.rows;
    // OpenCV always outputs in BGR while wxImage only accepts RGB
    cv::cvtColor(newImage, image, cv::COLOR_BGR2RGB);
    newImageReady = true;

    ATASSERT("Matrix points to continuous memory.", image.isContinuous());
    ATASSERT("Matrix is a non-empty or null, 2d image.",
        !image.empty() && image.rows > 1 && image.cols > 1);

    if (sizeChanged)
    {
        CallAfter([this, x = image.cols, y = image.rows]
            {
                SetMinSize(wxSize(x, y));
            });
    }
}

void PreviewPane::SetVisible(bool visible)
{
    Show(visible);
    if (visible)
    {
        renderLoop->StartLoop(RENDER_UPS);
    }
    else
    {
        renderLoop->StopLoop();
    }
}

PreviewRenderLoop::PreviewRenderLoop(PreviewPane& _pane, int updatesPerSecond)
    : wxTimer(), pane(_pane)
{
    if (updatesPerSecond > 0)
        Start(updatesPerSecond);
}

void PreviewRenderLoop::StartLoop(int updatesPerSecond)
{
    if (updatesPerSecond > 0)
    {
        const int millis = 1000 / updatesPerSecond;
        wxTimer::Start(millis);
    }
    else if (updatesPerSecond == -1)
    {
        wxTimer::Start();
    }
}

void PreviewRenderLoop::Notify()
{
    pane.Refresh(false);
    pane.Update();
}

PreviewFrame::PreviewFrame(const wxString& _title)
    : title(_title), id(wxWindow::NewControlId())
{
}

PreviewFrame::~PreviewFrame()
{
    SetVisible(false);
}

void PreviewFrame::UpdateImage(const cv::Mat& image)
{
    const auto lock = std::lock_guard{frameLock};
    if (!frame.has_value()) return;
    pane->UpdateImage(image);
}

void PreviewFrame::SetVisible(bool visible, bool userCanDestroy)
{
    const auto lock = std::lock_guard{frameLock};
    if (visible)
    {
        // already visible
        if (frame.has_value()) return;

        int style = wxDEFAULT_FRAME_STYLE;
        if (!userCanDestroy) style &= ~wxCLOSE_BOX;
        /// Frees itself in the close window event
        RefPtr<wxFrame> newFrame = new wxFrame(nullptr, id, title, wxDefaultPosition, wxDefaultSize, style);

        pane = NewWindow<PreviewPane>(newFrame, wxWindow::NewControlId());
        newFrame->SetClientSize(pane->GetSize());
        pane->SetVisible(true);

        auto horiz = NewSizer<wxBoxSizer>(newFrame, wxHORIZONTAL);

        horiz->AddStretchSpacer();
        horiz->Add(pane, wxSizerFlags().Expand().Shaped());
        horiz->AddStretchSpacer();

        newFrame->Show();

        newFrame->Bind(
            wxEVT_CLOSE_WINDOW, [this](auto&)
            {
                SetVisible(false);
            },
            id);

        frame = newFrame;
    }
    else if (frame.has_value())
    {
        frame.value()->Destroy();
        frame = std::nullopt;
    }
}

bool PreviewFrame::IsVisible()
{
    const auto lock = std::lock_guard{frameLock};
    return frame.has_value();
}
