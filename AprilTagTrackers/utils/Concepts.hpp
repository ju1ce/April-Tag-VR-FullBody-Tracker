#pragma once

#include <concepts> // IWYU pragma: keep

namespace utils
{

template <typename T, typename... TArgs>
concept Callable = std::is_invocable_v<T, TArgs...>;
template <typename T, typename TResult, typename... TArgs>
concept CallableR = std::is_invocable_r_v<TResult, T, TArgs...>;

template <typename T1, typename T2>
concept DecayAs = std::same_as<std::decay_t<T1>, std::decay_t<T2>>;

template <typename TValue, typename... TArgs>
concept ArgsDecayAs = ((DecayAs<TValue, TArgs>) && ...);
template <typename TValue, typename... TArgs>
concept ArgsSameAs = ((std::same_as<TValue, TArgs>) && ...);

} // namespace utils
