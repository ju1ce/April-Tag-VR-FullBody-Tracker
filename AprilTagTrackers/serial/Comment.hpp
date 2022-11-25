#pragma once

#include "Serial.hpp"
#include "utils/Macros.hpp"
#include "utils/Reflectable.hpp"

#define ATT_DETAIL_SERIAL_COMMENT(p_str, p_counter)                                                            \
    REFLECTABLE_FIELD_DATA_COUNTER_(const ::serial::Comment, ATT_CONCAT(_srl_comment_, p_counter), p_counter); \
    static constexpr ::serial::Comment ATT_CONCAT(_srl_comment_, p_counter)                                    \
    {                                                                                                          \
        p_str                                                                                                  \
    }

#define ATT_SERIAL_COMMENT(p_str) \
    ATT_DETAIL_SERIAL_COMMENT(p_str, __COUNTER__)

namespace serial
{

struct Comment
{
    constexpr explicit Comment(std::string_view s) : str(s) {}

    std::string_view str;
};

template <>
struct Serial<Comment>
{
    void Format(auto& ctx, const Comment& value)
    {
        ctx.WriteComment(value.str);
    }
    void Parse(auto&, Comment&) {}
    void FormatKey(auto& ctx, std::string_view, const Comment& value)
    {
        ctx.WriteComment(value.str);
    }
    bool ParseKey(auto&, std::string_view, Comment&)
    {
        return false;
    }
};

} // namespace serial
