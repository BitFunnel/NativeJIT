#include "stdafx.h"


#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"
#include "TestSetup.h"


namespace NativeJIT
{
    namespace UnsignedUnitTest
    {
        TestClass(FunctionTest), private TestFixture
        {
        public:
            //
            // TODO:
            //   Other boolean operators: shift, sub, mul, etc.
            //   Signed vs unsigned
            //

            //
            // Immediate values
            //

            // TODO: cases for (signed, unsigned) x (1, 2, 4, 8), float
            TestCase(ImmediateU64)
            {
                auto setup = GetSetup();

                {
                    Function<unsigned __int64> expression(setup->GetAllocator(), setup->GetCode());

                    unsigned __int64 value = 0x1234ull;
                    auto & a = expression.Immediate(value);
                    auto function = expression.Compile(a);

                    auto expected = value;
                    auto observed = function();

                    TestAssert(observed == expected);
                }
            }


            //
            // FieldPointer
            //

#pragma pack(push, 1)
            class InnerClass
            {
            public:
                unsigned __int32 m_a;
                unsigned __int64 m_b;
            };


            class OuterClass
            {
            public:
                unsigned __int16 m_p;
                InnerClass* m_innerPointer;
                InnerClass m_innerEmbedded;
                long long m_q;
            };
#pragma pack(pop)

            TestCase(FieldPointerPrimitive)
            {
                auto setup = GetSetup();

                {
                    Function<unsigned __int64, InnerClass*> expression(setup->GetAllocator(), setup->GetCode());

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
                auto setup = GetSetup();

                {
                    Function<unsigned __int64, OuterClass**> expression(setup->GetAllocator(), setup->GetCode());

                    const unsigned pointersIndex = 4;

                    auto & outerPtrs = expression.GetP1();
                    auto & outerPtr = expression.Deref(outerPtrs, pointersIndex);
                    auto & inner = expression.FieldPointer(outerPtr, &OuterClass::m_innerEmbedded);
                    auto & innerBPtr = expression.FieldPointer(inner, &InnerClass::m_b);
                    auto & innerB = expression.Deref(innerBPtr);
                    auto function = expression.Compile(innerB);

                    OuterClass outerClass;
                    outerClass.m_innerEmbedded.m_b = 2345ull;

                    OuterClass* p1Ptrs[pointersIndex + 1] = { nullptr };
                    p1Ptrs[pointersIndex] = &outerClass;

                    OuterClass** p1 = &p1Ptrs[0];

                    auto expected = p1[pointersIndex]->m_innerEmbedded.m_b;
                    auto observed = function(p1);

                    TestAssert(observed == expected);
                }
            }


            TestCase(FieldPointerEmbeddedWithCommonSubexpression)
            {
                auto setup = GetSetup();

                {
                    Function<unsigned __int64, OuterClass*> e(setup->GetAllocator(), setup->GetCode());

                    // The inner variable has a single FieldPointer parent (the
                    // outer parameter) and it is a common parent to both innerA
                    // and innerB FieldPointers into the same object. Given that
                    // FieldPointerNode has optimization related to collapsing
                    // nested accesses to the same object, this test is meant
                    // to implicitly ensure that cache references are set correctly
                    // and that evaluations are done the expected number of times.
                    auto & inner = e.FieldPointer(e.GetP1(), &OuterClass::m_innerEmbedded);
                    auto & innerA = e.Deref(e.FieldPointer(inner, &InnerClass::m_a));
                    auto & innerB = e.Deref(e.FieldPointer(inner, &InnerClass::m_b));

                    auto & sum = e.Add(e.Cast<unsigned __int64>(innerA),
                                       innerB);

                    auto function = e.Compile(sum);

                    OuterClass outerClass;
                    outerClass.m_innerEmbedded.m_a = 10;
                    outerClass.m_innerEmbedded.m_b = 1;

                    auto expected = outerClass.m_innerEmbedded.m_a
                                    + outerClass.m_innerEmbedded.m_b;
                    auto observed = function(&outerClass);

                    TestEqual(expected, observed);
                }
            }


            //
            // Binary operations
            //

            TestCase(AddUnsignedInt32)
            {
                auto setup = GetSetup();

                {
                    Function<unsigned __int32, unsigned __int32, unsigned __int32> expression(setup->GetAllocator(), setup->GetCode());

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

            TestCase(ArrayOfIntAsPointer)
            {
                auto setup = GetSetup();

                {
                    Function<unsigned __int64, unsigned __int64*> expression(setup->GetAllocator(), setup->GetCode());

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


            TestCase(ArrayOfInt)
            {
                auto setup = GetSetup();

                {
                    struct S
                    {
                        unsigned __int64 m_array[10];
                    };

                    Function<unsigned __int64, S*> e(setup->GetAllocator(), setup->GetCode());

                    auto & arrayPtr = e.FieldPointer(e.GetP1(), &S::m_array);
                    auto & left = e.Add(arrayPtr, e.Immediate(1));
                    auto & right = e.Add(arrayPtr, e.Immediate(2));
                    auto & sum = e.Add(e.Deref(left), e.Deref(right));
                    auto function = e.Compile(sum);

                    struct S s;
                    s.m_array[1] = 456;
                    s.m_array[2] = 123000;

                    auto expected = s.m_array[1] + s.m_array[2];
                    auto observed = function(&s);

                    TestEqual(expected, observed);
                }
            }


            TestCase(ArrayOfClass)
            {
                auto setup = GetSetup();

                {
                    Function<__int64, OuterClass*, unsigned __int64> expression(setup->GetAllocator(), setup->GetCode());

                    auto & a = expression.Add(expression.GetP1(), expression.GetP2());
                    auto & b = expression.FieldPointer(a, &OuterClass::m_q);
                    auto & c = expression.Deref(b);
                    auto function = expression.Compile(c);

                    OuterClass array[10];
                    OuterClass* p1 = array;
                    unsigned __int64 p2 = 3ull;

                    p1[p2].m_q = 0x0123456789ABCDEF;

                    auto expected = p1[p2].m_q;
                    auto observed = function(p1, p2);

                    TestAssert(observed == expected);
                }
            }


            //
            // Common sub expressions
            //

            TestCase(CommonSubExpressions)
            {
                auto setup = GetSetup();

                {
                    Function<__int64, __int64, __int64> expression(setup->GetAllocator(), setup->GetCode());

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
        };
    }
}
