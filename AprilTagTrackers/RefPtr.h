// Defines the RefPtr<T> class, a default constructable std::reference_wrapper.

#pragma once

#include "Debug.h"

#include <memory>
#include <optional>
#include <type_traits>

// forward declare for RefPtr conversion
template <typename T>
class OptRefPtr;

/// Non-owning, never null, "reference pointer".
/// A reference pointer acts like a reference most of the time,
/// however, it allows for stl containers to store references.
/// This is almost identical to std::reference_wrapper,
/// except that it has a default constructor which initializes to an invalid state (nullptr),
/// and the define, ATT_ENABLE_ASSERT, will cause assertion failure
/// for all other constructors and operators used with nullptr values.
template <typename T>
class RefPtr
{
    static_assert(!std::is_reference_v<T>, "Cannot form pointer to reference.");

private:
    /// Is implicit pointer conversion possible from T pointer to this pointer.
    template <typename From>
    static constexpr bool IsConv() noexcept
    {
        return std::is_convertible_v<std::add_pointer_t<From>, Pointer>;
    }

public:
    // Friend any template instantiation.
    template <typename T2>
    friend class RefPtr;

    using Type = T;
    using Pointer = Type*;
    using Reference = Type&;

    // Non-owning.
    ~RefPtr() = default;

    /// Only use for default constructors.
    /// No null checking as this could be a class member.
    constexpr RefPtr() noexcept
        : rawPtr(nullptr) {}

    /// Copy from RefPtr.
    /// No null checking as this could be a class member.
    constexpr RefPtr(const RefPtr& other) noexcept
        : rawPtr(other.rawPtr) {}

    /// Move from and invalidate not-null, RefPtr (sets to nullptr).
    /// No null checking as this could be a class member.
    constexpr RefPtr(RefPtr&& other) noexcept
        : rawPtr(other.rawPtr)
    {
#ifdef ATT_ENABLE_ASSERT
        // Make assertions fail for moved from (other),
        // unnecessary if assertions are disabled,
        // as using moved from data is undefined behaviour.
        other.rawPtr = nullptr;
#endif
    }

    /// Copy from not-null RefPtr.
    /// No null checking as this could be a class member.
    RefPtr& operator=(const RefPtr& rhs) noexcept
    {
        rawPtr = rhs.rawPtr;
        return *this;
    }

    /// Move from and invalidate not-null RefPtr (sets to nullptr).
    /// No null checking as this could be a class member.
    RefPtr& operator=(RefPtr&& rhs) noexcept
    {
        rawPtr = rhs.rawPtr;
#ifdef ATT_ENABLE_ASSERT
        rhs.rawPtr = nullptr;
#endif
        return *this;
    }

    // /// Construct with not-null, raw pointer.
    // RefPtr(Pointer _rawPtr) noexcept
    //     : rawPtr(_rawPtr)
    // {
    //     ATASSERT("Never null.", rawPtr != nullptr);
    // }

    /// Construct with not-null, convertable, raw pointer.
    template <typename T2, typename = std::enable_if_t<IsConv<T2>()>>
    RefPtr(T2* _rawPtr) noexcept
        : rawPtr(_rawPtr)
    {
        ATASSERT("Never null.", rawPtr != nullptr);
    }

    /// Copy from convertable RefPtr.
    template <typename T2, typename = std::enable_if_t<IsConv<T2>()>>
    RefPtr(const RefPtr<T2>& other) noexcept
        : rawPtr(other.rawPtr)
    {
        ATASSERT("Never null.", rawPtr != nullptr);
    }

    /// Move from and invalidate not-null, convertable, RefPtr (sets to nullptr).
    template <typename T2, typename = std::enable_if_t<IsConv<T2>()>>
    RefPtr(RefPtr<T2>&& other) noexcept
        : rawPtr(other.rawPtr)
    {
#ifdef ATT_ENABLE_ASSERT
        other.rawPtr = nullptr;
#endif
        ATASSERT("Never null.", rawPtr != nullptr);
    }

    /// Copy from not-null, convertable OptRefPtr
    template <typename T2, typename = std::enable_if_t<IsConv<T2>()>>
    RefPtr(const OptRefPtr<T2>& other)
        : rawPtr(other.Get())
    {
    }

    /// Copy from not-null std::unique_ptr.get().
    template <typename T2, typename = std::enable_if_t<IsConv<T2>()>>
    RefPtr(const std::unique_ptr<T2>& other) noexcept
        : RefPtr<T>(other.get())
    {
        ATASSERT("Never null.", rawPtr != nullptr);
    }

    /// Copy from not-nullopt or nullptr std::optional.value().
    template <typename T2, typename = std::enable_if_t<IsConv<T2>()>>
    RefPtr(const std::optional<T2>& other) noexcept
        : RefPtr<T>(other.value())
    {
        ATASSERT("Never null.", rawPtr != nullptr);
    }

    Pointer Get() const noexcept
    {
        ATASSERT("Never null.", rawPtr != nullptr);
        return rawPtr;
    }

    Pointer operator->() const noexcept { return Get(); }
    Reference operator*() const noexcept { return *Get(); }
    operator Pointer() const noexcept { return Get(); }

    /// dynamic_cast pointer to T2 pointer, returns nullopt if failed
    template <typename T2>
    std::optional<RefPtr<T2>> DynamicCast() const
    {
        T2* rawCasted = dynamic_cast<T2*>(Get());
        if (rawCasted == nullptr) return std::nullopt;
        return std::optional(RefPtr<T2>(rawCasted));
    }

private:
    Pointer rawPtr;
};

template <typename T1, typename T2>
bool operator==(const RefPtr<T1>& lhs, const RefPtr<T2>& rhs)
{
    return lhs.Get() == rhs.Get();
}
template <typename T1, typename T2>
bool operator!=(const RefPtr<T1>& lhs, const RefPtr<T2>& rhs)
{
    return !operator==(lhs, rhs);
}

template <typename T>
bool operator==(const RefPtr<T>& lhs, const std::nullptr_t& rhs) = delete;
template <typename T>
bool operator==(const std::nullptr_t& lhs, const RefPtr<T>& rhs) = delete;

template <typename T>
bool operator!=(const RefPtr<T>& lhs, const std::nullptr_t& rhs) = delete;
template <typename T>
bool operator!=(const std::nullptr_t& lhs, const RefPtr<T>& rhs) = delete;

template <typename T>
RefPtr(std::unique_ptr<T>) -> RefPtr<T>;
template <typename T>
RefPtr(std::unique_ptr<T*>) -> RefPtr<T>;
template <typename T>
RefPtr(std::optional<T>) -> RefPtr<T>;
template <typename T>
RefPtr(std::optional<T*>) -> RefPtr<T>;
template <typename T>
RefPtr(OptRefPtr<T>) -> RefPtr<T>;
template <typename T>
RefPtr(OptRefPtr<T*>) -> RefPtr<T>;

/// Nullable non-owning pointer, like std::optional<RefPtr<T>> without the annoyances and unnecessary bool.
template <typename T>
class OptRefPtr
{
    static_assert(!std::is_reference_v<T>, "Cannot form pointer to reference.");

private:
    /// Is implicit pointer conversion possible from T pointer to this pointer.
    template <typename From>
    static constexpr bool IsConv() noexcept
    {
        return std::is_convertible_v<std::add_pointer_t<From>, Pointer>;
    }

public:
    // Friend any template instantiation.
    template <typename T2>
    friend class OptRefPtr;

    using Type = T;
    using Pointer = Type*;
    using Reference = Type&;

    // Non-owning.
    ~OptRefPtr() = default;

    /// Default construct with nullptr
    constexpr OptRefPtr() noexcept
        : rawPtr(nullptr) {}

    /// Copy from OptRefPtr.
    constexpr OptRefPtr(const OptRefPtr& other) noexcept
        : rawPtr(other.rawPtr) {}

    /// Move from and invalidate OptRefPtr (sets to nullptr).
    constexpr OptRefPtr(OptRefPtr&& other) noexcept
        : rawPtr(other.rawPtr)
    {
#ifdef ATT_ENABLE_ASSERT
        // Make assertions fail for moved from (other),
        // unnecessary if assertions are disabled,
        // as using moved from data is undefined behaviour.
        other.rawPtr = nullptr;
#endif
    }

    /// Copy from OptRefPtr.
    OptRefPtr& operator=(const OptRefPtr& rhs) noexcept
    {
        rawPtr = rhs.rawPtr;
        return *this;
    }

    /// Move from and invalidate OptRefPtr (sets to nullptr).
    OptRefPtr& operator=(OptRefPtr&& rhs) noexcept
    {
        rawPtr = rhs.rawPtr;
#ifdef ATT_ENABLE_ASSERT
        rhs.rawPtr = nullptr;
#endif
        return *this;
    }

    // /// Construct with raw pointer.
    // OptRefPtr(Pointer _rawPtr) noexcept
    //     : rawPtr(_rawPtr)
    // {
    // }

    /// Construct with convertable raw pointer.
    template <typename T2, typename = std::enable_if_t<IsConv<T2>()>>
    OptRefPtr(T2* _rawPtr) noexcept
        : rawPtr(_rawPtr)
    {
    }

    /// Copy from convertable OptRefPtr.
    template <typename T2, typename = std::enable_if_t<IsConv<T2>()>>
    OptRefPtr(const OptRefPtr<T2>& other) noexcept
        : rawPtr(other.rawPtr)
    {
    }

    /// Move from and invalidate convertable OptRefPtr (sets to nullptr).
    template <typename T2, typename = std::enable_if_t<IsConv<T2>()>>
    OptRefPtr(OptRefPtr<T2>&& other) noexcept
        : rawPtr(other.rawPtr)
    {
#ifdef ATT_ENABLE_ASSERT
        other.rawPtr = nullptr;
#endif
    }

    /// Copy from not-null, convertable RefPtr
    template <typename T2, typename = std::enable_if_t<IsConv<T2>()>>
    OptRefPtr(const RefPtr<T2>& other) noexcept
        : rawPtr(other.Get())
    {
    }

    /// Copy from std::unique_ptr.get().
    template <typename T2, typename = std::enable_if_t<IsConv<T2>()>>
    OptRefPtr(const std::unique_ptr<T2>& other) noexcept
        : OptRefPtr(other.get())
    {
    }

    /// Copy from std::optional.
    template <typename T2, typename = std::enable_if_t<IsConv<T2>()>>
    OptRefPtr(const std::optional<T2>& other) noexcept
        : OptRefPtr(other.has_value() ? other.value() : nullptr)
    {
    }

    bool IsNull() const noexcept
    {
        return rawPtr == nullptr;
    }

    bool NotNull() const noexcept
    {
        return rawPtr != nullptr;
    }

    Pointer Get() const noexcept
    {
        ATASSERT("Not null.", rawPtr != nullptr);
        return rawPtr;
    }

    Pointer operator->() const noexcept { return Get(); }
    Reference operator*() const noexcept { return *Get(); }
    operator Pointer() const noexcept { return Get(); }

    /// dynamic_cast pointer to T2 pointer, returns nullopt if failed
    template <typename T2>
    std::optional<OptRefPtr<T2>> DynamicCast() const
    {
        T2* rawCasted = dynamic_cast<T2*>(Get());
        if (rawCasted == nullptr) return std::nullopt;
        return std::optional(OptRefPtr<T2>(rawCasted));
    }

private:
    Pointer rawPtr;
};

template <typename T1, typename T2>
bool operator==(const OptRefPtr<T1>& lhs, const OptRefPtr<T2>& rhs)
{
    return lhs.rawPtr == rhs.rawPtr;
}
template <typename T1, typename T2>
bool operator!=(const OptRefPtr<T1>& lhs, const OptRefPtr<T2>& rhs)
{
    return !operator==(lhs, rhs);
}

template <typename T>
OptRefPtr(std::unique_ptr<T>) -> OptRefPtr<T>;
template <typename T>
OptRefPtr(std::unique_ptr<T*>) -> OptRefPtr<T>;
template <typename T>
OptRefPtr(std::optional<T>) -> OptRefPtr<T>;
template <typename T>
OptRefPtr(std::optional<T*>) -> OptRefPtr<T>;
template <typename T>
OptRefPtr(RefPtr<T>) -> OptRefPtr<T>;
template <typename T>
OptRefPtr(RefPtr<T*>) -> OptRefPtr<T>;
