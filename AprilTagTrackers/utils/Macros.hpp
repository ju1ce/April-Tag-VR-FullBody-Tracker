#pragma once

#define ATT_DETAIL_CONCAT(a, b) a##b
#define ATT_CONCAT(a, b) ATT_DETAIL_CONCAT(a, b)

#define ATT_DETAIL_STRINGIZE(a) #a
#define ATT_STRINGIZE(a) ATT_DETAIL_STRINGIZE(a)

#define ATT_NOOP() static_cast<void>(0)
