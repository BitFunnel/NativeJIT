#include "stdafx.h"

#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/CodeGen/ValuePredicates.h"
#include "NativeJIT/Function.h"
#include "NativeJIT/Nodes/CastNode.h"
#include "Temporary/Allocator.h"
#include "TestSetup.h"


namespace NativeJIT
{
    namespace CastUnitTest
    {
        TEST_FIXTURE_START(CastTest)

        protected:
            template <typename FROM>
            void TestCasts(FROM testValue)
            {
                TestCast<int8_t>(testValue);
                TestCast<int16_t>(testValue);
                TestCast<int32_t>(testValue);
                TestCast<int64_t>(testValue);

                TestCast<uint8_t>(testValue);
                TestCast<uint16_t>(testValue);
                TestCast<uint32_t>(testValue);
                TestCast<uint64_t>(testValue);

                TestCast<float>(testValue);
                TestCast<double>(testValue);
            }


            template <typename TO, typename FROM>
            void TestCast(FROM testValue)
            {
                TestCastImmediate<TO>(testValue);
                TestCastDirect<TO>(testValue);
                TestCastIndirect<TO>(testValue);
            }


            template <typename TO, typename FROM>
            void TestCastImmediate(FROM testValue)
            {
                auto setup = GetSetup();

                Function<TO> expression(setup->GetAllocator(), setup->GetCode());

                auto & immediate = expression.Immediate(testValue);
                auto & cast = expression.template Cast<TO>(immediate);
                auto function = expression.Compile(cast);

                auto expected = ForcedCast<TO>(testValue);
                auto observed = function();

                TestEqual(expected,
                          observed,
                          "Incorrect cast from %s to %s (immediate)",
                          typeid(FROM).name(),
                          typeid(TO).name());
            }


            template <typename TO, typename FROM>
            void TestCastDirect(FROM testValue)
            {
                auto setup = GetSetup();

                Function<TO, FROM> expression(setup->GetAllocator(), setup->GetCode());

                auto & cast = expression.template Cast<TO>(expression.GetP1());
                auto function = expression.Compile(cast);

                auto expected = ForcedCast<TO>(testValue);
                auto observed = function(testValue);

                TestEqual(expected,
                          observed,
                          "Incorrect cast from %s to %s (direct)",
                          typeid(FROM).name(),
                          typeid(TO).name());
            }


            template <typename TO, typename FROM>
            void TestCastIndirect(FROM testValue)
            {
                auto setup = GetSetup();

                struct TestInput
                {
                    void* m_dummy;
                    FROM m_value;
                };

                TestInput input = { nullptr, testValue };
                Function<TO, TestInput*> expression(setup->GetAllocator(), setup->GetCode());

                auto & valueField = expression.FieldPointer(expression.GetP1(),
                                                            &TestInput::m_value);
                auto & from = expression.Deref(valueField);
                auto & cast = expression.template Cast<TO>(from);
                auto function = expression.Compile(cast);

                auto expected = ForcedCast<TO>(testValue);
                auto observed = function(&input);

                TestEqual(expected,
                          observed,
                          "Incorrect cast from %s to %s (indirect)",
                          typeid(FROM).name(),
                          typeid(TO).name());
            }


            // Dummy function, to test casting from/to function pointers.
            static uint64_t DummyFunc(uint64_t x)
            {
                return x * 2;
            }

        TEST_FIXTURE_END_TEST_CASES_BEGIN


        TEST_CASE_F(CastTest, FromInt8)
        {
            TestCasts<int8_t>(-1);
            TestCasts<uint8_t>(0xFF);
        }


        TEST_CASE_F(CastTest, FromInt16)
        {
            TestCasts<int16_t>(-1);
            TestCasts<uint16_t>(0xFFFF);
        }


        TEST_CASE_F(CastTest, FromInt32)
        {
            TestCasts<int32_t>(-1);
            TestCasts<uint32_t>(0xFFFFFFFF);
        }


        TEST_CASE_F(CastTest, FromInt64)
        {
            TestCasts<int64_t>(-1);
            TestCasts<uint64_t>(0xFFFFFFFFFFFFFFFFul);
        }


        TEST_CASE_F(CastTest, FromFloat)
        {
            TestCasts<float>(-123.7f);
            TestCasts<float>(123.7f);

            // Special case, target is an UInt64 that has the MSB bit set.
            TestCast<uint64_t>(static_cast<float>(0xA000000000000000));
        }


        TEST_CASE_F(CastTest, FromDouble)
        {
            TestCasts<double>(-123.7);
            TestCasts<double>(123.7);

            // Special case, target is an UInt64 that has the MSB bit set.
            TestCast<uint64_t>(static_cast<double>(0xA000000000000000));
        }


        TEST_CASE_F(CastTest, Packed)
        {
            auto packed = Packed<>::Push<5>(31).Push<4>(15).Push<3>(7);

            TestCast<uint64_t, decltype(packed)>(packed);
            TestCast<decltype(packed), uint64_t>(123456);
        }


        TEST_CASE_F(CastTest, VoidPointer)
        {
            TestCast<uint64_t, void*>(this);
        }


        TEST_CASE_F(CastTest, FuncPointer)
        {
            auto funcPtr = &DummyFunc;
            TestCast<decltype(funcPtr), void*>(reinterpret_cast<void*>(funcPtr));
        }


        TEST_CASES_END
    }
}
