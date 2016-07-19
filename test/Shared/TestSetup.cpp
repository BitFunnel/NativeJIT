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


#include <iostream>     // For diagnostics.
#include <memory>       // For std::make_unique.

#include "TestSetup.h"


namespace NativeJIT
{
    //
    // TestFixture
    //

    std::ostream* const TestFixture::c_defaultDiagnosticsStream = nullptr;

    TestFixture::TestFixture()
        : TestFixture(c_defaultCodeAllocatorCapacity,
                      c_defaultGeneralAllocatorCapacity,
                      c_defaultDiagnosticsStream)
    {
    }


    TestFixture::TestFixture(unsigned codeAllocatorCapacity,
                             unsigned generalAllocatorCapacity,
                             std::ostream* diagnosticsStream)
        : m_codeAllocator(codeAllocatorCapacity),
          m_generalAllocator(generalAllocatorCapacity),
          m_code(m_codeAllocator, codeAllocatorCapacity),
          m_testCaseAllocator(generalAllocatorCapacity),
          m_diagnosticsStream(diagnosticsStream)
    {
        if (m_diagnosticsStream != nullptr)
        {
            m_code.EnableDiagnostics(*m_diagnosticsStream);
        }
    }


    std::unique_ptr<TestCaseSetup> TestFixture::GetSetup()
    {
        return std::make_unique<TestCaseSetup>(m_code, m_testCaseAllocator);
    }


    std::ostream* TestFixture::GetDiagnosticsStream() const
    {
        return m_diagnosticsStream;
    }


    //
    // TestCaseSetup
    //

    TestCaseSetup::TestCaseSetup(FunctionBuffer& code, Allocator& generalAllocator)
        : m_allocator(generalAllocator),
          m_code(code)
    {
    }


    TestCaseSetup::~TestCaseSetup()
    {
        m_allocator.Reset();
        m_code.Reset();
    }


    FunctionBuffer& TestCaseSetup::GetCode()
    {
        return m_code;
    }


    Allocators::IAllocator& TestCaseSetup::GetAllocator()
    {
        return m_allocator;
    }
}
