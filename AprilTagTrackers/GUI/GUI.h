#pragma once

#include <memory>
#include <string>

namespace gui
{

/// Unicode string used for all user facing text, as well as translation files
class UString
{
public:
    /// empty string
    UString();
    /// utf8 or ascii c string
    UString(const char* cStr);
    /// utf8 or ascii std string
    UString(std::string&& stdStr);

private:
    // Pimpl Pattern
    class UStringImpl;       // Forward declare impl
    UStringImpl* const impl; // Manage impl memory
};

};