#pragma once

#include "utils/Cross.hpp"

#include <string_view>

namespace utils
{

template <std::size_t Length>
    requires(Length >= 1)
using RawStringLiteral = const char(&)[Length]; // NOLINT(*c-arrays):

template <typename T>
concept IsRawStringLiteral = std::same_as<T, RawStringLiteral<std::extent_v<std::remove_reference_t<T>>>>;

class StringLiteral
{
public:
    template <std::size_t Length>
    consteval StringLiteral(RawStringLiteral<Length> literal) noexcept // NOLINT(*explicit-constructor)
        : mView(literal, Length - 1)
    {
        if (literal[Length - 1] != '\0') utils::Unreachable();
    }

    constexpr std::string_view view() const noexcept { return mView; } // NOLINT(*naming)

private:
    std::string_view mView;
};

} // namespace utils
