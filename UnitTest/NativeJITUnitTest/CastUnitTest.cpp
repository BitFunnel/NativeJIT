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


#include <iostream>

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

                ASSERT_EQ(expected, observed)
                    << "Incorrect cast from "
                    << typeid(FROM).name()
                    << " to "
                    << typeid(TO).name();
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

                ASSERT_EQ(expected, observed)
                    << "Incorrect cast from "
                    << typeid(FROM).name()
                    << " to "
                    << typeid(TO).name();
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

                ASSERT_EQ(expected, observed)
                    << "Incorrect cast from "
                    << typeid(FROM).name()
                    << " to "
                    << typeid(TO).name();
            }


            // Dummy function, to test casting from/to function pointers.
            static uint64_t DummyFunc(uint64_t x)
            {
                return x * 2;
            }

        TEST_FIXTURE_END_TEST_CASES_BEGIN

        // A no-op test which ensures that the type returned by
        // Add/RemoveTargetConstCast is correct by getting compiled.
        TEST_F(CastTest, TargetCast)
        {
            auto setup = GetSetup();

            Function<void const *, void*> add(setup->GetAllocator(), setup->GetCode());
            add.Return(add.AddTargetConstCast(add.GetP1()));

            Function<void*, void const *> rem(setup->GetAllocator(), setup->GetCode());
            rem.Return(rem.RemoveTargetConstCast(rem.GetP1()));
        }


        TEST_F(CastTest, ConstCastNonReference)
        {
            auto setup = GetSetup();

            Function<int const, int> add(setup->GetAllocator(), setup->GetCode());
            add.Return(add.AddConstCast(add.GetP1()));

            Function<int, int const> rem(setup->GetAllocator(), setup->GetCode());
            rem.Return(rem.RemoveConstCast(rem.GetP1()));
        }


        TEST_F(CastTest, ConstCastReference)
        {
            auto setup = GetSetup();

            Function<int const &, int &> add(setup->GetAllocator(), setup->GetCode());
            add.Return(add.AddConstCast(add.GetP1()));

            Function<int &, int const &> rem(setup->GetAllocator(), setup->GetCode());
            rem.Return(rem.RemoveConstCast<int const &>(rem.GetP1()));
        }


        TEST_F(CastTest, FromInt8)
        {
            TestCasts<int8_t>(-1);
            TestCasts<uint8_t>(0xFF);
        }


        TEST_F(CastTest, FromInt16)
        {
            TestCasts<int16_t>(-1);
            TestCasts<uint16_t>(0xFFFF);
        }


        TEST_F(CastTest, FromInt32)
        {
            TestCasts<int32_t>(-1);
            TestCasts<uint32_t>(0xFFFFFFFF);
        }


        TEST_F(CastTest, FromInt64)
        {
            TestCasts<int64_t>(-1);
            TestCasts<uint64_t>(0xFFFFFFFFFFFFFFFFul);
        }


        TEST_F(CastTest, FromFloat)
        {
            TestCasts<float>(-123.7f);
            TestCasts<float>(123.7f);

            // Special case, target is an UInt64 that has the MSB bit set.
            TestCast<uint64_t>(static_cast<float>(0xA000000000000000));
        }


        TEST_F(CastTest, FromDouble)
        {
            TestCasts<double>(-123.7);
            TestCasts<double>(123.7);

            // Special case, target is an UInt64 that has the MSB bit set.
            TestCast<uint64_t>(static_cast<double>(0xA000000000000000));
        }


        TEST_F(CastTest, Packed)
        {
            auto packed = Packed<3, 4, 5>::FromComponents(7, 15, 31);

            TestCast<PackedUnderlyingType, decltype(packed)>(packed);
            TestCast<decltype(packed), PackedUnderlyingType>(123456);
        }


        TEST_F(CastTest, VoidPointer)
        {
            TestCast<uint64_t, void*>(this);
        }


        TEST_F(CastTest, FuncPointer)
        {
            auto funcPtr = &DummyFunc;
            TestCast<decltype(funcPtr), void*>(reinterpret_cast<void*>(funcPtr));
        }


        TEST_CASES_END
    }
}
