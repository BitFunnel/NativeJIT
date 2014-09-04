#include "stdafx.h"


#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"



namespace NativeJIT
{
    namespace FloatingPointUnitTest3
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


            // TODO
            //   float (vs double)
            //   Sethi-Ullman register allocator works correctly with floats.

            //
            // Immediate values.
            //

            //
            // Codebase needs some work before this test can be enabled.
            //   TODO: Complete RIP relative addressing.
            //   TODO: Remove or simplify ExpressionTree::Mov() overloads.
            //

            TestCase(ImmediateDouble)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<double> expression(m_allocator, *m_code);

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
                AutoResetAllocator reset(m_allocator);

                {
                    Function<float> expression(m_allocator, *m_code);

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
                AutoResetAllocator reset(m_allocator);

                {
                    Function<double, double, double> expression(m_allocator, *m_code);

                    auto & a = expression.Add(expression.GetP2(), expression.GetP1());
                    auto function = expression.Compile(a);

                    double p1 = 12340000.0;
                    double p2 = 5678.0;

                    auto expected = p1 + p2;
                    auto observed = function(p1, p2);

                    TestAssert(observed == expected);
                }
            }


            //
            // Codebase needs some work before this test can be enabled.
            //   TODO: Complete RIP relative addressing.
            //   TODO: Remove or simplify ExpressionTree::Mov() overloads.
            //

            TestCase(AddImmediateDouble)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<double, double> expression(m_allocator, *m_code);

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


// The following test won't compile.
// Error	1	error C2664: 'void NativeJIT::X64CodeGenerator::Mov<8>(NativeJIT::Register<SIZE,ISFLOAT>,int,NativeJIT::Register<8,true>)' : cannot convert parameter 3 from 'NativeJIT::Register<SIZE,ISFLOAT>' to 'NativeJIT::Register<SIZE,ISFLOAT>'	c:\git\nativejit\nativejit\inc\nativejit\x64codegenerator.h	1011
            //TestCase(AddTwoImmediateFloat)
            //{
            //    AutoResetAllocator reset(m_allocator);

            //    {
            //        Function<float> expression(m_allocator, *m_code);

            //        float immediate1 = 123.0f;
            //        float immediate2 = 0.456f;
            //        auto & a = expression.Immediate(immediate1);
            //        auto & b = expression.Immediate(immediate2);
            //        auto & c = expression.Add(a, b);
            //        auto function = expression.Compile(c);


            //        auto expected = immediate1 + immediate2;
            //        auto observed = function();

            //        TestAssert(observed == expected);
            //    }
            //}


            TestCase(AddTwoImmediateDouble)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<double> expression(m_allocator, *m_code);

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


        private:
            Allocator m_allocator;
            ExecutionBuffer m_executionBuffer;
            std::unique_ptr<FunctionBuffer> m_code;
        };
    }
}
