#pragma once

#include <type_traits>
#include <utility>

/// Placed in a class before a list of REFLECTABLE_FIELDS
#define REFLECTABLE_BEGIN                \
    friend class ::Reflect;              \
    template <size_t N, typename = void> \
    struct _rfl_FieldData;               \
    static constexpr size_t _rfl_fieldOffset = __COUNTER__

/// Placed in a class after a list of REFLECTABLE_FIELDS
#define REFLECTABLE_END \
    static constexpr size_t _rfl_fieldCount = __COUNTER__ - _rfl_fieldOffset - 1

#define REFLECTABLE_FIELD_DATA_COUNTER_(a_type, a_fieldName, a_counter)           \
    template <typename _unused_>                                                  \
    struct _rfl_FieldData<a_counter - _rfl_fieldOffset - 1, _unused_>             \
    {                                                                             \
        using Type = a_type;                                                      \
        static constexpr const char* name = #a_fieldName;                         \
        template <typename Self>                                                  \
        static inline Type& GetField(Self& self)                                  \
        {                                                                         \
            return self.a_fieldName;                                              \
        }                                                                         \
        template <typename Self>                                                  \
        static inline const std::remove_const_t<Type>& GetField(const Self& self) \
        {                                                                         \
            return self.a_fieldName;                                              \
        }                                                                         \
    }

#define REFLECTABLE_CONCAT_(a, b) a##b
#define REFLECTABLE_CONCAT(a, b) REFLECTABLE_CONCAT_(a, b)

#define REFLECTABLE_STRINGIZE_(a) #a
#define REFLECTABLE_STRINGIZE(a) REFLECTABLE_STRINGIZE_(a)

#define REFLECTABLE_FIELD_NONAME_COUNTER_(a_type, a_fieldNamePrefix, a_counter)                           \
    REFLECTABLE_FIELD_DATA_COUNTER_(a_type, REFLECTABLE_CONCAT(a_fieldNamePrefix, a_counter), a_counter); \
    a_type REFLECTABLE_CONCAT(a_fieldNamePrefix, a_counter)

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
    template <typename RT, typename F, size_t... Is>
    static constexpr void ForEachInternal(RT& reflType, F&& func, std::index_sequence<Is...>)
    {
        // Fold into list of func calls using fielddata at each index.
        (func(RT::template _rfl_FieldData<Is>::name,
             RT::template _rfl_FieldData<Is>::GetField(reflType)),
            ...);
    }

    template <typename... T>
    struct Voider
    {
        using Type = void;
    };

    template <typename... T>
    using VoiderT = typename Voider<T...>::Type;

    template <typename RT, typename = void>
    struct IsReflectable : std::false_type
    {
    };

    template <typename RT>
    struct IsReflectable<RT,
        VoiderT<typename RT::template _rfl_FieldData<0>,
            decltype(RT::_rfl_fieldOffset),
            decltype(RT::_rfl_fieldCount)>> : std::true_type
    {
    };

public:
    /// Iterate the type's reflectable fields.
    /// RT must contain the REFLECTABLE_BEGIN macro.
    /// Static polymorphism is necessary if a base class uses this function.
    /// @tparam F [](const char* name, FieldType& field) {}
    template <typename RT, typename F>
    static constexpr void ForEach(RT& reflType, F&& func)
    {
        ForEachInternal(reflType,
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

    /// Comptime check if a (template) type is reflectable (it uses the reflectable macros).
    /// eg. template <typename T> std::enable_if_t<Reflect::IsReflectable<T>> MyFunc() {}
    template <typename RT>
    static constexpr bool IsReflectableV = IsReflectable<RT>::value;
};
