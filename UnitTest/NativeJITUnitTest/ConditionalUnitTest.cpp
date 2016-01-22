#include "stdafx.h"


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


        // TODO: Test all comparision operators.

        //
        // Conditionals
        //


        TEST_CASE_F(Conditional, Conditional)
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

                TestEqual(expected, observed);

                p1 = 5;
                p2 = 4;

                expected = (p1 > p2) ? trueValue : falseValue;
                observed = function(p1, p2);

                TestEqual(expected, observed);
            }
        }


        TEST_CASE_F(Conditional, IfNotZero)
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

            TestEqual(expected, observed);

            p1 = 1;

            expected = p1 != 0 ? p2 : p3;
            observed = function(p1, p2, p3);

            TestEqual(expected, observed);
        }


        TEST_CASE_F(Conditional, If)
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

            TestEqual(expected, observed);

            p1 = false;

            expected = p1 ? p2 : p3;
            observed = function(p1, p2, p3);

            TestEqual(expected, observed);
        }

        TEST_CASES_END
    }
}
