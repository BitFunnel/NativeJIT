#pragma once

#include <cstdarg>
#include <cstdio>
#include <stdexcept>
#include <sstream>
#include <vector>


namespace NativeJIT
{
    #define Assert(condition, ...)       \
    {                                    \
        AssertImpl(__FILE__,             \
                    __FUNCTION__,        \
                    __LINE__,            \
                    condition,           \
                    #condition,          \
                    __VA_ARGS__);        \
    }


    // Note: inlining only for convenience, there are no .cpp files for the
    // temporary compatibility headers.
    inline void AssertImpl(char const * filename,
                           char const * function,
                           unsigned lineNumber,
                           bool condition,
                           char const * conditionText,
                           char const * format,
                           ...)
    {
        if (!condition)
        {
            const unsigned messageBufferSize = 1024;
            std::vector<char> message(messageBufferSize, '\0');

            va_list arguments;
            va_start(arguments, format);

            vsnprintf_s(message.data(), message.size(), _TRUNCATE, format, arguments);
            va_end(arguments);

            std::stringstream sstream;
            sstream << "Assertion " << conditionText << " failed in " << function
                    << " at " << filename << ":" << lineNumber << ": " << message.data();

            throw std::exception(sstream.str().c_str());
        }
    }
}
