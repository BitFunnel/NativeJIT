#include "stdafx.h"


#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"
#include "TestSetup.h"


namespace NativeJIT
{
    namespace FloatingPointUnitTest3
    {
        TestClass(FunctionTest), private TestFixture
        {
        public:
            TestCase(ImmediateDouble)
            {
                auto setup = GetSetup();

                {
                    Function<double> expression(setup->GetAllocator(), setup->GetCode());

                    double value = 123.456;
                    auto & a = expression.Immediate(value);

                    auto function = expression.Compile(a);

                    auto expected = value;
                    auto observed = function();

                    TestAssert(observed == expected);
                }
            }


            TestCase(ImmediateFloat)
            {
                auto setup = GetSetup();

                {
                    Function<float> expression(setup->GetAllocator(), setup->GetCode());

                    float value = 123.456f;
                    auto & a = expression.Immediate(value);

                    auto function = expression.Compile(a);

                    auto expected = value;
                    auto observed = function();

                    TestAssert(observed == expected);
                }
            }


            //
            // Binary operations
            //

            TestCase(AddDouble)
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

                    TestAssert(observed == expected);
                }
            }


            TestCase(AddImmediateDouble)
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

                    TestAssert(observed == expected);
                }
            }


            TestCase(AddTwoImmediateFloat)
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

                    TestAssert(observed == expected);
                }
            }


            TestCase(AddTwoImmediateDouble)
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

                    TestAssert(observed == expected);
                }
            }
        };
    }
}
