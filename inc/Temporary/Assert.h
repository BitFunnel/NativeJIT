#pragma once

// MSVC has __pragma directive for use in macros since #pragma would apply to the macro itself.
#ifdef NATIVEJIT_PLATFORM_WINDOWS
#define PREPROCESSOR_PRAGMA_WARNING(x) __pragma(warning(x))
#else
#define PREPROCESSOR_PRAGMA_WARNING(x)
#endif


#define LogThrowAssert(condition, ...)        \
do                                            \
{                                             \
    if (!(condition))                         \
    {                                         \
        NativeJIT::LogThrowImpl(__FILE__,     \
                                __FUNCTION__, \
                                __LINE__,     \
                                #condition,   \
                                __VA_ARGS__); \
    }                                         \
PREPROCESSOR_PRAGMA_WARNING(push)             \
PREPROCESSOR_PRAGMA_WARNING(disable:4127)     \
} while (false)                               \
PREPROCESSOR_PRAGMA_WARNING(pop)


#define LogThrowAbort(...)                    \
    NativeJIT::LogThrowImpl(__FILE__,         \
                            __FUNCTION__,     \
                            __LINE__,         \
                            "Forced failure", \
                            __VA_ARGS__)


namespace NativeJIT
{
    void LogThrowImpl(char const * filename,
                      char const * function,
                      unsigned lineNumber,
                      char const * title,
                      char const * format,
                      ...);
}
