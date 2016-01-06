#include "stdafx.h"

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
          m_code(m_codeAllocator, codeAllocatorCapacity, m_generalAllocator),
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
