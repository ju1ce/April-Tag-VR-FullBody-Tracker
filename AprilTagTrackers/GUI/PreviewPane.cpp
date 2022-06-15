#include "PreviewPane.h"

#include "Debug.h"
#include "wxHelpers.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <wx/dcclient.h>
#include <wx/image.h>
#include <wx/rawbmp.h>
#include <wx/sizer.h>

PreviewPane::PreviewPane(RefPtr<wxWindow> parent, wxWindowID _id)
    : wxPanel(parent, _id, wxDefaultPosition, wxSize(640, 480)), id(_id),
      renderLoop(std::make_unique<PreviewRenderLoop>(*this))
{
    Hide();
    SetBackgroundStyle(wxBackgroundStyle::wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &PreviewPane::OnPaintEvent, this, id);
}

void PreviewPane::OnPaintEvent(wxPaintEvent&)
{
    wxPaintDC dc(this);
    // Uses a more efficient backend for drawing
    const auto gc = std::unique_ptr<wxGraphicsContext>(wxGraphicsContext::Create(dc));
    gc->SetInterpolationQuality(wxINTERPOLATION_FAST);
    gc->SetCompositionMode(wxCOMPOSITION_SOURCE);
    gc->SetAntialiasMode(wxANTIALIAS_NONE);

    bool rebuildBitmap = false;
    {
        const auto lock = std::lock_guard(imageMutex);
        if (newImageReady && !image.empty())
        {
            newImageReady = false;
            rebuildBitmap = true;
            cv::swap(image, swapImage);
        }
    }

    wxSize drawArea = GetSize();
    if (swapImage.empty() || drawArea.x < 10 || drawArea.y < 10)
    {
        if (backgroundBrush.IsNull())
        {
            backgroundBrush = gc->CreateBrush(*wxGREY_BRUSH);
        }
        gc->SetBrush(backgroundBrush);
        gc->DrawRectangle(0, 0, drawArea.x, drawArea.y);
        return;
    }

    if (rebuildBitmap || bitmap.IsNull())
    {
        // Sets up a reference to the pixel buffer that wxBitmap can use
        constexpr bool isStaticData = true;
        // expects data to be RGB 8U
        wxImage imageRef(swapImage.cols, swapImage.rows, swapImage.data, isStaticData);
        // copies the wxImage data to some native format
        bitmap = gc->CreateBitmapFromImage(imageRef);
    }

    gc->DrawBitmap(bitmap, 0, 0, drawArea.x, drawArea.y);
}

void PreviewPane::UpdateImage(const cv::Mat& newImage)
{
    bool sizeChanged = false;
    {
        const auto lock = std::lock_guard(imageMutex);
        sizeChanged = image.cols != newImage.cols || image.rows != newImage.rows;
        // OpenCV always outputs in BGR while wxImage only accepts RGB
        cv::cvtColor(newImage, image, cv::COLOR_BGR2RGB);
        newImageReady = true;

        ATASSERT("Matrix points to continuous memory.", image.isContinuous());
        ATASSERT("Matrix is a non-empty or null, 2d image.",
            !image.empty() && image.rows > 1 && image.cols > 1);
    }
    if (sizeChanged)
    {
        CallAfter([this, x = image.cols, y = image.rows]
            {
                // Update wxSHAPED sizing
                OptRefPtr<wxSizerItem> item = GetContainingSizer()->GetItem(this);
                if (item.NotNull())
                {
                    item->SetRatio(x, y);
                }
                SendSizeEventToParent();
            });
    }
}

void PreviewPane::ShowPane()
{
    Show();
    renderLoop->StartLoop(RENDER_UPS);
}

void PreviewPane::HidePane()
{
    Hide();
    renderLoop->StopLoop();
}

PreviewRenderLoop::PreviewRenderLoop(PreviewPane& _pane)
    : wxTimer(), pane(_pane)
{
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
}

PreviewFrame::PreviewFrame(const wxString& _title)
    : title(_title), id(wxWindow::NewControlId()), frame(nullptr, FrameDestroyer)
{
}

void PreviewFrame::UpdateImage(const cv::Mat& image)
{
    const auto lock = std::lock_guard{frameLock};
    if (!frame) return;
    pane->UpdateImage(image);
}

void PreviewFrame::Show(CloseButton closeButton)
{
    const auto lock = std::lock_guard{frameLock};
    // already visible
    if (frame) return;

    int style = wxDEFAULT_FRAME_STYLE;
    if (closeButton == CloseButton::Hide)
        style &= ~wxCLOSE_BOX;
    frame.reset(new wxFrame(nullptr, id, title, wxDefaultPosition, wxDefaultSize, style));

    pane = NewWindow<PreviewPane>(frame.get(), wxWindow::NewControlId());
    pane->ShowPane();

    auto horiz = NewSizer<wxBoxSizer>(frame.get(), wxHORIZONTAL);

    horiz->AddStretchSpacer();
    horiz->Add(pane, wxSizerFlags().Expand().Shaped());
    horiz->AddStretchSpacer();

    frame->Fit();
    frame->Show();

    frame->Bind(
        wxEVT_CLOSE_WINDOW, [this](auto&)
        {
            Hide();
        },
        id);
}

void PreviewFrame::Hide()
{
    const auto lock = std::lock_guard{frameLock};
    // already destroyed
    if (!frame) return;
    frame.reset();
}

bool PreviewFrame::IsVisible()
{
    const auto lock = std::lock_guard{frameLock};
    return static_cast<bool>(frame);
}
