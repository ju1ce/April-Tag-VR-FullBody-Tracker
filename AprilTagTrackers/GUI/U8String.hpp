// Allows for public apis to take unicode strings without dealing with wxString.
// Main purpose is to call wxString::FromUTF8Unchecked when passed to a wxString arg in gui code,
// as by default, it will use the current locale.

#pragma once

#include "serial/Serial.hpp"

#include <string>
#include <string_view>

class wxString;

class U8String
{
public:
    friend struct serial::Serial<U8String>;

    U8String() = default;
    U8String(const char* const _str) : str(_str) {}
    U8String(std::string _str) : str(std::move(_str)) {}
    operator wxString() const;

    U8String& operator+=(const U8String& rhs)
    {
        str += rhs.str;
        return *this;
    }
    U8String& operator+=(const std::string& rhs)
    {
        str += rhs;
        return *this;
    }
    friend U8String operator+(const U8String& lhs, const U8String& rhs) { return lhs.str + rhs.str; }
    friend U8String operator+(const U8String& lhs, const std::string& rhs) { return lhs.str + rhs; }
    friend U8String operator+(const std::string& lhs, const U8String& rhs) { return lhs + rhs.str; }

private:
    std::string str;
};

class U8StringView
{
public:
    constexpr U8StringView(std::string_view _str) : str(std::move(_str)) {}
    constexpr U8StringView(const char* const _str) : str(_str) {}
    operator wxString() const;

private:
    std::string_view str;
};

template <>
struct serial::Serial<U8String>
{
    static void Parse(auto& ctx, U8String& value)
    {
        ctx.Read(value.str);
    }
    static void Format(auto& ctx, const U8String& value)
    {
        ctx.Write(value.str);
    }
};
