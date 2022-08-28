#pragma once

#include "Macros.hpp"

#include <type_traits>

/// Placed in a class before a list of REFLECTABLE_FIELDs
#define REFLECTABLE_BEGIN                \
    friend class ::Reflect;              \
    template <size_t N, typename = void> \
    struct _rfl_FieldData;               \
    static constexpr size_t _rfl_fieldOffset = __COUNTER__

/// Placed in a class after a list of REFLECTABLE_FIELDS
#define REFLECTABLE_END \
    static constexpr size_t _rfl_fieldCount = __COUNTER__ - _rfl_fieldOffset - 1

#define REFLECTABLE_FIELD_DATA_COUNTER_(a_type, a_fieldName, a_counter) \
    template <typename _unused_>                                        \
    struct _rfl_FieldData<a_counter - _rfl_fieldOffset - 1, _unused_>   \
    {                                                                   \
        using Type = a_type;                                            \
        static constexpr const char* name = #a_fieldName;               \
        template <typename Self>                                        \
        static inline Type& GetField(Self& self)                        \
        {                                                               \
            return self.a_fieldName;                                    \
        }                                                               \
        template <typename Self>                                        \
        static inline const Type& GetField(const Self& self)            \
        {                                                               \
            return self.a_fieldName;                                    \
        }                                                               \
    }

#define REFLECTABLE_FIELD_NONAME_COUNTER_(a_type, a_fieldNamePrefix, a_counter)                   \
    REFLECTABLE_FIELD_DATA_COUNTER_(a_type, ATT_CONCAT(a_fieldNamePrefix, a_counter), a_counter); \
    a_type ATT_CONCAT(a_fieldNamePrefix, a_counter)

/// Declare a reflectable field with type and name, expects a member field with name to be defined in the class
#define REFLECTABLE_FIELD_DATA(a_type, a_fieldName) \
    REFLECTABLE_FIELD_DATA_COUNTER_(a_type, a_fieldName, __COUNTER__)

/// Declare a reflectable field with type and a unique name with custom prefix
#define REFLECTABLE_FIELD_NONAME(a_type, a_fieldNamePrefix) \
    REFLECTABLE_FIELD_NONAME_COUNTER_(a_type, a_fieldNamePrefix, __COUNTER__)

/// Declare a reflectable field with type and name
#define REFLECTABLE_FIELD(a_type, a_fieldName)   \
    REFLECTABLE_FIELD_DATA(a_type, a_fieldName); \
    a_type a_fieldName

/// Auto-friended by reflectable types, provides functionality on them.
class Reflect
{
public:
    template <typename RT, typename = void>
    struct IsReflectable : std::false_type
    {
    };

    template <typename RT>
    struct IsReflectable<RT,
        std::void_t<typename RT::template _rfl_FieldData<0>,
            decltype(RT::_rfl_fieldOffset),
            decltype(RT::_rfl_fieldCount)>> : std::true_type
    {
    };

    template <typename RT, size_t I, typename = void>
    struct IsReflectableIndex : std::false_type
    {
    };

    template <typename RT, size_t I>
    struct IsReflectableIndex<RT, I,
        std::void_t<decltype(RT::template _rfl_FieldData<I>::name)>> : std::true_type
    {
    };

    /// Comptime check if a (template) type is reflectable (it uses the reflectable macros).
    /// eg. template <typename T> std::enable_if_t<Reflect::IsReflectable<T>> MyFunc() {}
    template <typename RT>
    static constexpr bool IsReflectableV = IsReflectable<RT>::value;
    template <typename RT, size_t I>
    static constexpr bool IsReflectableIndexV = IsReflectableIndex<RT, I>::value;
    template <typename RT>
    static constexpr size_t FieldCount = RT::_rfl_fieldCount;

private:
    template <typename RT, typename F, size_t I>
    static constexpr void ForEachCall(RT& reflType, F&& func)
    {
        if constexpr (IsReflectableIndexV<RT, I>)
        {
            using FieldData = typename RT::template _rfl_FieldData<I>;
            func(FieldData::name, FieldData::GetField(reflType));
        }
    }

    template <typename RT, typename F, size_t... Is>
    static constexpr void ForEachFold(RT& reflType, F&& func, std::index_sequence<Is...>)
    {
        // Fold into list of func calls using field data at each index.
        (ForEachCall<RT, F, Is>(reflType, std::forward<F>(func)), ...);
    }

public:
    /// Iterate the type's reflectable fields.
    /// RT must contain the REFLECTABLE_BEGIN macro.
    /// Static polymorphism is necessary if a base class uses this function.
    /// @tparam F [](const char* name, FieldType& field) {}
    template <typename RT, typename F>
    static void ForEach(RT& reflType, F&& func)
    {
        ForEachFold(reflType,
            std::forward<F>(func),
            std::make_index_sequence<RT::_rfl_fieldCount>());
    }

    /// To help with static polymorphism when calling ForEach.
    /// Add DerT as a template parameter of BaseT
    template <typename DerT, typename BaseT>
    static constexpr DerT& DerivedThis(BaseT& baseThis)
    {
        static_assert(std::is_base_of_v<BaseT, DerT>, "DerT is derived from BaseT.");
        return static_cast<DerT&>(baseThis);
    }

    /// To help with static polymorphism when calling ForEach.
    /// Add DerT as a template parameter of BaseT
    template <typename DerT, typename BaseT>
    static constexpr const DerT& DerivedThis(const BaseT& baseThis)
    {
        static_assert(std::is_base_of_v<BaseT, DerT>, "DerT is derived from BaseT.");
        return static_cast<const DerT&>(baseThis);
    }
};
