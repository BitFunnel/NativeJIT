#pragma once

#include <stdexcept>


namespace NativeJIT
{
    inline void Assert(bool condition, char const * message)
    {
        if (!condition)
        {
            throw std::exception(message);
        }
    }
}