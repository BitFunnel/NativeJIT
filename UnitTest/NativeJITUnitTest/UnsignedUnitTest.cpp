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



#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/Function.h"
#include "Temporary/Allocator.h"
#include "TestSetup.h"


namespace NativeJIT
{
    namespace UnsignedUnitTest
    {
        TEST_FIXTURE_START(Unsigned)
        TEST_FIXTURE_END_TEST_CASES_BEGIN

        //
        // Immediate values
        //

        TEST_F(Unsigned, ImmediateU64)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t value = 0x1234ull;
                auto & a = expression.Immediate(value);
                auto function = expression.Compile(a);

                auto expected = value;
                auto observed = function();

                ASSERT_EQ(observed, expected);
            }
        }


        //
        // FieldPointer
        //

#pragma pack(push, 1)
        class InnerClass
        {
        public:
            uint32_t m_a;
            uint64_t m_b;
        };


        class OuterClass
        {
        public:
            uint16_t m_p;
            InnerClass* m_innerPointer;
            InnerClass m_innerEmbedded;
            int64_t m_q;
        };
#pragma pack(pop)

        TEST_F(Unsigned, FieldPointerPrimitive)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, InnerClass*> expression(setup->GetAllocator(), setup->GetCode());

                auto & a = expression.GetP1();
                auto & b = expression.FieldPointer(a, &InnerClass::m_b);
                auto & c = expression.Deref(b);
                auto function = expression.Compile(c);

                InnerClass innerClass;
                innerClass.m_b = 1234ull;
                InnerClass* p1 = &innerClass;

                auto expected = p1->m_b;
                auto observed = function(p1);

                ASSERT_EQ(observed, expected);
            }
        }


        TEST_F(Unsigned, FieldPointerEmbedded)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, OuterClass**> expression(setup->GetAllocator(), setup->GetCode());

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

                ASSERT_EQ(observed, expected);
            }
        }


        TEST_F(Unsigned, FieldPointerEmbeddedWithCommonSubexpression)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, OuterClass*> e(setup->GetAllocator(), setup->GetCode());

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

                auto & sum = e.Add(e.Cast<uint64_t>(innerA),
                                    innerB);

                auto function = e.Compile(sum);

                OuterClass outerClass;
                outerClass.m_innerEmbedded.m_a = 10;
                outerClass.m_innerEmbedded.m_b = 1;

                auto expected = outerClass.m_innerEmbedded.m_a
                                + outerClass.m_innerEmbedded.m_b;
                auto observed = function(&outerClass);

                ASSERT_EQ(expected, observed);
            }
        }


        //
        // Binary operations
        //

        TEST_F(Unsigned, AddUnsignedInt32)
        {
            auto setup = GetSetup();

            {
                Function<uint32_t, uint32_t, uint32_t> expression(setup->GetAllocator(), setup->GetCode());

                auto & a = expression.Add(expression.GetP2(), expression.GetP1());
                auto function = expression.Compile(a);

                uint32_t p1 = 12340000ul;
                uint32_t p2 = 5678ul;

                auto expected = p1 + p2;
                auto observed = function(p1, p2);

                ASSERT_EQ(observed, expected);
            }
        }

        TEST_F(Unsigned, MulUnsignedInt32)
        {
            auto setup = GetSetup();

            {
                Function<uint32_t, uint32_t, uint32_t> expression(setup->GetAllocator(), setup->GetCode());

                auto & a = expression.Mul(expression.GetP2(), expression.GetP1());
                auto function = expression.Compile(a);

                uint32_t p1 = 12340000ul;
                uint32_t p2 = 5678ul;

                auto expected = p1 * p2;
                auto observed = function(p1, p2);

                ASSERT_EQ(observed, expected);
            }
        }

        TEST_F(Unsigned, RolUnsignedInt32)
        {
            auto setup = GetSetup();

            {
                Function<uint32_t, uint32_t, uint32_t> expression(setup->GetAllocator(), setup->GetCode());

                uint8_t p2 = 1ul;

                auto & a = expression.Rol(expression.GetP1(), p2);
                auto function = expression.Compile(a);

                uint32_t p1 = 0x80000001;

                uint32_t expected = 0x00000003;
                uint32_t observed = function(p1, p2);

                ASSERT_EQ(observed, expected);
            }
        }

        TEST_F(Unsigned, SubUnsignedInt32)
        {
            auto setup = GetSetup();

            {
                Function<uint32_t, uint32_t, uint32_t> expression(setup->GetAllocator(), setup->GetCode());

                auto & a = expression.Sub(expression.GetP2(), expression.GetP1());
                auto function = expression.Compile(a);

                uint32_t p1 = 12340000ul;
                uint32_t p2 = 5678ul;

                auto expected = p2 - p1;
                auto observed = function(p1, p2);

                ASSERT_EQ(observed, expected);
            }
        }

        TEST_F(Unsigned, ShlUnsignedInt32)
        {
            auto setup = GetSetup();

            {
                Function<uint32_t, uint32_t, uint32_t> expression(setup->GetAllocator(), setup->GetCode());

                uint8_t p2 = 3ul;

                auto & a = expression.Shl(expression.GetP1(), p2);
                auto function = expression.Compile(a);

                uint32_t p1 = 1ul;

                auto expected = p1 << p2;
                auto observed = function(p1, p2);

                ASSERT_EQ(observed, expected);
            }
        }

        TEST_F(Unsigned, ShrUnsignedInt32)
        {
            auto setup = GetSetup();

            {
                Function<uint32_t, uint32_t, uint32_t> expression(setup->GetAllocator(), setup->GetCode());

                uint8_t p2 = 3ul;

                auto & a = expression.Shr(expression.GetP1(), p2);
                auto function = expression.Compile(a);

                uint32_t p1 = 0xffffffff;

                auto expected = p1 >> p2;
                auto observed = function(p1, p2);

                ASSERT_EQ(observed, expected);
            }
        }


        //
        // Ternary operations
        //

        TEST_F(Unsigned, ShldUnsignedInt32)
        {
            auto setup = GetSetup();

            {
                Function<uint32_t, uint32_t, uint32_t, uint32_t> expression(setup->GetAllocator(), setup->GetCode());

                uint8_t p3 = 4ul;

                auto & a = expression.Shld(expression.GetP2(), expression.GetP1(), p3);
                auto function = expression.Compile(a);

                uint32_t p1 = 1ul;
                uint32_t p2 = 3ul;

                auto expected = p2 << p3;
                auto observed = function(p1, p2, p3);

                ASSERT_EQ(observed, expected);
            }
        }

        TEST_F(Unsigned, ShldPairUnsignedInt32)
        {
            auto setup = GetSetup();

            {
                Function<uint32_t, uint32_t, uint32_t, uint32_t> expression(setup->GetAllocator(), setup->GetCode());

                uint8_t p3 = 4ul;

                auto & a = expression.Shld(expression.GetP2(), expression.GetP1(), p3);
                auto function = expression.Compile(a);

                uint32_t p1 = 0xaaaaaaaa;
                uint32_t p2 = 3ul;

                auto expected = p2 << p3 | (p1 & 0xf);
                auto observed = function(p1, p2, p3);

                ASSERT_EQ(observed, expected);
            }
        }


        //
        // Array indexing
        //

        TEST_F(Unsigned, ArrayOfIntAsPointer)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t*> expression(setup->GetAllocator(), setup->GetCode());

                auto & a = expression.Add(expression.GetP1(), expression.Immediate<uint64_t>(1ull));
                auto & b = expression.Add(expression.GetP1(), expression.Immediate<uint64_t>(2ull));
                auto & c = expression.Add(expression.Deref(a), expression.Deref(b));
                auto function = expression.Compile(c);

                uint64_t array[10];
                array[1] = 456;
                array[2] = 123000;

                uint64_t * p1 = array;

                auto expected = array[1] + array[2];
                auto observed = function(p1);

                ASSERT_EQ(observed, expected);
            }
        }


        TEST_F(Unsigned, ArrayOfInt)
        {
            auto setup = GetSetup();

            {
                struct S
                {
                    uint64_t m_array[10];
                };

                Function<uint64_t, S*> e(setup->GetAllocator(), setup->GetCode());

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

                ASSERT_EQ(expected, observed);
            }
        }


        TEST_F(Unsigned, ArrayOfClass)
        {
            auto setup = GetSetup();

            {
                Function<int64_t, OuterClass*, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                auto & a = expression.Add(expression.GetP1(), expression.GetP2());
                auto & b = expression.FieldPointer(a, &OuterClass::m_q);
                auto & c = expression.Deref(b);
                auto function = expression.Compile(c);

                OuterClass array[10];
                OuterClass* p1 = array;
                uint64_t p2 = 3ull;

                p1[p2].m_q = 0x0123456789ABCDEF;

                auto expected = p1[p2].m_q;
                auto observed = function(p1, p2);

                ASSERT_EQ(observed, expected);
            }
        }


        //
        // Common sub expressions
        //

        TEST_F(Unsigned, CommonSubExpressions)
        {
            auto setup = GetSetup();

            {
                Function<int64_t, int64_t, int64_t> expression(setup->GetAllocator(), setup->GetCode());

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

                int64_t p1 = 1ll;
                int64_t p2 = 10ll;

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

                ASSERT_EQ(observed, expected);
            }
        }

        TEST_CASES_END
    }
}
