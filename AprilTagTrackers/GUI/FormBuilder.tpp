#pragma once

#include "FormBuilder.hpp"

namespace Form
{

template <typename SizerT, typename... Args>
inline FormBuilder& FormBuilder::PushSizer(Args&&... sizerArgs)
{
    sizerStack.push(NewSizer<SizerT>(GetSizer(), sizerFlags, std::forward<Args>(sizerArgs)...));
    return *this;
}

template <typename ElemT>
inline RefPtr<ElemT> FormBuilder::AddGet(ElemT&& elem, wxSizerFlags flags)
{
    static_assert(std::is_base_of_v<IElement, ElemT>, "ElemT is derived from IElement.");
    static_assert(!std::is_same_v<FormBuilder, ElemT>, "Use FormBuilder::SubForm.");

    // Owns until the emplace_back, still need reference to derived ElemT.
    auto* elemPtr = new ElemT(std::forward<ElemT>(elem));
    elements.emplace_back(static_cast<IElement*>(elemPtr));

    elemPtr->Create(parent, GetSizer(), flags);
    return elemPtr;
}

template <typename ElemT>
inline RefPtr<ElemT> FormBuilder::AddGet(ElemT&& elem)
{
    return AddGet(std::forward<ElemT>(elem), sizerFlags);
}

template <typename ElemT>
inline FormBuilder& FormBuilder::Add(ElemT&& elem, wxSizerFlags flags)
{
    AddGet(std::forward<ElemT>(elem), flags);
    return *this;
}

template <typename ElemT>
inline FormBuilder& FormBuilder::Add(ElemT&& elem)
{
    AddGet(std::forward<ElemT>(elem), sizerFlags);
    return *this;
}

template <typename WinT>
template <typename EventTag, typename F>
inline void Widget<WinT>::Bind(const EventTag& eventType, const F& func)
{
    GetWidget()->Bind(eventType, func, id);
}

template <typename WinT>
template <typename... Args>
inline RefPtr<WinT> Widget<WinT>::CreateWidget(RefPtr<wxWindow> parent, Args&&... args)
{
    id = wxWindow::NewControlId();
    widget = NewWindow<WinT>(parent, id, std::forward<Args>(args)...);
    return widget;
}

template <typename T>
inline void InputText<T>::Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags)
{
    CreateWidget(parent, ToWXString(backingValue));
    sizer->Add(GetWidget(), flags);
}

template <typename T>
inline void InputText<T>::Submit()
{
    FromWXString(GetWidget()->GetValue(), backingValue);
    // ChangeValue will not trigger the text event
    GetWidget()->ChangeValue(ToWXString(backingValue));
}

template <typename T>
void InputNumber<T>::Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags)
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
            val = static_cast<double>(val)+(evt.GetWheelRotation() > 0 ? 1 : -1);
            ctrl->ChangeValue(ToWXString(val));
        });

    Bind(wxEVT_TEXT,
        [&val = backingValue, ctrl = GetWidget()](wxCommandEvent&)
        {
            FromWXString(ctrl->GetValue(), val);
        });
}

template <typename T>
void InputNumber<T>::Update()
{
    GetWidget()->ChangeValue(ToWXString(backingValue));
}


template <typename T>
inline auto InputNumber<T>::CreateAdjustBackingFunc(int amount)
{
    return [&val = backingValue, ctrl = GetWidget(), amount](wxCommandEvent&)
    {
        val = static_cast<double>(val)+amount;
        ctrl->ChangeValue(ToWXString(val));
    };
}

template <typename F>
inline void CheckBoxButton<F>::Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags)
{
    CreateWidget(parent, label);
    sizer->Add(GetWidget(), flags);
    Bind(wxEVT_CHECKBOX, onChange);
}

template <typename T>
template <typename StrList>
inline Choice<T>::Choice(T& _backingValue, const StrList& _options)
    : backingValue(_backingValue), mappings()
{
    int index = 0;
    options.Alloc(_options.size());
    mappings.reserve(_options.size());
    for (const auto& str : _options)
    {
        options.Add(str);
        mappings.emplace_back(index++);
    }

    ATASSERT("Options is not empty.",
        !options.empty());
}

template <typename T>
template <typename StrList, typename TList>
inline Choice<T>::Choice(T& _backingValue, const StrList& _options, const TList& _mappings)
    : backingValue(_backingValue), mappings()
{
    options.Alloc(_options.size());
    for (const auto& str : _options)
    {
        options.Add(str);
    }
    mappings.reserve(_mappings.size());
    for (const auto& elem : _mappings)
    {
        mappings.emplace_back(elem);
    }

    ATASSERT("Options list and mappings list are the same size.",
        options.size() == mappings.size());
    ATASSERT("Lists are not empty.",
        !options.empty() && !mappings.empty());
}

template <typename T>
inline void Choice<T>::Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags)
{
    CreateWidget(parent, wxDefaultPosition, wxDefaultSize, options);
    sizer->Add(GetWidget(), flags);

    auto mappingIt = std::find(mappings.begin(), mappings.end(), backingValue);

    if (mappingIt != mappings.end())
    {
        GetWidget()->SetSelection(std::distance(mappings.begin(), mappingIt));
    }
    else
    {
        // backing value set to some incorrect value, pick the first option,
        // when submit is called, it will be updated.
        GetWidget()->SetSelection(0);
    }
}

template <typename T>
inline void Choice<T>::Submit()
{
    int mappedIndex = GetWidget()->GetSelection();
    backingValue = mappings[mappedIndex];
}

template <typename F>
inline void Button<F>::Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags)
{
    CreateWidget(parent, label);
    sizer->Add(GetWidget(), flags);
    Bind(wxEVT_BUTTON, onPress);
}

template <typename ElemT>
inline void Labeled<ElemT>::Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags)
{
    auto subSizer = NewSizer<wxBoxSizer>(sizer, flags, orient);
    label.Create(parent, subSizer, wxSizerFlags().Border(wxBOTTOM | wxRIGHT, 5));
    elem.Create(parent, subSizer, {});
    // Forward label tooltip to the element
    wxString toolTip = label.GetWidget()->GetToolTipText();
    if (!toolTip.empty())
        elem.GetWidget()->SetToolTip(toolTip);
}

template <typename ElemT>
inline void Labeled<ElemT>::SetVisible(bool visible)
{
    label.SetVisible(visible);
    elem.SetVisible(visible);
}

template <typename ElemT>
inline void Labeled<ElemT>::SetEnabled(bool enabled)
{
    label.SetEnabled(enabled);
    elem.SetEnabled(enabled);
}

} // namespace Form
