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
            //   Register spilling
            //   Common sub-expressions
            //   Calling out to C++
            //   Storage class
            //   Array of primitives
            //   Other boolean operators: shift, sub, mul, etc.
            //   Floating point
            //   Signed vs unsigned
            //   Richer integration examples (e.g. JitExample1)
            //


            //
            // Immediate values
            //

            // TODO: cases for (signed, unsigned) x (1, 2, 4, 8), float
            TestCase(ImmediateU64)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<unsigned __int64> expression(m_allocator, *m_code);

                    unsigned __int64 value = 0x1234ull;
                    auto & a = expression.Immediate(value);
                    auto function = expression.Compile(a);

                    auto expected = value;
                    auto observed = function();

                    TestAssert(observed == expected);
                }
            }


            //
            // Functions
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
            // FieldPointer
            //

            class InnerClass
            {
            public:
                unsigned __int64 m_a;
                unsigned __int64 m_b;
            };


            class OuterClass
            {
            public:
                long long m_p;
                InnerClass* m_innerPointer;
                InnerClass m_innerEmbedded;
                long long m_q;
            };


            TestCase(FieldPointerPrimitive)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<unsigned __int64, InnerClass*> expression(m_allocator, *m_code);

                    auto & a = expression.GetP1();
                    auto & b = expression.FieldPointer(a, &InnerClass::m_b);
                    auto & c = expression.Deref(b);
                    auto function = expression.Compile(c);

                    InnerClass innerClass;
                    innerClass.m_b = 1234ull;
                    InnerClass* p1 = &innerClass;

                    auto expected = p1->m_b;
                    auto observed = function(p1);

                    TestAssert(observed == expected);
                }
            }


            TestCase(FieldPointerEmbedded)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<unsigned __int64, OuterClass*> expression(m_allocator, *m_code);

                    auto & a = expression.GetP1();
                    auto & b = expression.FieldPointer(a, &OuterClass::m_innerEmbedded);
                    auto & c = expression.FieldPointer(b, &InnerClass::m_b);
                    auto & d = expression.Deref(c);
                    auto function = expression.Compile(d);

                    OuterClass outerClass;
                    outerClass.m_innerEmbedded.m_b = 1234ull;
                    OuterClass* p1 = &outerClass;

                    auto expected = p1->m_innerEmbedded.m_b;
                    auto observed = function(p1);

                    TestAssert(observed == expected);
                }
            }


            //
            // Binary operations
            //

            TestCase(AddUnsignedInt32)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<unsigned __int32, unsigned __int32, unsigned __int32> expression(m_allocator, *m_code);

                    auto & a = expression.Add(expression.GetP2(), expression.GetP1());
                    auto function = expression.Compile(a);

                    unsigned __int32 p1 = 12340000ul;
                    unsigned __int32 p2 = 5678ul;

                    auto expected = p1 + p2;
                    auto observed = function(p1, p2);

                    TestAssert(observed == expected);
                }
            }


            //
            // Array indexing
            //

            TestCase(ArrayOfInt)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<unsigned __int64, unsigned __int64*> expression(m_allocator, *m_code);

                    auto & a = expression.Add(expression.GetP1(), expression.Immediate<unsigned __int64>(1ull));
                    auto & b = expression.Add(expression.GetP1(), expression.Immediate<unsigned __int64>(2ull));
                    auto & c = expression.Add(expression.Deref(a), expression.Deref(b));
                    auto function = expression.Compile(c);

                    unsigned __int64 array[10];
                    array[1] = 456;
                    array[2] = 123000;

                    unsigned __int64 * p1 = array;

                    auto expected = array[1] + array[2];
                    auto observed = function(p1);

                    TestAssert(observed == expected);
                }
            }


            TestCase(ArrayOfClass)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<__int64, OuterClass*, unsigned __int64> expression(m_allocator, *m_code);

                    auto & a = expression.Add(expression.GetP1(), expression.GetP2());
                    auto & b = expression.FieldPointer(a, &OuterClass::m_q);
                    auto & c = expression.Deref(b);
                    auto function = expression.Compile(c);

                    OuterClass array[10];
                    OuterClass* p1 = array;
                    unsigned __int64 p2 = 3ull;

                    // TODO: First set p1[p2].m_q

                    auto expected = p1[p2].m_q;
                    auto observed = function(p1, p2);

                    TestAssert(observed == expected);
                }
            }


            //
            // Conditionals
            //

            TestCase(Conditional)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<unsigned __int64, unsigned __int64, unsigned __int64> expression(m_allocator, *m_code);

                    unsigned __int64 trueValue = 5;
                    unsigned __int64 falseValue = 6;

                    auto & a = expression.GreaterThan(expression.GetP1(), expression.GetP2());
                    auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                    auto function = expression.Compile(b);

                    unsigned __int64 p1 = 3;
                    unsigned __int64 p2 = 4;

                    auto expected = (p1 > p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    TestAssert(observed == expected);

                    p1 = 5;
                    p2 = 4;

                    expected = (p1 > p2) ? trueValue : falseValue;
                    observed = function(p1, p2);

                    // TODO: Enable after implementing conditional jumps.
                    // TestAssert(observed == expected);
                }
            }


            //
            // Common sub expressions
            //

            TestCase(CommonSubExpressions)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<__int64, __int64, __int64> expression(m_allocator, *m_code);

                    // This tree has three common subexpressions: P1, P2, and node "a".
                    // Each common subexpression is referenced twice.

                    auto & a = expression.Add(expression.GetP1(), expression.GetP2());
                    auto & b = expression.Add(a, expression.GetP1());

                    // Perform this add after 2nd and last use of P1 to see if RCX is recycled.
                    auto & c = expression.Add(expression.Immediate(1ll), expression.Immediate(2ll));

                    auto & d = expression.Add(b, c);
                    auto & e = expression.Add(d, expression.GetP2());

                    // Perform this add after 2nd and last use of P2 to see if RDX is recycled.
                    auto & f = expression.Add(expression.Immediate(3ll), expression.Immediate(4ll));

                    auto & g = expression.Add(e, f);
                    auto & h = expression.Add(g, a);

                    // Perform this add after 2nd and last use of a to see if a's register is recycled.
                    auto & i = expression.Add(expression.Immediate(5ll), expression.Immediate(6ll));

                    auto & j = expression.Add(h, i);
                    auto function = expression.Compile(j);

                    __int64 p1 = 1ll;
                    __int64 p2 = 10ll;

                    auto expectedA = p1 + p2;
                    auto expectedB = expectedA + p1;
                    auto expectedC = 1ll + 2ll;
                    auto expectedD = expectedB + expectedC;
                    auto expectedE = expectedD + p2;
                    auto expectedF = 3ll + 4ll;
                    auto expectedG = expectedE + expectedF;
                    auto expectedH = expectedG + expectedA;
                    auto expectedI = 5ll + 6ll;

                    auto expected = expectedH + expectedI;

                    auto observed = function(p1, p2);

                    TestAssert(observed == expected);
                }
            }


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

            // TODO: Test register save/restore across call.
            // TODO: Test call register indirect.
            // TODO: Test throws from within generated code.


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
