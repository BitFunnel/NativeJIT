#include "stdafx.h"


#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"



namespace NativeJIT
{
    namespace UnsignedUnitTest
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


            // TODO: Test all comparision operators.
            // TODO: Factor out unit test environment base class.

            //
            // Conditionals
            //

            TestCase(GreaterThan)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<bool, unsigned __int64, unsigned __int64> expression(m_allocator, *m_code);

                    auto & test = expression.GreaterThan(expression.GetP1(), expression.GetP2());
                    auto greaterThan = expression.Compile(test);

                    TestEqual(false, greaterThan(3, 4));
                    TestEqual(true, greaterThan(4, 3));
                }
            }


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
