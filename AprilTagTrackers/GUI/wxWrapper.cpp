#include "wxWrapper.h"

#pragma warning(push)
#pragma warning(disable : 4996)
#include <wx/notebook.h>
#include <wx/string.h>
#include <wx/wx.h>
#pragma warning(pop)

namespace gui
{

// Pimpl Pattern
class UString::UStringImpl : public wxString
{
    // Inherit constructors
    using wxString::wxString;
};

UString::UString()
    : impl(new UStringImpl()) {}

UString::UString(const char* cStr)
    : impl(new UStringImpl(cStr)) {}

};