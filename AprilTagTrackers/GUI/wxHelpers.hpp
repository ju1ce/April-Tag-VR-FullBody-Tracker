#pragma once

#include "RefPtr.hpp"
#include "Serializable.hpp"
#include "utils/Assert.hpp"

#include <wx/button.h>
#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/window.h>

#include <memory>
#include <sstream>
#include <type_traits>

constexpr auto FrameDeleter = [](wxFrame* frame)
{
    frame->Destroy();
};
using FramePtr = std::unique_ptr<wxFrame, decltype(FrameDeleter)>;

template <typename... Args>
inline FramePtr NewFrame(Args&&... args)
{
    return FramePtr(new wxFrame(nullptr, std::forward<Args>(args)...), FrameDeleter);
}

/// Create T and return non-owning pointer, parent now owns the memory.
template <typename T, typename... Args>
inline RefPtr<T> NewWindow(RefPtr<wxWindow> parent, Args&&... args)
{
    static_assert(std::is_base_of_v<wxWindow, T>, "T is some window element.");
    // Parent takes ownership in constructor
    return new T(parent, std::forward<Args>(args)...);
}

/// Create T and return non-owning pointer, parent now owns the memory.
template <typename T, typename... Args>
inline RefPtr<T> NewSizer(RefPtr<wxWindow> parent, Args&&... args)
{
    static_assert(std::is_base_of_v<wxSizer, T>, "T is a sizer.");
    auto* child = new T(std::forward<Args>(args)...);
    // Parent takes ownership when setting sizer
    parent->SetSizer(child);
    return child;
}

/// Create T and return non-owning pointer, parent now owns the memory.
template <typename T, typename... Args>
inline RefPtr<T> NewSizer(RefPtr<wxSizer> parentSizer, wxSizerFlags flags, Args&&... args)
{
    // Sizer is immediately added to the parent sizer.
    static_assert(std::is_base_of_v<wxSizer, T>, "T is a sizer.");
    auto* child = new T(std::forward<Args>(args)...);
    // Parent sizer takes ownership of any sizer added to it
    parentSizer->Add(child, flags);
    return child;
}

/// Create T and return non-owning pointer, parent now owns the memory.
// Ignored T param to match api of NewSizer, and show what sizer is being created at callsite
template <typename T>
inline RefPtr<wxStaticBoxSizer> NewSizer(
    const wxString& label,
    RefPtr<wxWindow> parent,
    RefPtr<wxSizer> parentSizer,
    int orient = wxVERTICAL,
    wxSizerFlags flags = {})
{
    static_assert(std::is_same_v<T, wxStaticBoxSizer>, "Overload only for static box sizer.");
    auto* child = new wxStaticBoxSizer(orient, parent, label);
    // Parent sizer takes ownership of any sizer added to it
    parentSizer->Add(child, flags);
    return child;
}

/// Create button and return non-owning pointer to it.
/// Binds a lambda to the button click event
template <typename F>
inline RefPtr<wxButton> NewButton(RefPtr<wxWindow> parent, const wxString& label, wxSize size, const F& func)
{
    auto id = wxWindow::NewControlId();
    auto button = NewWindow<wxButton>(parent, id, label, wxDefaultPosition, size);
    parent->Bind(wxEVT_BUTTON, func, id);
    return button;
}

/// Create button and return non-owning pointer to it.
/// Binds a lambda to the button click event
template <typename F>
inline RefPtr<wxButton> NewButton(RefPtr<wxWindow> parent, const wxString& label, const F& func)
{
    return NewButton(parent, label, wxDefaultSize, func);
}

/// Create a menu bar and set its frame which now owns its memory
inline RefPtr<wxMenuBar> NewMenuBar(RefPtr<wxFrame> parent)
{
    auto* child = new wxMenuBar();
    parent->SetMenuBar(child);
    return child;
}

/// Append a menu to a menu bar
inline RefPtr<wxMenu> NewMenu(RefPtr<wxMenuBar> parent, const wxString& title)
{
    auto* child = new wxMenu();
    parent->Append(child, title);
    return child;
}

/// wraps a member function in a generic lambda.
/// lambda will store the instance pointer and function pointer.
template <typename T, typename RetT, typename... ArgTs>
inline auto MemberFunc(RetT (T::*memberFunc)(ArgTs...), T* instance)
{
    ATT_ASSERT(memberFunc != nullptr);
    ATT_ASSERT(instance != nullptr);
    return [=](ArgTs&&... args)
    {
        (instance->*memberFunc)(std::forward<ArgTs>(args)...);
    };
}

/// Convert from unicode, user input string, out_val is not modified if conversion fails.
/// Must be inverse of ToString.
inline bool FromWXString(const wxString& str, int& out_val)
{
    long val = 0;
    if (str.ToLong(&val))
    {
        out_val = static_cast<int>(val);
        return true;
    }
    return false;
}
inline bool FromWXString(const wxString& str, float& out_val)
{
    double val = 0;
    if (str.ToDouble(&val))
    {
        out_val = static_cast<float>(val);
        return true;
    }
    return false;
}
inline bool FromWXString(const wxString& str, double& out_val)
{
    double val = 0;
    if (str.ToDouble(&val))
    {
        out_val = val;
        return true;
    }
    return false;
}
inline bool FromWXString(const wxString& str, std::string& out_val)
{
    out_val = str.utf8_string();
    return true;
}
inline bool FromWXString(const wxString& str, wxString& out_val)
{
    out_val = str;
    return true;
}
template <typename T>
inline bool FromWXString(const wxString& str, FS::Valid<T>& out_val)
{
    T val;
    if (FromWXString(str, val))
    {
        out_val = std::forward<T>(val);
        return true;
    }
    return false;
}

/// Convert val to unicode string for display.
/// Must be inverse of FromString.
inline wxString ToWXString(const std::string& val)
{
    return wxString::FromUTF8Unchecked(val);
}
inline wxString ToWXString(const wxString& val)
{
    return val;
}
template <typename T>
inline wxString ToWXString(const T& val)
{
    std::ostringstream ss;
    ss << val;
    return wxString::FromUTF8(ss.str());
}
template <typename T>
inline wxString ToWXString(const FS::Valid<T>& val)
{
    return ToWXString(static_cast<const T&>(val));
}
