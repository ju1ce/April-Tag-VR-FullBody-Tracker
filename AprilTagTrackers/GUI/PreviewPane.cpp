#include "PreviewPane.hpp"

#include "math/CVHelpers.hpp"
#include "MatToBitmap.hpp"
#include "utils/Assert.hpp"
#include "wxHelpers.hpp"

std::unique_ptr<wxBoxSizer> PreviewPane::Create(RefPtr<wxWindow> _parent, wxSize size)
{
    ATT_ASSERT(utils::IsMainThread());
    ATT_ASSERT(panel.IsNull());
    parent = _parent;
    panelID = wxWindow::NewControlId();
    panel = NewWindow<wxPanel>(parent, panelID, wxDefaultPosition, size);
    panel->SetBackgroundStyle(wxBackgroundStyle::wxBG_STYLE_PAINT);

    auto widthSizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    widthSizer->AddStretchSpacer();
    widthSizer->Add(panel, wxSizerFlags().Expand().Shaped());
    widthSizer->AddStretchSpacer();

    panel->Bind(
        wxEVT_PAINT, [this](auto& evt)
        { this->OnPanelPaint(evt); },
        panelID);
    panel->Bind(
        wxEVT_SHOW, [this](wxShowEvent& evt)
        { this->OnPanelShow(evt.IsShown()); });
    Show();
    return std::move(widthSizer);
}

void PreviewPane::Destroy()
{
    ATT_ASSERT(utils::IsMainThread());
    ATT_ASSERT(panel.NotNull());
    isVisible = false;
    renderLoop->StopLoop();
    panel.SetNull();
    panelID = wxID_ANY;

    std::lock_guard lock{imageSwapMutex};
    readImage.release();
    writeImage.release();
}

void PreviewPane::Show()
{
    ATT_ASSERT(utils::IsMainThread());
    if (panel.IsNull()) return;
    panel->Show();
    isVisible = true;
    renderLoop->StartLoop(RENDER_UPS);
}

void PreviewPane::Hide()
{
    ATT_ASSERT(utils::IsMainThread());
    if (panel.IsNull()) return;
    isVisible = false;
    panel->Hide();
    renderLoop->StopLoop();
}

void PreviewPane::UpdateImage(const cv::Mat& newImage)
{
    ATT_ASSERT(!newImage.empty());
    if (!IsVisible()) return;

    { // lock scope
        const std::lock_guard lock{imageSwapMutex};
        newImage.copyTo(writeImage);
        writeImageUpdated = true;
    }
    UpdateRatioIfChanged();
}

void PreviewPane::UpdateImage(const cv::Mat& newImage, int constrainSize)
{
    ATT_ASSERT(!newImage.empty());
    ATT_ASSERT(constrainSize > 0);
    if (!IsVisible()) return;

    { // lock scope
        const std::lock_guard lock{imageSwapMutex};
        const cv::Size2i size = math::ConstrainSize(GetMatSize(newImage), constrainSize);
        cv::resize(newImage, writeImage, size);
        writeImageUpdated = true;
    }
    UpdateRatioIfChanged();
}

void PreviewPane::ClearBackground(RefPtr<wxGraphicsContext> context, wxSize drawArea)
{
    if (backgroundBrush.IsNull())
    {
        backgroundBrush = context->CreateBrush(*wxGREY_BRUSH);
    }
    context->SetBrush(backgroundBrush);
    context->DrawRectangle(0, 0, drawArea.x, drawArea.y);
}

void PreviewPane::Repaint()
{
    ATT_ASSERT(panel.NotNull());
    constexpr bool eraseBackground = false;
    panel->Refresh(eraseBackground);
}

bool PreviewPane::SwapReadWriteImage()
{
    /// TODO: test writeImageUpdated before this? its a race condition with SetImage but,
    /// does that matter? at most it would miss a single frame.
    std::lock_guard lock{imageSwapMutex};
    if (writeImageUpdated)
    {
        cv::swap(writeImage, readImage);
        writeImageUpdated = false;
        return true;
    }
    return false;
}

void PreviewPane::SetRatioUnsafe(float aspectRatio)
{
    ATT_ASSERT(panel.NotNull());
    ATT_ASSERT(utils::IsMainThread());
    ATT_ASSERT(aspectRatio > 0);

    if (OptRefPtr<wxSizer> sizer = panel->GetContainingSizer())
    {
        if (OptRefPtr<wxSizerItem> item = sizer->GetItem(panel.Get()))
        {
            item->SetRatio(aspectRatio);
        }
    }
    panel->SendSizeEventToParent();
}

void PreviewPane::UpdateRatioIfChanged()
{
    ATT_ASSERT(panel.NotNull());
    if (GetMatSize(writeImage) != GetMatSize(readImage))
    {
        const float aspectRatio = static_cast<float>(writeImage.cols) / static_cast<float>(writeImage.rows);
        panel->CallAfter([this, aspectRatio]
                         { this->SetRatioUnsafe(aspectRatio); });
    }
}

void PreviewPane::OnPanelPaint(wxPaintEvent&)
{
    ATT_ASSERT(panel.NotNull());
    const wxPaintDC dc{panel};
    /// uses faster platform specific drawing
    const std::unique_ptr<wxGraphicsContext> gc{wxGraphicsContext::Create(dc)};
    gc->SetInterpolationQuality(wxINTERPOLATION_FAST);
    gc->SetCompositionMode(wxCOMPOSITION_SOURCE);
    gc->SetAntialiasMode(wxANTIALIAS_NONE);

    const wxSize drawArea = panel->GetSize();
    if (drawArea.x < MIN_DRAW_SIZE || drawArea.y < MIN_DRAW_SIZE)
    {
        ClearBackground(gc, drawArea);
        return;
    }

    bool rebuildBitmap = SwapReadWriteImage();

    if (readImage.empty())
    {
        ClearBackground(gc, drawArea);
        return;
    }

    if (rebuildBitmap)
    {
        ReserveBitmapForMat(readImage, readImageBitmap);
        ConvertMatToBitmap(readImage, readImageBitmap);
    }

    gc->DrawBitmap(readImageBitmap, 0, 0, drawArea.x, drawArea.y);
}

void PreviewFrame::Create()
{
    constexpr int style = wxDEFAULT_FRAME_STYLE & (~wxCLOSE_BOX);
    CreateFrame(wxDefaultPosition, wxDefaultSize, style);
}

void PreviewFrame::CreateClosable()
{
    CreateFrame(wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE);
}

void PreviewFrame::Destroy()
{
    ATT_ASSERT(utils::IsMainThread());
    isVisible = false;
    previewPane.Destroy();
    frame.reset();
}

void PreviewFrame::CreateFrame(wxPoint pos, wxSize size, int style)
{
    ATT_ASSERT(utils::IsMainThread());
    if (IsVisibleUnsafe())
    {
        // expand if minimized
        frame->Iconize(false);
        // bring to front
        frame->Raise();
        return;
    }
    frameID = wxWindow::NewControlId();
    frame.reset(new wxFrame(nullptr, frameID, title, pos, size, style));
    isVisible = true;

    std::unique_ptr<wxBoxSizer> paneSizer = previewPane.Create(frame.get(), wxSize(640, 480));
    frame->SetSizer(paneSizer.release());

    frame->Bind(
        wxEVT_CLOSE_WINDOW, [this](auto&)
        { this->Destroy(); },
        frameID);

    previewPane.Show();
    frame->Fit();
    frame->Show();
}
