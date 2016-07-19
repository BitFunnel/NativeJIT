// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#pragma once

#include <iosfwd>   // Diagnostic stream declaration.
#include <memory>

#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "Temporary/Allocator.h"
#include "Temporary/NonCopyable.h"

#include "gtest/gtest.h"

typedef ::testing::Test TestFixtureBase;

#define TEST_FIXTURE_START(x)        \
  class x : public TestFixture       \
  {                                  \
  private:

#define TEST_FIXTURE_END_TEST_CASES_BEGIN };
#define TEST_CASES_END

namespace NativeJIT
{
    class TestCaseSetup;

    class TestFixture : public TestFixtureBase
    {
    public:
        static const unsigned c_defaultCodeAllocatorCapacity = 8192;
        static const unsigned c_defaultGeneralAllocatorCapacity = 8192;
        static std::ostream* const c_defaultDiagnosticsStream;

        TestFixture();
        TestFixture(unsigned codeAllocatorCapacity,
                    unsigned generalAllocatorCapacity,
                    std::ostream* diagnostic);

        std::unique_ptr<TestCaseSetup> GetSetup();

        std::ostream* GetDiagnosticsStream() const;

    private:
        // Executable buffer and general allocator for the function.
        ExecutionBuffer m_codeAllocator;
        Allocator m_generalAllocator;

        // Function buffer and allocator reset after each test run inside the fixture.
        FunctionBuffer m_code;
        Allocator m_testCaseAllocator;

        std::ostream* m_diagnosticsStream;
    };


    // A setup for a test case inside a test fixture. Features a function buffer
    // and an allocator that will both get reset after the test case ends.
    class TestCaseSetup final : private NonCopyable
    {
    public:
        TestCaseSetup(FunctionBuffer& code, Allocator& generalAllocator);
        ~TestCaseSetup();

        FunctionBuffer& GetCode();
        Allocators::IAllocator& GetAllocator();

    private:
        Allocator& m_allocator;
        FunctionBuffer& m_code;
    };
}
