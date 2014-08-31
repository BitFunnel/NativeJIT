#include "stdafx.h"


#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"



namespace NativeJIT
{
    namespace FloatingPointUnitTest
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

            TestCase(ImmediateDouble)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<double> expression(m_allocator, *m_code);

                    double value = 123.456;;
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

            //
            // Codebase needs some work before this test can be enabled.
            //   TODO: Need support for free list of XMM registers.
            //   TODO: ExpressionTree::Direct().
            //

            //TestCase(AddDouble)
            //{
            //    AutoResetAllocator reset(m_allocator);

            //    {
            //        Function<double, double, double> expression(m_allocator, *m_code);

            //        auto & a = expression.Add(expression.GetP2(), expression.GetP1());
            //        auto function = expression.Compile(a);

            //        double p1 = 12340000.0;
            //        double p2 = 5678.0;

            //        auto expected = p1 + p2;
            //        auto observed = function(p1, p2);

            //        TestAssert(observed == expected);
            //    }
            //}


        private:
            Allocator m_allocator;
            ExecutionBuffer m_executionBuffer;
            std::unique_ptr<FunctionBuffer> m_code;
        };
    }
}
