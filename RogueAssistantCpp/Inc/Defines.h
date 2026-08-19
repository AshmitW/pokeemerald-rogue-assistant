#pragma once

#include <cstdint>
#include <cstring>
#include <cmath>
#include <climits>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define ARRAY_COUNT(arr) (size_t)(sizeof(arr) / sizeof((arr)[0]))

#define IMGUI_SUPPORT 0

#ifndef _MSC_VER
// memcpy_s is an MSVC extension. Same contract: refuse to write past the
// destination rather than truncating silently.
inline int memcpy_s(void* dest, size_t destSize, void const* src, size_t count)
{
    if (dest == nullptr) return 22;             // EINVAL
    if (count == 0) return 0;
    if (src == nullptr || count > destSize) return 34; // ERANGE
    std::memcpy(dest, src, count);
    return 0;
}
#endif
