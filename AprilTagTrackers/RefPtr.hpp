// Defines the RefPtr<T> class, a default constructable std::reference_wrapper.

#pragma once

#include "utils/Assert.hpp"

#include <concepts>
#include <memory>
#include <type_traits>

// NOLINTBEGIN(*explicit-constructor)
template <typename T>
concept CanBePtr = (!std::is_array_v<T> &&
                    !std::is_reference_v<T> &&
                    !std::is_same_v<T, nullptr_t>);
template <typename From, typename To>
concept PtrConvertibleTo = (CanBePtr<From> &&
                            std::convertible_to<std::add_pointer_t<From>, std::add_pointer_t<To>>);

/// raw pointer to single item with null assertions
template <CanBePtr T>
class Ptr
{
public:
    using value_type = T; // proxy tag

    ATT_ALWAYS_INLINE(constexpr explicit Ptr(nullptr_t) noexcept) : mRawPtr(nullptr) {}
    ATT_ALWAYS_INLINE(constexpr Ptr(Ptr&& other) noexcept) = default;
    ATT_ALWAYS_INLINE(constexpr Ptr(const Ptr& other) noexcept) = default;
    template <PtrConvertibleTo<T> U>
    ATT_ALWAYS_INLINE(constexpr Ptr(Ptr<U> other) noexcept) : mRawPtr(other.Get()) {}
    template <std::derived_from<T> U>
    ATT_ALWAYS_INLINE(constexpr Ptr(U* rawPtr) noexcept) : mRawPtr(rawPtr) { ATT_ASSERT(mRawPtr != nullptr); }
    template <PtrConvertibleTo<T> U>
        requires(!std::derived_from<U, T>)
    ATT_ALWAYS_INLINE(constexpr explicit Ptr(U* rawPtr) noexcept) : mRawPtr(rawPtr)
    {
        ATT_ASSERT(mRawPtr != nullptr);
    }
    template <PtrConvertibleTo<T> U>
    [[deprecated]] ATT_ALWAYS_INLINE(constexpr Ptr(const std::unique_ptr<U>& other) noexcept) : mRawPtr(other.get())
    {
        ATT_ASSERT(mRawPtr != nullptr);
    }

    template <PtrConvertibleTo<T> U>
    ATT_ALWAYS_INLINE(constexpr void Reset(U* rawPtr) noexcept)
    {
        ATT_ASSERT(rawPtr != nullptr);
        mRawPtr = rawPtr;
    }
    template <PtrConvertibleTo<T> U>
    ATT_ALWAYS_INLINE(constexpr void Reset(Ptr<U> ptr) noexcept) { mRawPtr = ptr.Get(); }
    ATT_ALWAYS_INLINE(constexpr void Reset(nullptr_t) noexcept) { mRawPtr = nullptr; }
    ATT_ALWAYS_INLINE(constexpr T* Get() const noexcept)
    {
        ATT_ASSERT(mRawPtr != nullptr);
        return mRawPtr;
    }
    ATT_ALWAYS_INLINE(constexpr T* operator->() const noexcept) { return Get(); }
    ATT_ALWAYS_INLINE(constexpr T& operator*() const noexcept) { return *Get(); }
    ATT_ALWAYS_INLINE(constexpr operator bool() const noexcept) { return mRawPtr != nullptr; }

    template <PtrConvertibleTo<T> U>
    ATT_ALWAYS_INLINE(constexpr bool operator==(Ptr<U> rhs) const noexcept) { return Get() == rhs.Get(); }
    template <PtrConvertibleTo<T> U>
    ATT_ALWAYS_INLINE(constexpr bool operator!=(Ptr<U> rhs) const noexcept) { return Get() != rhs.Get(); }

    [[deprecated("use nullptr constructor")]] constexpr Ptr() noexcept : mRawPtr(nullptr) {}
    [[deprecated("use Reset")]] constexpr Ptr& operator=(Ptr&& rhs) noexcept = default;
    [[deprecated("use Reset")]] constexpr Ptr& operator=(const Ptr& rhs) noexcept = default;

    [[deprecated("use Get")]] constexpr operator T*() noexcept { return Get(); }

    [[deprecated("use Reset")]] constexpr void SetNull() noexcept { mRawPtr = nullptr; }
    [[deprecated("use bool conversion")]] constexpr bool IsNull() const noexcept { return mRawPtr == nullptr; }
    [[deprecated("use bool conversion")]] constexpr bool NotNull() const noexcept { return mRawPtr != nullptr; }
    /// dynamic_cast pointer to T2 pointer

    template <CanBePtr U>
    [[deprecated("use dynamic_cast")]] Ptr<U> DynamicCast() const noexcept
    {
        U* result = dynamic_cast<U*>(Get());
        if (result == nullptr) return Ptr<U>(nullptr);
        return Ptr<U>(result);
    }

private:
    T* mRawPtr;
};

template <CanBePtr T>
Ptr(T*) -> Ptr<T>;

/// RefPtr with a nullptr constructor
template <CanBePtr T>
class LateRefPtr : public Ptr<T>
{
public:
    using Ptr<T>::Ptr;
    constexpr LateRefPtr(Ptr<T> ptr) : Ptr<T>(ptr.Get()) {}

    template <PtrConvertibleTo<T> U>
    constexpr void Reset(Ptr<U>) noexcept = delete;
    template <PtrConvertibleTo<T> U>
    constexpr void Reset(U*) noexcept = delete;
    constexpr void Reset(nullptr_t) noexcept = delete;
};

/// never null pointer
template <CanBePtr T>
class RefPtr : public LateRefPtr<T>
{
public:
    using LateRefPtr<T>::LateRefPtr;
    [[deprecated("use LateRefPtr")]] constexpr explicit RefPtr(nullptr_t) noexcept
        : LateRefPtr<T>(nullptr) {}
};
// NOLINTEND(*explicit-constructor)

template <typename T>
using OptRefPtr = Ptr<T>;
