#include "stdafx.h"

#include <cstdarg>
#include <stdexcept>
#include <sstream>
#include <vector>

#include "Temporary/Assert.h"


namespace NativeJIT
{
    void LogThrowImpl(char const * filename,
                      char const * function,
                      unsigned lineNumber,
                      char const * title,
                      char const * format,
                      ...)
    {
        const unsigned messageBufferSize = 1024;
        char message[messageBufferSize];

        va_list arguments;
        va_start(arguments, format);

#ifdef NATIVEJIT_PLATFORM_WINDOWS
        vsnprintf_s(message, _TRUNCATE, format, arguments);
#else
        vsnprintf(&message[0], messageBufferSize, format, arguments);
#endif

        va_end(arguments);

        std::stringstream sstream;
        sstream << "Assertion " << title << " failed in " << function
                << " at " << filename << ":" << lineNumber << ": " << message;

        throw std::runtime_error(sstream.str().c_str());
    }
}
