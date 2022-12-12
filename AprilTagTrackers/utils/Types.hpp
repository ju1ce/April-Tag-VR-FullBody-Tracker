#pragma once

#include <cstddef>

namespace utils
{

// set `cppcoreguidelines-narrowing-conversions.IgnoreConversionFromTypes: size_t;size_type`
// in .clang-tidy to silence some warnings for unsigned to signed conversion when using this type
using Index = std::ptrdiff_t;

}; // namespace utils

using utils::Index;
