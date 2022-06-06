#pragma once

#include <array>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

class SemVer
{
public:
    constexpr SemVer(int _major = 0, int _minor = 0, int _patch = 0) noexcept
        : major(_major), minor(_minor), patch(_patch)
    {
    }

    /// Parse "major[.minor[.patch]]", defaults to 0 if part is missing.
    static constexpr SemVer Parse(std::string_view str) noexcept;
    /// Test api compatibility
    static constexpr bool Compatible(const SemVer& a, const SemVer& b);
    static constexpr int Compare(const SemVer& a, const SemVer& b)
    {
        if (a.major != b.major) return a.major - b.major;
        if (a.minor != b.minor) return a.minor - b.minor;
        if (a.patch != b.patch) return a.patch - b.patch;
        return 0;
    }

    void Major(int _major) noexcept { major = _major; }
    void Minor(int _minor) noexcept { minor = _minor; }
    void Patch(int _patch) noexcept { patch = _patch; }

    int Major() const noexcept { return major; }
    int Minor() const noexcept { return minor; }
    int Patch() const noexcept { return patch; }

    std::string ToString() const
    {
        return (std::ostringstream() << major << '.' << minor << '.' << patch).str();
    }

    friend std::ostream& operator<<(std::ostream& os, const SemVer& ver)
    {
        return os << ver.major << "." << ver.minor << "." << ver.patch;
    }

private:
    int major = 0;
    int minor = 0;
    int patch = 0;
};

constexpr bool operator==(const SemVer& a, const SemVer& b) { return SemVer::Compare(a, b) == 0; }
constexpr bool operator!=(const SemVer& a, const SemVer& b) { return SemVer::Compare(a, b) != 0; }
constexpr bool operator>(const SemVer& a, const SemVer& b) { return SemVer::Compare(a, b) > 0; }
constexpr bool operator>=(const SemVer& a, const SemVer& b) { return SemVer::Compare(a, b) >= 0; }
constexpr bool operator<(const SemVer& a, const SemVer& b) { return SemVer::Compare(a, b) < 0; }
constexpr bool operator<=(const SemVer& a, const SemVer& b) { return SemVer::Compare(a, b) <= 0; }

constexpr SemVer SemVer::Parse(std::string_view str) noexcept
{
    std::array<int, 3> ver = {0, 0, 0};
    int verIdx = 0;
    for (int i = 0; i < static_cast<int>(str.size()); i++)
    {
        const char c = str[i];
        if (c >= '0' && c <= '9')
        {
            int digit = static_cast<int>(c - '0');
            ver[verIdx] = (ver[verIdx] * 10) + digit;
        }
        else if (c == '.')
        {
            if (verIdx == 2) break;
            if (i == static_cast<int>(str.size()) - 1) break;
            verIdx++;
        }
        else break;
    }
    return {ver[0], ver[1], ver[2]};
}

constexpr bool SemVer::Compatible(const SemVer& a, const SemVer& b)
{
    /// doesn't make sense to compare zero versions
    if (a == SemVer(0, 0, 0) || b == SemVer(0, 0, 0)) return false;
    /// if major is 0, alpha phase means minor can change compatibility
    if (a.major == 0 && b.major == 0)
        return a.minor == b.minor;
    /// otherwise, major indicates compatibility
    return a.major == b.major;
}
