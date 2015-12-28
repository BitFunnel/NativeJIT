#include "stdafx.h"

#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "CastNode.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"
#include "TestSetup.h"

namespace NativeJIT
{
    namespace CastUnitTest
    {
        TestClass(CastTest), private TestFixture
        {
        public:
            TestCase(FromInt8)
            {
                TestCasts<__int8>(-1);
                TestCasts<unsigned __int8>(0xFF);
            }


            TestCase(FromInt16)
            {
                TestCasts<__int16>(-1);
                TestCasts<unsigned __int16>(0xFFFF);
            }


            TestCase(FromInt32)
            {
                TestCasts<__int32>(-1);
                TestCasts<unsigned __int32>(0xFFFFFFFF);
            }


            TestCase(FromInt64)
            {
                TestCasts<__int64>(-1);
                TestCasts<unsigned __int64>(0xFFFFFFFFFFFFFFFFul);
            }


            TestCase(FromFloat)
            {
                TestCasts<float>(-123.7f);
                TestCasts<float>(123.7f);

                // Special case, target is an UInt64 that has the MSB bit set.
                TestCast<unsigned __int64>(static_cast<float>(0xA000000000000000));
            }


            TestCase(FromDouble)
            {
                TestCasts<double>(-123.7);
                TestCasts<double>(123.7);

                // Special case, target is an UInt64 that has the MSB bit set.
                TestCast<unsigned __int64>(static_cast<double>(0xA000000000000000));
            }


            TestCase(Packed)
            {
                auto packed = Packed<>::Push<5>(31).Push<4>(15).Push<3>(7);

                TestCast<unsigned __int64, decltype(packed)>(packed);
                TestCast<decltype(packed), unsigned __int64>(123456);
            }


            TestCase(VoidPointer)
            {
                TestCast<unsigned __int64, void*>(this);
            }


            TestCase(FuncPointer)
            {
                auto funcPtr = &DummyFunc;
                TestCast<decltype(funcPtr), void*>(funcPtr);
            }


        private:
            template <typename FROM>
            void TestCasts(FROM testValue)
            {
                TestCast<__int8>(testValue);
                TestCast<__int16>(testValue);
                TestCast<__int32>(testValue);
                TestCast<__int64>(testValue);

                TestCast<unsigned __int8>(testValue);
                TestCast<unsigned __int16>(testValue);
                TestCast<unsigned __int32>(testValue);
                TestCast<unsigned __int64>(testValue);

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
                auto & cast = expression.Cast<TO>(immediate);
                auto function = expression.Compile(cast);

                auto expected = Casting::ForcedCast<TO>(testValue);
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

                auto & cast = expression.Cast<TO>(expression.GetP1());
                auto function = expression.Compile(cast);

                auto expected = Casting::ForcedCast<TO>(testValue);
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
                auto & cast = expression.Cast<TO>(from);
                auto function = expression.Compile(cast);

                auto expected = Casting::ForcedCast<TO>(testValue);
                auto observed = function(&input);

                TestEqual(expected,
                          observed,
                          "Incorrect cast from %s to %s (indirect)",
                          typeid(FROM).name(),
                          typeid(TO).name());
            }


            // Dummy function, to test casting from/to function pointers.
            static unsigned __int64 DummyFunc(unsigned __int64 x)
            {
                return x * 2;
            }
        };
    }
}
