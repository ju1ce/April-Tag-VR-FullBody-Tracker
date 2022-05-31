#include "FormBuilder.h"

namespace Form
{

void FormBuilder::Submit()
{
    for (auto& e : elements) e->Submit();
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
    if (staticSizer)
    {
        ATASSERT("wxStaticBoxSizer requires the parent to be its static box.",
            parent.DynamicCast<wxStaticBox>().has_value());
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

void InputNumber::Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags)
{
    wxSize buttonSize{25, 25};
    auto subSizer = NewSizer<wxBoxSizer>(sizer, flags, wxHORIZONTAL);
    CreateWidget(parent, ToWXString(backingValue));

    subSizer->Add(NewButton(parent, "<<", buttonSize, CreateAdjustBackingFunc(-10)));
    subSizer->Add(NewButton(parent, "<", buttonSize, CreateAdjustBackingFunc(-1)));
    subSizer->Add(GetWidget());
    subSizer->Add(NewButton(parent, ">", buttonSize, CreateAdjustBackingFunc(1)));
    subSizer->Add(NewButton(parent, ">>", buttonSize, CreateAdjustBackingFunc(10)));

    Bind(wxEVT_MOUSEWHEEL,
        [&val = backingValue, ctrl = GetWidget()](wxMouseEvent& evt)
        {
            val += evt.GetWheelRotation() > 0 ? 1 : -1;
            ctrl->ChangeValue(ToWXString(val));
        });

    Bind(wxEVT_TEXT,
        [&val = backingValue, ctrl = GetWidget()](wxCommandEvent&)
        {
            FromWXString(ctrl->GetValue(), val);
        });
}

void InputNumber::Update()
{
    GetWidget()->ChangeValue(ToWXString(backingValue));
}

} // namespace Form
