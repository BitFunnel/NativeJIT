#include "stdafx.h"

#include <memory>

#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"

#include "CallNode.h"

namespace NativeJIT
{
    namespace FunctionUnitTest
    {
        TestClass(FunctionTest)
        {
        public:
            FunctionTest()
                : m_allocator(5000),
                  m_executionBuffer(5000)
            {
                m_code.reset(new FunctionBuffer(m_executionBuffer, 5000, 10, 10, 3, 0, false));
            }


            //
            // TODO:
            //   Test register save/restore across call.
            //   Test call register indirect.
            //   Test throws from within generated code.


            //
            // JIT Functions with 0, 1, 2, 3, and 4 parameters.
            //

            TestCase(FunctionZeroParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<__int64> expression(m_allocator, *m_code);

                    __int64 expected = 1234ll;
                    auto & a = expression.Immediate(expected);
                    auto function = expression.Compile(a);

                    auto observed = function();

                    TestAssert(observed == expected);
                }
            }


            TestCase(FunctionOneParameter)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<__int64, __int64> expression(m_allocator, *m_code);

                    auto & a = expression.GetP1();
                    auto function = expression.Compile(a);

                    __int64 p1 = 1234ll;

                    auto expected = p1;
                    auto observed = function(p1);

                    TestAssert(observed == expected);
                }
            }


            TestCase(FunctionTwoParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<__int64, __int64, __int64> expression(m_allocator, *m_code);

                    auto & a = expression.Add(expression.GetP1(), expression.GetP2());
                    auto function = expression.Compile(a);

                    __int64 p1 = 12340000ll;
                    __int64 p2 = 5678ll;

                    auto expected = p1 + p2;
                    auto observed = function(p1, p2);

                    TestAssert(observed == expected);
                }
            }


            TestCase(FunctionThreeParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<int, int, int, int> expression(m_allocator, *m_code);

                    auto & a = expression.Sub(expression.GetP2(), expression.GetP1());
                    auto & b = expression.Add(expression.GetP3(), a);
                    auto function = expression.Compile(b);

                    int p1 = 0x12340000;
                    int p2 = 0x5678;
                    int p3 = 0x11111111;

                    auto expected = p2 - p1 + p3;
                    auto observed = function(p1, p2, p3);

                    TestAssert(observed == expected);
                }
            }


            TestCase(FunctionFourParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<char, char, char, char, char> expression(m_allocator, *m_code);

                    auto & a = expression.Sub(expression.GetP1(), expression.GetP2());
                    auto & b = expression.Sub(expression.GetP3(), expression.GetP4());
                    auto & c = expression.Sub(a, b);
                    auto function = expression.Compile(c);

                    char p1 = 100;
                    char p2 = 50;
                    char p3 = 10;
                    char p4 = 5;

                    auto expected = (p1 - p2) - (p3 - p4);
                    auto observed = function(p1, p2, p3, p4);

                    TestAssert(observed == expected);
                }
            }


            //
            // Calling C functions with 0, 1, 2, 3, and 4 parameters.
            //

            static int s_sampleFunctionCalls;
            static int s_intParameter1;
            static int s_intParameter2;

            static char s_charParameter1;

            static __int64 s_int64Parameter3;

            static bool s_boolParameter4;


            static int SampleFunction0()
            {
                ++FunctionTest::s_sampleFunctionCalls;
                return 12345;
            }


            static int SampleFunction1(char p1)
            {
                TestAssert(p1 == s_charParameter1);
                ++FunctionTest::s_sampleFunctionCalls;
                return p1 + 3;
            }


            static int SampleFunction2(int p1, int p2)
            {
                TestAssert(p1 == s_intParameter1);
                TestAssert(p2 == s_intParameter2);
                ++FunctionTest::s_sampleFunctionCalls;
                return p1 + p2;
            }


            static __int64 SampleFunction3(char p1, int p2, __int64 p3)
            {
                TestAssert(p1 == s_charParameter1);
                TestAssert(p2 == s_intParameter2);
                TestAssert(p3 == s_int64Parameter3);
                ++FunctionTest::s_sampleFunctionCalls;
                return p1 + p2 + p3;
            }


            static __int64 SampleFunction3(char p1, int p2, __int64 p3, bool p4)
            {
                TestAssert(p1 == s_charParameter1);
                TestAssert(p2 == s_intParameter2);
                TestAssert(p3 == s_int64Parameter3);
                TestAssert(p4 == s_boolParameter4);
                ++FunctionTest::s_sampleFunctionCalls;
                return p1 + p2 + p3 + (p4 ? 123 : 456);
            }


            TestCase(CallZeroParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<int> expression(m_allocator, *m_code);

                    typedef int (*F)();
                    auto & sampleFunction = expression.Immediate<F>(SampleFunction0);
                    auto & a = expression.Call(sampleFunction);
                    auto function = expression.Compile(a);

                    auto expected = SampleFunction0();

                    FunctionTest::s_sampleFunctionCalls = 0;
                    auto observed = function();

                    TestAssert(observed == expected);
                    TestAssert(FunctionTest::s_sampleFunctionCalls == 1);
                }
            }


            TestCase(CallOneParameter)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<int, char> expression(m_allocator, *m_code);

                    typedef int (*F)(char);
                    auto & sampleFunction = expression.Immediate<F>(SampleFunction1);
                    auto & a = expression.Call(sampleFunction, expression.GetP1());
                    auto function = expression.Compile(a);

                    char& p1 = s_charParameter1;;

                    p1 = 0x73;

                    auto expected = SampleFunction1(p1);

                    FunctionTest::s_sampleFunctionCalls = 0;
                    auto observed = function(p1);

                    TestAssert(observed == expected);
                    TestAssert(FunctionTest::s_sampleFunctionCalls == 1);
                }
            }


            TestCase(CallTwoParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<int, int, int> expression(m_allocator, *m_code);

                    typedef int (*F)(int, int);
                    auto & sampleFunction = expression.Immediate<F>(SampleFunction2);
                    auto & a = expression.Call(sampleFunction, expression.GetP2(), expression.GetP1());
                    auto function = expression.Compile(a);

                    int& p1 = s_intParameter2;
                    int& p2 = s_intParameter1;

                    p1 = 12340000ll;
                    p2 = 5678ll;
                    p1 = 0x73;

                    auto expected = SampleFunction2(p2, p1);

                    FunctionTest::s_sampleFunctionCalls = 0;
                    auto observed = function(p1, p2);

                    TestAssert(observed == expected);
                    TestAssert(FunctionTest::s_sampleFunctionCalls == 1);
                }
            }


            TestCase(CallThreeParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<__int64, __int64, int, char> expression(m_allocator, *m_code);

                    typedef __int64 (*F)(char, int, __int64);
                    auto & sampleFunction = expression.Immediate<F>(SampleFunction3);
                    auto & a = expression.Call(sampleFunction,
                                               expression.GetP3(),
                                               expression.GetP2(),
                                               expression.GetP1());
                    auto function = expression.Compile(a);

                    char& p1 = s_charParameter1;
                    int& p2 = s_intParameter2;
                    __int64& p3 = s_int64Parameter3;

                    p1 = 0x73;
                    p2 = 5678;
                    p3 = 12340000ll;

                    auto expected = SampleFunction3(p1, p2, p3);

                    FunctionTest::s_sampleFunctionCalls = 0;
                    auto observed = function(p3, p2, p1);

                    TestAssert(observed == expected);
                    TestAssert(FunctionTest::s_sampleFunctionCalls == 1);
                }
            }


            TestCase(CallFourParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<__int64, bool, __int64, int, char> expression(m_allocator, *m_code);

                    typedef __int64 (*F)(char, int, __int64, bool);
                    auto & sampleFunction = expression.Immediate<F>(SampleFunction3);
                    auto & a = expression.Call(sampleFunction,
                                               expression.GetP4(),
                                               expression.GetP3(),
                                               expression.GetP2(),
                                               expression.GetP1());
                    auto function = expression.Compile(a);

                    char& p1 = s_charParameter1;
                    int& p2 = s_intParameter2;
                    __int64& p3 = s_int64Parameter3;
                    bool& p4 = s_boolParameter4;

                    p1 = 0x73;
                    p2 = 5678;
                    p3 = 12340000ll;
                    p4 = true;

                    auto expected = SampleFunction3(p1, p2, p3, p4);

                    FunctionTest::s_sampleFunctionCalls = 0;
                    auto observed = function(p4, p3, p2, p1);

                    TestAssert(observed == expected);
                    TestAssert(FunctionTest::s_sampleFunctionCalls == 1);
                }
            }


        private:
            Allocator m_allocator;
            ExecutionBuffer m_executionBuffer;
            std::unique_ptr<FunctionBuffer> m_code;
        };

        int FunctionTest::s_sampleFunctionCalls;

        int FunctionTest::s_intParameter1;
        int FunctionTest::s_intParameter2;

        char FunctionTest::s_charParameter1;

        __int64 FunctionTest::s_int64Parameter3;

        bool FunctionTest::s_boolParameter4;
    }
}
