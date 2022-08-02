#pragma once

#include "Reader.hpp"
#include "Writer.hpp"

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

class Comment
{
public:
    constexpr explicit Comment(const char* _str) : str(_str) {}

    friend void Read<Comment>(const Reader& reader, const std::string& key, Comment& value);
    friend void Write<Comment>(Writer& writer, const std::string& key, const Comment& value);

private:
    const char* str;
};

template <>
inline void Read(const Reader& reader, const std::string& key, Comment& value)
{
    // nothing to read
}
template <>
inline void Write(Writer& writer, const std::string& key, const Comment& value)
{
    writer.writeComment(value.str);
}

} // namespace serial
