#pragma once

#include <iosfwd>   // Diagnostic stream declaration.
#include <memory>

#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "Temporary/Allocator.h"
#include "Temporary/NonCopyable.h"

#ifdef USE_SUITECPP_TESTS
    #include "SuiteCpp/UnitTest.h"

    typedef NativeJIT::NonCopyable TestFixtureBase;

    #define TEST_FIXTURE_START(x) \
        class x : public SuiteCpp::TestClassFactory<x>, private TestFixture \
        { \
        private:

    #define TEST_FIXTURE_END_TEST_CASES_BEGIN public:

    #define TEST_CASE(className, name) TestCase(name)
    #define TEST_CASE_F(className, name) TestCase(name)

    #define TEST_CASES_END };

    #define TestEqualCharPtrs TestEqual

    // TestAssert calls etc. inside sub-routines in SuiteCPP will fail immediately,
    // no need to explicitly verify.
    #define ASSERT_NO_FATAL_FAILURE(x) x
    #define ASSERT_NO_FATAL_FAILURES()

#else
    #include "gtest/gtest.h"

    typedef ::testing::Test TestFixtureBase;

    #define TEST_FIXTURE_START(x) \
        class x : public TestFixture \
        { \
        private:

    #define TEST_FIXTURE_END_TEST_CASES_BEGIN };

    #define TEST_CASE(className, name) TEST(className, name)
    #define TEST_CASE_F(className, name) TEST_F(className, name)

    #define TEST_CASES_END
    
    #define TestAssert(condition, ...) ASSERT_TRUE(condition)
    #define TestEqual(expected, actual, ...) ASSERT_EQ(expected, actual)
    #define TestEqualCharPtrs(expected, actual, ...) ASSERT_EQ(std::string(expected), std::string(actual))
    #define TestNotEqual(expected, actual, ...) ASSERT_NE(expected, actual)
    #define TestFail(x) FAIL() << x

    // To be called only from TEST or TEST_F methods.
    #define ASSERT_NO_FATAL_FAILURES() ASSERT_TRUE(!HasFatalFailure())
#endif

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
        // Executable buffer and general allocator for the function buffer.
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
