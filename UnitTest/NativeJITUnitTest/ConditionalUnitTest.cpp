#include "stdafx.h"


#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"
#include "TestSetup.h"


namespace NativeJIT
{
    namespace ConditionalUnitTest
    {
        TestClass(FunctionTest), private TestClassSetup
        {
        public:

            // TODO: Test all comparision operators.

            //
            // Conditionals
            //

            TestCase(GreaterThan)
            {
                auto setup = GetSetup();

                {
                    Function<bool, unsigned __int64, unsigned __int64> expression(setup->GetAllocator(), setup->GetCode());

                    auto & test = expression.GreaterThan(expression.GetP1(), expression.GetP2());
                    auto greaterThan = expression.Compile(test);

                    TestEqual(false, greaterThan(3, 4));
                    TestEqual(true, greaterThan(4, 3));
                }
            }


            TestCase(Conditional)
            {
                auto setup = GetSetup();

                {
                    Function<unsigned __int64, unsigned __int64, unsigned __int64> expression(setup->GetAllocator(), setup->GetCode());

                    unsigned __int64 trueValue = 5;
                    unsigned __int64 falseValue = 6;

                    auto & a = expression.GreaterThan(expression.GetP1(), expression.GetP2());
                    auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
                    auto function = expression.Compile(b);

                    unsigned __int64 p1 = 3;
                    unsigned __int64 p2 = 4;

                    auto expected = (p1 > p2) ? trueValue : falseValue;
                    auto observed = function(p1, p2);

                    TestEqual(expected, observed);

                    p1 = 5;
                    p2 = 4;

                    expected = (p1 > p2) ? trueValue : falseValue;
                    observed = function(p1, p2);

                    TestEqual(expected, observed);
                }
            }


            TestCase(IfNotZero)
            {
                auto setup = GetSetup();

                Function<unsigned __int64,
                         unsigned __int32,
                         unsigned __int64,
                         unsigned __int64> e(setup->GetAllocator(), setup->GetCode());

                auto & test = e.IfNotZero(e.GetP1(), e.GetP2(), e.GetP3());
                auto function = e.Compile(test);

                unsigned __int32 p1 = 0;
                unsigned __int64 p2 = 0xFFFFFFFFFFFFFFFF;
                unsigned __int64 p3 = 0;

                auto expected = p1 != 0 ? p2 : p3;
                auto observed = function(p1, p2, p3);

                TestEqual(expected, observed);

                p1 = 1;

                expected = p1 != 0 ? p2 : p3;
                observed = function(p1, p2, p3);

                TestEqual(expected, observed);
            }


            TestCase(If)
            {
                auto setup = GetSetup();

                Function<unsigned __int64,
                         bool,
                         unsigned __int64,
                         unsigned __int64> e(setup->GetAllocator(), setup->GetCode());

                auto & test = e.If(e.GetP1(), e.GetP2(), e.GetP3());
                auto function = e.Compile(test);

                bool p1 = true;
                unsigned __int64 p2 = 0xFFFFFFFFFFFFFFFF;
                unsigned __int64 p3 = 0;

                auto expected = p1 ? p2 : p3;
                auto observed = function(p1, p2, p3);

                TestEqual(expected, observed);

                p1 = false;

                expected = p1 ? p2 : p3;
                observed = function(p1, p2, p3);

                TestEqual(expected, observed);
            }
        };
    }
}
