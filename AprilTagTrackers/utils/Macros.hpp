#pragma once

#define ATT_DETAILS_CONCAT(a, b) a##b
#define ATT_CONCAT(a, b) ATT_DETAILS_CONCAT(a, b)

#define ATT_DETAILS_STRINGIZE(a) #a
#define ATT_STRINGIZE(a) ATT_DETAILS_STRINGIZE(a)

#define ATT_NOOP() static_cast<void>(0)
