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
    namespace ConditionalUnitTest
    {
        TEST_FIXTURE_START(Conditional)
        TEST_FIXTURE_END_TEST_CASES_BEGIN


        // Test all comparision operators. See bug#32.

        //
        // Conditionals
        //


        TEST_F(Conditional, Conditional)
        {
            auto setup = GetSetup();

            {
                Function<uint64_t, uint64_t, uint64_t> expression(setup->GetAllocator(), setup->GetCode());

                uint64_t trueValue = 5;
                uint64_t falseValue = 6;

                auto & a = expression.Compare<JccType::JA>(expression.GetP1(), expression.GetP2());
                auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                auto function = expression.Compile(b);

                uint64_t p1 = 3;
                uint64_t p2 = 4;

                auto expected = (p1 > p2) ? trueValue : falseValue;
                auto observed = function(p1, p2);

                ASSERT_EQ(expected, observed);

                p1 = 5;
                p2 = 4;

                expected = (p1 > p2) ? trueValue : falseValue;
                observed = function(p1, p2);

                ASSERT_EQ(expected, observed);
            }
        }


        TEST_F(Conditional, IfNotZero)
        {
            auto setup = GetSetup();

            Function<uint64_t,
                        uint32_t,
                        uint64_t,
                        uint64_t> e(setup->GetAllocator(), setup->GetCode());

            auto & test = e.IfNotZero(e.GetP1(), e.GetP2(), e.GetP3());
            auto function = e.Compile(test);

            uint32_t p1 = 0;
            uint64_t p2 = 0xFFFFFFFFFFFFFFFF;
            uint64_t p3 = 0;

            auto expected = p1 != 0 ? p2 : p3;
            auto observed = function(p1, p2, p3);

            ASSERT_EQ(expected, observed);

            p1 = 1;

            expected = p1 != 0 ? p2 : p3;
            observed = function(p1, p2, p3);

            ASSERT_EQ(expected, observed);
        }


        TEST_F(Conditional, If)
        {
            auto setup = GetSetup();

            Function<uint64_t,
                        bool,
                        uint64_t,
                        uint64_t> e(setup->GetAllocator(), setup->GetCode());

            auto & test = e.If(e.GetP1(), e.GetP2(), e.GetP3());
            auto function = e.Compile(test);

            bool p1 = true;
            uint64_t p2 = 0xFFFFFFFFFFFFFFFF;
            uint64_t p3 = 0;

            auto expected = p1 ? p2 : p3;
            auto observed = function(p1, p2, p3);

            ASSERT_EQ(expected, observed);

            p1 = false;

            expected = p1 ? p2 : p3;
            observed = function(p1, p2, p3);

            ASSERT_EQ(expected, observed);
        }

        TEST_CASES_END
    }
}
