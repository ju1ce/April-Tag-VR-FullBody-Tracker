#pragma once

#include "Debug.h"
#include "wxHelpers.h"

#include <wx/arrstr.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/window.h>

#include <algorithm>
#include <sstream>
#include <stack>
#include <string>
#include <type_traits>
#include <vector>

namespace Form
{

/// Form element
struct IElement
{
    virtual ~IElement() = default;

    /// Create the element in the parent and sizer, only called by FormBuilder::Add.
    virtual void Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags = {}) = 0;
    /// Submit the form, usually means parsing an input then setting the referenced backing variable.
    virtual void Submit() {}
    /// Apply some update to the form, like showing changed backing values.
    virtual void Update() {}

    virtual void SetVisible(bool = true) {}
    virtual void SetEnabled(bool = true) {}
};

/// Build a form.
class FormBuilder : public IElement
{
public:
    /// Must initialize later with contructor providing parent and sizer.
    FormBuilder() = default;
    FormBuilder(FormBuilder&&) = default;
    FormBuilder& operator=(FormBuilder&&) = default;

    /// Parents widgets to parent, and adds to sizer.
    FormBuilder(RefPtr<wxWindow> _parent, RefPtr<wxSizer> sizer)
        : parent(std::move(_parent))
    {
        sizerStack.push(std::move(sizer));
    }

    // Forwards to all interfaces in elements list.
    void Submit() override;
    void Update() override;
    void SetVisible(bool visible = true) override;
    void SetEnabled(bool enabled = true) override;
    /// Calls ShowItems() on the top sizer
    void SetSizerVisible(bool visible = true);

    /// Set sizer flag state.
    FormBuilder& Flags(wxSizerFlags _sizerFlags = {});
    /// Update sizer flag state with border.
    FormBuilder& Border(int borderFlag = wxALL, int borderInPixels = wxSizerFlags::GetDefaultBorder());

    template <typename SizerT, typename... Args>
    FormBuilder& PushSizer(Args&&... sizerArgs);
    /// Special sizer because it changes the parent window.
    FormBuilder& PushStaticBoxSizer(const wxString& label, int orient = wxVERTICAL);
    /// Set sizer to the parent of the current sizer.
    /// If current is static box sizer,
    /// will set parent window to parent of static box.
    FormBuilder& PopSizer();
    /// Get the top of the sizer stack.
    RefPtr<wxSizer> GetSizer();
    /// Get the current parent.
    RefPtr<wxWindow> GetParent() { return parent; }

    /// Add an IElement and return it.
    /// Moves the element to a list of unique_ptr elements.
    /// Use sizerFlags for this element instead of this instance's.
    template <typename ElemT>
    RefPtr<ElemT> AddGet(ElemT&& elem, wxSizerFlags flags);
    /// Use this instances sizerFlags.
    template <typename ElemT>
    RefPtr<ElemT> AddGet(ElemT&& elem);
    /// Add an IElement.
    /// Moves the element to a list of unique_ptr elements.
    /// Use sizerFlags for this element instead of this instance's.
    template <typename ElemT>
    FormBuilder& Add(ElemT&& elem, wxSizerFlags flags);
    /// Use this instances sizerFlags.
    template <typename ElemT>
    FormBuilder& Add(ElemT&& elem);

    /// Create a sub form with the current parent, sizer, and flags.
    /// Memory is owned by this form
    RefPtr<FormBuilder> SubForm();

private:
    /// FormBuilder is a special element, create should do nothing when called as a sub form.
    void Create(RefPtr<wxWindow>, RefPtr<wxSizer>, wxSizerFlags = {}) override {}

    /// window used when adding elements.
    RefPtr<wxWindow> parent;
    /// List of sub sizers, wx doesn't store containing sizer for sizers.
    std::stack<RefPtr<wxSizer>> sizerStack;
    /// sizer flag state used when adding elements.
    wxSizerFlags sizerFlags;

    std::vector<std::unique_ptr<IElement>> elements;
};

/// Some wxWidgets control.
template <typename WinT>
class Widget : public IElement
{
public:
    static_assert(std::is_base_of_v<wxControl, WinT>, "WinT is derived from wxControl.");

    void SetVisible(bool visible = true) override { GetWidget()->Show(visible); }
    void SetEnabled(bool enabled = true) override { GetWidget()->Enable(enabled); }

    template <typename EventTag, typename F>
    void Bind(const EventTag& eventType, const F& func);

    RefPtr<WinT> GetWidget() const { return widget; }

protected:
    template <typename... Args>
    RefPtr<WinT> CreateWidget(RefPtr<wxWindow> parent, Args&&... args);

private:
    RefPtr<WinT> widget;
    wxWindowID id{};
};

/// Text box that sets a backed value with FromString/ToString.
template <typename T>
class InputText : public Widget<wxTextCtrl>
{
public:
    InputText(T& _backingValue) : backingValue(_backingValue) {}

    void Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags = {}) override;
    void Submit() override;

private:
    T& backingValue;
};

/// Advanced number input with multiple step buttons and scroll wheel input.
template <typename T>
class InputNumber : public Widget<wxTextCtrl>
{
private:
    auto CreateAdjustBackingFunc(int amount);

public:
    InputNumber(T& _backingValue) : backingValue(_backingValue) {}

    void Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags = {}) override;
    void Update() override;

private:
    T& backingValue;
    std::vector<RefPtr<wxWindow>> buttons;
};

/// CheckBox backed by bool.
class CheckBox : public Widget<wxCheckBox>
{
public:
    CheckBox(bool& _backingValue)
        : backingValue(_backingValue) {}

    void Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags = {}) override;
    void Submit() override;

private:
    bool& backingValue;
};

/// CheckBox that is bound to a onChange function.
/// F is a lambda of type: void (bool isChecked).
template <typename F>
class CheckBoxButton : public Widget<wxCheckBox>
{
public:
    CheckBoxButton(const wxString& _label, F&& _onChange)
        : label(_label), onChange(_onChange) {}

    void Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags = {}) override;

private:
    wxString label;
    const F& onChange;
};

/// Label with optional tooltip.
class Label : public Widget<wxStaticText>
{
public:
    Label(const wxString& _label, const wxString& _toolTip = "")
        : label(_label), toolTip(_toolTip) {}

    void Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags = {}) override;

private:
    wxString label;
    wxString toolTip;
};

/// Dropdown of selection options, sets backing int to index in options.
template <typename T>
class Choice : public Widget<wxChoice>
{
public:
    template <typename StrList, typename TList>
    Choice(T& _backingValue, const StrList& _options, const TList& _mappings);
    /// Backing value will be mapped to index of option
    template <typename StrList>
    Choice(T& _backingValue, const StrList& _options);

    void Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags = {}) override;
    void Submit() override;

private:
    T& backingValue;
    wxArrayString options;
    std::vector<T> mappings;
};

/// F is a lambda of type: void (wxCommandEvent&).
template <typename F>
class Button : public Widget<wxButton>
{
public:
    Button(const wxString& _label, const F& _onPress)
        : label(_label), onPress(_onPress)
    {
    }

    void Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags = {}) override;

private:
    wxString label;
    const F& onPress;
};

/// Calls sizer->AddStretchSpacer.
class StretchSpacer : public IElement
{
public:
    StretchSpacer(int _proportion = 1) : proportion(_proportion) {}

    void Create(RefPtr<wxWindow>, RefPtr<wxSizer> sizer, wxSizerFlags = {}) override;

private:
    int proportion;
};

/// Label text and tooltip above or next to an element.
template <typename T>
class Labeled : public IElement
{
public:
    static_assert(std::is_base_of_v<IElement, T>, "T is derived from IElement.");

    Labeled(int _orient, const wxString& _label, const wxString& _toolTip, T&& _elem = {})
        : label(_label, _toolTip), orient(_orient), elem(std::move(_elem)) {}

    Labeled(const wxString& _label, const wxString& _toolTip, T&& _elem = {})
        : label(_label, _toolTip), elem(std::move(_elem)) {}

    Labeled(const wxString& _label, T&& _elem = {})
        : label(_label), elem(std::move(_elem)) {}

    /// Places label next to elem inside box sizer.
    void Create(RefPtr<wxWindow> parent, RefPtr<wxSizer> sizer, wxSizerFlags flags = {}) override;

    void Submit() override { elem.Submit(); }
    void Update() override { elem.Update(); }
    void SetVisible(bool visible = true) override;
    void SetEnabled(bool enabled = true) override;

    T& GetElem() { return elem; }

private:
    Label label;
    T elem;

    int orient = wxVERTICAL;
};

} // namespace Form

#include "FormBuilder.tpp" // NOLINT: Template implementation
