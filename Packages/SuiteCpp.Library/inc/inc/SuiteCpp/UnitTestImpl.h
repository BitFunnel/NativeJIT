//*****************************************************************************
//
// Calling any of the following code explicitly is not supported.
// Please use the macros defined in UnitTest.h
//
//*****************************************************************************

#pragma once

#include <cstdarg>
#include <sstream>

#include "SuiteCpp\AssertFailureException.h"


namespace SuiteCpp
{
    const unsigned c_messageBufferSize = 1024;

    class TestClassFactoryBase
    {
        _declspec(dllexport) virtual void* _m_SuiteCpp_Create() = 0;
        _declspec(dllexport) virtual void _m_SuiteCpp_Destroy(void* p) = 0;
    };


    template<typename T>
    class TestClassFactory : public TestClassFactoryBase
    {
        __declspec(dllexport) void* _m_SuiteCpp_Create() { return new T; };
        __declspec(dllexport) void _m_SuiteCpp_Destroy(void* p) { delete static_cast<T*>(p); };
    };


    inline void GetMessage(std::ostream& message,
                            const char* condition,
                            const char* format,
                            va_list args)
    {
        if (condition != nullptr)
        {
            message << condition << " ";
        }
        if (format != nullptr)
        {
            char assertMessage[c_messageBufferSize];
            vsprintf_s(assertMessage, sizeof(assertMessage), format, args);
            message << "[" << assertMessage << "]";
        }
    }


    inline void ThrowAssertFailure(const char* file,
                                    unsigned line,
                                    const char* assertName,
                                    const char* message)
    {
        std::stringstream error;
        error << file << "(" << line << "): " << assertName;
        if (message != nullptr)
        {
            error << ": " << message;
        }
        throw AssertFailureException(error.str());
    }


//*****************************************************************************
//
// TestFail
//
//*****************************************************************************
    inline void TestFailImpl(const char* file, unsigned line, const char* format, ...)
    {
        std::stringstream message;
        va_list args;
        va_start(args, format);
        GetMessage(message, nullptr, format, args);
        va_end(args);
        ThrowAssertFailure(file, line, "TestFail", message.str().c_str());
    }


    inline void TestFailImpl(const char* file, unsigned line)
    {
        TestFailImpl(file, line, nullptr);
    }


//*****************************************************************************
//
// TestAssert
//
//*****************************************************************************
    inline void TestAssertImpl(const char* file, unsigned line, bool condition, const char* expression, const char* format, ...)
    {
        if (!condition)
        {
            std::stringstream ss_condition;
            ss_condition << "!(" << expression << ")";
            std::stringstream message;
            va_list args;
            va_start(args, format);
            GetMessage(message, ss_condition.str().c_str(), format, args);
            va_end(args);
            ThrowAssertFailure(file, line, "TestAssert", message.str().c_str());
        }
    }


    inline void TestAssertImpl(const char* file, unsigned line, bool condition, const char* expression)
    {
        TestAssertImpl(file, line, condition, expression, nullptr);
    }


//*****************************************************************************
//
// TestEqual
//
//*****************************************************************************
    template <typename T1, typename T2>
    inline void TestEqualImpl(const char* file, unsigned line, T1 expected, T2 actual, const char* expectedName, const char* actualName, const char* format, ...)
    {
        if (expected != actual)
        {
            std::stringstream condition;
            condition << expectedName << "<" << expected << "> != " << actualName << "<" << actual << ">";
            std::stringstream message;
            va_list args;
            va_start(args, format);
            GetMessage(message, condition.str().c_str(), format, args);
            va_end(args);
            ThrowAssertFailure(file, line, "TestEqual", message.str().c_str());
        }
    }


    template <>
    inline void TestEqualImpl<const char*, const char*>(const char* file, unsigned line, const char* expected, const char* actual, const char* expectedName, const char* actualName, const char* format, ...)
    {
        if (strcmp(expected, actual) != 0)
        {
            std::stringstream condition;
            condition << expectedName << "<" << expected << "> != " << actualName << "<" << actual << ">";
            std::stringstream message;
            va_list args;
            va_start(args, format);
            GetMessage(message, condition.str().c_str(), format, args);
            va_end(args);
            ThrowAssertFailure(file, line, "TestEqual", message.str().c_str());
        }
    }


    template <typename T1, typename T2>
    inline void TestEqualImpl(const char* file, unsigned line, T1 expected, T2 actual, const char* expectedName, const char* actualName)
    {
        TestEqualImpl(file, line, expected, actual, expectedName, actualName, nullptr);
    }


//*****************************************************************************
//
// TestNotEqual
//
//*****************************************************************************
    template <typename T1, typename T2>
    inline void TestNotEqualImpl(const char* file, unsigned line, T1 notExpected, T2 actual, const char* notExpectedName, const char* actualName, const char* format, ...)
    {
        if (notExpected == actual)
        {
            std::stringstream condition;
            condition << notExpectedName << "<" << notExpected << "> == " << actualName << "<" << actual << ">";
            std::stringstream message;
            va_list args;
            va_start(args, format);
            GetMessage(message, condition.str().c_str(), format, args);
            va_end(args);
            ThrowAssertFailure(file, line, "TestNotEqual", message.str().c_str());
        }
    }


    template <>
    inline void TestNotEqualImpl<const char*, const char*> (const char* file, unsigned line, const char* notExpected, const char* actual, const char* notExpectedName, const char* actualName, const char* format, ...)
    {
        if (strcmp(notExpected, actual) == 0)
        {
            std::stringstream condition;
            condition << notExpectedName << "<" << notExpected << "> == " << actualName << "<" << actual << ">";
            std::stringstream message;
            va_list args;
            va_start(args, format);
            GetMessage(message, condition.str().c_str(), format, args);
            va_end(args);
            ThrowAssertFailure(file, line, "TestNotEqual", message.str().c_str());
        }
    }


    template <typename T1, typename T2>
    inline void TestNotEqualImpl(const char* file, unsigned line, T1 notExpected, T2 actual, const char* notExpectedName, const char* actualName)
    {
        TestNotEqualImpl(file, line, notExpected, actual, notExpectedName, actualName, nullptr);
    }


//*****************************************************************************
//
// TestNull
//
//*****************************************************************************
    template <typename T>
    inline void TestNullImpl(const char* file, unsigned line, T* pointer, const char* pointerName, const char* format, ...)
    {
        if (pointer != nullptr)
        {
            std::stringstream condition;
            condition << pointerName << " != nullptr";
            std::stringstream message;
            va_list args;
            va_start(args, format);
            GetMessage(message, condition.str().c_str(), format, args);
            va_end(args);
            ThrowAssertFailure(file, line, "TestNull", message.str().c_str());
        }
    }


    template <typename T>
    inline void TestNullImpl(const char* file, unsigned line, T* pointer, const char* pointerName)
    {
        TestNullImpl(file, line, pointer, pointerName, nullptr);
    }


//*****************************************************************************
//
// TestNotNull
//
//*****************************************************************************
    template <typename T>
    inline void TestNotNullImpl(const char* file, unsigned line, T* pointer, const char* pointerName, const char* format, ...)
    {
        if (pointer == nullptr)
        {
            std::stringstream condition;
            condition << pointerName << " == nullptr";
            std::stringstream message;
            va_list args;
            va_start(args, format);
            GetMessage(message, condition.str().c_str(), format, args);
            va_end(args);
            ThrowAssertFailure(file, line, "TestNotNull", message.str().c_str());
        }
    }


    template <typename T>
    inline void TestNotNullImpl(const char* file, unsigned line, T* pointer, const char* pointerName)
    {
        TestNotNullImpl(file, line, pointer, pointerName, nullptr);
    }
}