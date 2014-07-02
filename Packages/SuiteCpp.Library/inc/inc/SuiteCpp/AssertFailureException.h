#pragma once

#include <stdexcept>
#include <string>

namespace SuiteCpp
{
    class AssertFailureException : public std::runtime_error
    {
    public:
        AssertFailureException(const std::string& message)
            : runtime_error(message)
        { }
    };
}