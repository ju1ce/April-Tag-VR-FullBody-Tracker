#pragma once

#define ATT_DETAIL_CONCAT(a, b) a##b
#define ATT_CONCAT(a, b) ATT_DETAIL_CONCAT(a, b)

#define ATT_DETAIL_STRINGIZE(a) #a
#define ATT_STRINGIZE(a) ATT_DETAIL_STRINGIZE(a)

#define ATT_NOOP() static_cast<void>(0)

#define ATT_SFINAE_TYPE_TRAIT(p_name, p_tType, p_tName, ...)              \
    template <p_tType p_tName, typename = void>                           \
    struct p_name : ::std::false_type                                     \
    {                                                                     \
    };                                                                    \
    template <p_tType p_tName>                                            \
    struct p_name<p_tName, ::std::void_t<__VA_ARGS__>> : ::std::true_type \
    {                                                                     \
    }

#define ATT_SFINAE_TYPE_TRAIT2(p_name, p_t1Type, p_t1Name, p_t2Type, p_t2Name, ...)  \
    template <p_t1Type p_t1Name, p_t2Type p_t2Name, typename = void>                 \
    struct p_name : ::std::false_type                                                \
    {                                                                                \
    };                                                                               \
    template <p_t1Type p_t1Name, p_t2Type p_t2Name>                                  \
    struct p_name<p_t1Name, p_t2Name, ::std::void_t<__VA_ARGS__>> : ::std::true_type \
    {                                                                                \
    }
