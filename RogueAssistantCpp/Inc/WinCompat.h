#pragma once

// The Windows API surface this project actually uses is tiny and confined to
// logging, debugger detection and thread naming. Rather than #ifdef every call
// site, provide Linux equivalents behind the same names.

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#else

#include <pthread.h>
#include <ctime>
#include <cstdio>
#include <cerrno>
#include <cstdint>

inline bool IsDebuggerPresent() { return false; }
inline void OutputDebugStringA(char const*) {}

inline unsigned long GetCurrentThreadId()
{
    return static_cast<unsigned long>(reinterpret_cast<uintptr_t>(
        reinterpret_cast<void*>(pthread_self())));
}

// MSVC "secure" CRT variants, same argument order and 0-on-success contract.
inline int fopen_s(FILE** stream, char const* filename, char const* mode)
{
    if (stream == nullptr) return EINVAL;
    *stream = std::fopen(filename, mode);
    return (*stream != nullptr) ? 0 : errno;
}

inline int localtime_s(struct tm* result, time_t const* time)
{
    if (result == nullptr || time == nullptr) return EINVAL;
    return (localtime_r(time, result) != nullptr) ? 0 : EINVAL;
}

inline void Sleep(unsigned long milliseconds)
{
    timespec request;
    request.tv_sec = static_cast<time_t>(milliseconds / 1000);
    request.tv_nsec = static_cast<long>((milliseconds % 1000) * 1000000L);
    nanosleep(&request, nullptr);
}

#endif

// Symbol export: __declspec on MSVC, visibility attribute elsewhere.
#ifdef _WIN32
#define ROGUE_EXPORT __declspec(dllexport)
#else
#define ROGUE_EXPORT __attribute__((visibility("default")))
#endif
