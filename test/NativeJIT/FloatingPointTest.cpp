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



#include "NativeJIT/Function.h"
#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "Temporary/Allocator.h"
#include "TestSetup.h"


namespace NativeJIT
{
    namespace FloatingPointUnitTest
    {
        TEST_FIXTURE_START(FloatingPoint)
        TEST_FIXTURE_END_TEST_CASES_BEGIN

        TEST_F(FloatingPoint, ImmediateDouble)
        {
            auto setup = GetSetup();

            {
                Function<double> expression(setup->GetAllocator(), setup->GetCode());

                double value = 123.456;
                auto & a = expression.Immediate(value);

                auto function = expression.Compile(a);

                auto expected = value;
                auto observed = function();

                ASSERT_EQ(observed, expected);
            }
        }


        TEST_F(FloatingPoint, ImmediateFloat)
        {
            auto setup = GetSetup();

            {
                Function<float> expression(setup->GetAllocator(), setup->GetCode());

                float value = 123.456f;
                auto & a = expression.Immediate(value);

                auto function = expression.Compile(a);

                auto expected = value;
                auto observed = function();

                ASSERT_EQ(observed, expected);
            }
        }


        //
        // Binary operations
        //

        TEST_F(FloatingPoint, AddDouble)
        {
            auto setup = GetSetup();

            {
                Function<double, double, double> expression(setup->GetAllocator(), setup->GetCode());

                auto & a = expression.Add(expression.GetP2(), expression.GetP1());
                auto function = expression.Compile(a);

                double p1 = 12340000.0;
                double p2 = 5678.0;

                auto expected = p1 + p2;
                auto observed = function(p1, p2);

                ASSERT_EQ(observed, expected);
            }
        }


        TEST_F(FloatingPoint, AddImmediateDouble)
        {
            auto setup = GetSetup();

            {
                Function<double, double> expression(setup->GetAllocator(), setup->GetCode());

                double immediate = 123.456;
                auto & a = expression.Immediate(immediate);
                auto & b = expression.Add(expression.GetP1(), a);
                auto function = expression.Compile(b);

                double p1 = 12340000.0;

                auto expected = p1 + immediate;
                auto observed = function(p1);

                ASSERT_EQ(observed, expected);
            }
        }


        TEST_F(FloatingPoint, AddTwoImmediateFloat)
        {
            auto setup = GetSetup();

            {
                Function<float> expression(setup->GetAllocator(), setup->GetCode());

                float immediate1 = 123.0f;
                float immediate2 = 0.456f;
                auto & a = expression.Immediate(immediate1);
                auto & b = expression.Immediate(immediate2);
                auto & c = expression.Add(a, b);
                auto function = expression.Compile(c);


                auto expected = immediate1 + immediate2;
                auto observed = function();

                ASSERT_EQ(observed, expected);
            }
        }


        TEST_F(FloatingPoint, AddTwoImmediateDouble)
        {
            auto setup = GetSetup();

            {
                Function<double> expression(setup->GetAllocator(), setup->GetCode());

                double immediate1 = 123.0;
                double immediate2 = 0.456;
                auto & a = expression.Immediate(immediate1);
                auto & b = expression.Immediate(immediate2);
                auto & c = expression.Add(a, b);
                auto function = expression.Compile(c);


                auto expected = immediate1 + immediate2;
                auto observed = function();

                ASSERT_EQ(observed, expected);
            }
        }

        TEST_CASES_END
    }
}
