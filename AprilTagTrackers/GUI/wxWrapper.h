#pragma once

#include <string>

namespace gui
{

// On Windows this will be utf-16 and is essentially a typedef for std::wstring,
//  other platforms this is an encoding aware utf-8 string.
// May be useful for windows file paths aswell, although opencv and other
//  libraries don't always support io with widestring paths anyway.
// In general, all unicode string literals should be in the translation yaml files,
//  and not in a source file.

/// Unicode string used for all user facing text, as well as translation files
class UString
{
public:
    /// empty string
    UString();
    /// utf8 or ascii c string
    UString(const char* cStr);

private:
    // Pimpl Pattern
    class UStringImpl;       // Forward declare impl
    UStringImpl* const impl; // Manage impl memory
};

/// Represents an OS window.
class Window
{
public:

};

/// Holds UI elements and is placed on a window
class Page
{
public:

};

};