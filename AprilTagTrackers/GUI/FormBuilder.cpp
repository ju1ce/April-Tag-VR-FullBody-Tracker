#include "FormBuilder.hpp"

namespace Form
{

void FormBuilder::Submit()
{
    for (auto& e : elements) e->Submit();
}

void FormBuilder::Update()
{
    for (auto& e : elements) e->Update();
}

void FormBuilder::SetVisible(bool visible)
{
    for (auto& e : elements) e->SetVisible(visible);
}

void FormBuilder::SetEnabled(bool enabled)
{
    for (auto& e : elements) e->SetEnabled(enabled);
}

void FormBuilder::SetSizerVisible(bool visible)
{
    GetSizer()->ShowItems(visible);
    GetParent()->Layout();
}

FormBuilder& FormBuilder::Flags(wxSizerFlags _sizerFlags)
{
    sizerFlags = _sizerFlags;
    return *this;
}

FormBuilder& FormBuilder::Border(int borderFlag, int borderInPixels)
{
    sizerFlags = sizerFlags.Border(borderFlag, borderInPixels);
    return *this;
}

FormBuilder& FormBuilder::PushStaticBoxSizer(const wxString& label, int orient)
{
    auto staticSizer = NewSizer<wxStaticBoxSizer>(label, parent, GetSizer(), orient, sizerFlags);
    parent = staticSizer->GetStaticBox();
    sizerStack.push(staticSizer);
    return *this;
}

FormBuilder& FormBuilder::PopSizer()
{
    auto staticSizer = GetSizer().DynamicCast<wxStaticBoxSizer>();
    if (staticSizer.NotNull())
    {
        ATASSERT("wxStaticBoxSizer requires the parent to be its static box.",
            parent.DynamicCast<wxStaticBox>().NotNull());
        parent = parent->GetParent();
    }
    sizerStack.pop();
    ATASSERT("Sizer stack always has the initial sizer.",
        !sizerStack.empty());
    return *this;
}

RefPtr<wxSizer> FormBuilder::GetSizer()
{
    ATASSERT("Sizer stack always has the initial sizer.",
        !sizerStack.empty());
    return sizerStack.top();
}

RefPtr<FormBuilder> FormBuilder::SubForm()
{
    auto* elemPtr = new FormBuilder(parent, GetSizer());
    elements.emplace_back(static_cast<IElement*>(elemPtr));

    elemPtr->Flags(sizerFlags);
    return elemPtr;
}

void Label::Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags)
{
    CreateWidget(parent, label);
    if (!toolTip.empty()) GetWidget()->SetToolTip(toolTip);
    sizer->Add(GetWidget(), flags);
}

void CheckBox::Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags)
{
    CreateWidget(parent, "");
    GetWidget()->SetValue(backingValue);
    sizer->Add(GetWidget(), flags);
}

void CheckBox::Submit()
{
    backingValue = GetWidget()->IsChecked();
}

void StretchSpacer::Create(RefPtr<wxWindow>, RefPtr<wxSizer> sizer, wxSizerFlags)
{
    sizer->AddStretchSpacer(proportion);
}
} // namespace Form
