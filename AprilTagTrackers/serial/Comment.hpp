#pragma once

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
    constexpr explicit Comment(const char* _str) : str(_str) {}

    const char* str;
};

} // namespace serial
