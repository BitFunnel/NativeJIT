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


#include <cmath>        // For float std::abs(float).
#include <iostream>
#include <memory>

#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/Function.h"
#include "Temporary/Allocator.h"
#include "TestSetup.h"


#ifndef _MSC_VER
#define _AddressOfReturnAddress() __builtin_frame_address(0)
#endif


namespace NativeJIT
{
    namespace FunctionUnitTest
    {
        TEST_FIXTURE_START(FunctionTest)

        protected:
            static int s_sampleFunctionCalls;
            static int s_intParameter1;
            static int s_intParameter2;

            static char s_charParameter1;

            static int64_t s_int64Parameter3;

            static bool s_boolParameter4;

            static unsigned s_regPreserveTestFuncCallCount;

            static int SampleFunction0()
            {
                ++s_sampleFunctionCalls;
                return 12345;
            }


            static int SampleFunction1(char p1)
            {
                s_charParameter1 = p1;
                ++s_sampleFunctionCalls;
                return p1 + 3;
            }


            static int SampleFunction2(int p1, int p2)
            {
                s_intParameter1 = p1;
                s_intParameter2 = p2;
                ++s_sampleFunctionCalls;
                return p1 + p2;
            }


            static int SampleFunction2DifferentTypes(char p1, int p2)
            {
                s_charParameter1 = p1;
                s_intParameter2 = p2;
                ++s_sampleFunctionCalls;
                return p1 + p2;
            }


            static int64_t SampleFunction3(char p1, int p2, int64_t p3)
            {
                s_charParameter1 = p1;
                s_intParameter2 = p2;
                s_int64Parameter3 = p3;
                ++s_sampleFunctionCalls;
                return p1 + p2 + p3;
            }


            static int64_t SampleFunction4(char p1, int p2, int64_t p3, bool p4)
            {
                s_charParameter1 = p1;
                s_intParameter2 = p2;
                s_int64Parameter3 = p3;
                s_boolParameter4 = p4;
                ++s_sampleFunctionCalls;
                return p1 + p2 + p3 + (p4 ? 123 : 456);
            }


            // These helper functions are used to overwrite the EAX/XMM0s
            // registers with a specific value, different than some special value
            // that other functions return.

            template <int VAL>
            static int ReturnInt()
            {
                return VAL;
            }


            static float Return7Point7()
            {
                return 7.7f;
            }


            static float Return3Point3()
            {
                return 3.3f;
            }


            // These two functions return a fixed value in EAX/XMM0s and overwrite
            // the opposite register (XMM0s/EAX) with an unrelated value using
            // the functions above. This makes it possible to verify that XMM0s/EAX
            // is preserved when necessary (f. ex. for its re-use in cache).
            // Since the order of the calls is important in such tests, they
            // ensure that the call is made when expected.

            // A wrapper assert method for GTest which has a void return type
            // so that test asserts can compile.
            static void AssertCallOrder(char const * s_methodName,
                                        unsigned expectedCallNumber,
                                        unsigned actualCallNumber)
            {
                static_cast<void>(s_methodName); // Unreferenced parameter for GTest.

                EXPECT_EQ(expectedCallNumber, actualCallNumber)
                    << "unexpected call order "
                    << s_methodName;
            }


            template <unsigned CALLNUMBER>
            static int Return10()
            {
                ++s_regPreserveTestFuncCallCount;
                AssertCallOrder("Return10", CALLNUMBER, s_regPreserveTestFuncCallCount);

                // Call any function which returns a float other than 1.3 used
                // in the function below to make sure that xmm0s is overwritten.
                // This will ensure that the test will succeed only if the caller
                // which needs to re-use the original value correctly preserved it.
                Return7Point7();

                return 10;
            }


            template <unsigned CALLNUMBER>
            static float Return1Point3()
            {
                ++s_regPreserveTestFuncCallCount;
                AssertCallOrder("Return1Point3", CALLNUMBER, s_regPreserveTestFuncCallCount);

                // Call any function which returns an integer other than 10 used
                // in the function above to make sure eax is overwritten.
                // This will ensure that the test will succeed only if the caller
                // which needs to re-use the original value correctly preserved it.
                ReturnInt<999>();

                return 1.3f;
            }


        TEST_FIXTURE_END_TEST_CASES_BEGIN


        //
        // JIT Functions with 0, 1, 2, 3, and 4 parameters.
        //

        TEST_F(FunctionTest, FunctionZeroParameters)
        {
            auto setup = GetSetup();

            {
                Function<int64_t> expression(setup->GetAllocator(), setup->GetCode());

                int64_t expected = 1234ll;
                auto & a = expression.Immediate(expected);
                auto function = expression.Compile(a);

                auto observed = function();

                EXPECT_EQ(expected, observed);
            }
        }


        TEST_F(FunctionTest, FunctionOneParameter)
        {
            auto setup = GetSetup();

            {
                Function<int64_t, int64_t> expression(setup->GetAllocator(), setup->GetCode());

                auto & a = expression.GetP1();
                auto function = expression.Compile(a);

                int64_t p1 = 1234ll;

                auto expected = p1;
                auto observed = function(p1);

                EXPECT_EQ(expected, observed);
            }
        }


        TEST_F(FunctionTest, FunctionTwoParameters)
        {
            auto setup = GetSetup();

            {
                Function<int64_t, int64_t, int64_t> expression(setup->GetAllocator(), setup->GetCode());

                auto & a = expression.Add(expression.GetP1(), expression.GetP2());
                auto function = expression.Compile(a);

                int64_t p1 = 12340000ll;
                int64_t p2 = 5678ll;

                auto expected = p1 + p2;
                auto observed = function(p1, p2);

                EXPECT_EQ(expected, observed);
            }
        }


        TEST_F(FunctionTest, FunctionThreeParameters)
        {
            auto setup = GetSetup();

            {
                Function<int, int, int, int> expression(setup->GetAllocator(), setup->GetCode());

                auto & a = expression.Sub(expression.GetP2(), expression.GetP1());
                auto & b = expression.Add(expression.GetP3(), a);
                auto function = expression.Compile(b);

                int p1 = 0x12340000;
                int p2 = 0x5678;
                int p3 = 0x11111111;

                auto expected = p2 - p1 + p3;
                auto observed = function(p1, p2, p3);

                EXPECT_EQ(expected, observed);
            }
        }


        TEST_F(FunctionTest, FunctionFourParameters)
        {
            auto setup = GetSetup();

            {
                Function<char, char, char, char, char> expression(setup->GetAllocator(), setup->GetCode());

                auto & a = expression.Sub(expression.GetP1(), expression.GetP2());
                auto & b = expression.Sub(expression.GetP3(), expression.GetP4());
                auto & c = expression.Sub(a, b);
                auto function = expression.Compile(c);

                char p1 = 100;
                char p2 = 50;
                char p3 = 10;
                char p4 = 5;

                auto expected = (p1 - p2) - (p3 - p4);
                auto observed = function(p1, p2, p3, p4);

                EXPECT_EQ(expected, observed);
            }
        }


        // The Windows x64 ABI and the System V ABI handle parameter passing
        // differently. This test case verifies that a function with int and
        // float parameters works correctly.
        TEST_F(FunctionTest, FunctionTwoMixedParameters)
        {
            auto setup = GetSetup();

            {
                Function<float, int, float> expression(setup->GetAllocator(), setup->GetCode());

                float c10 = 10.0f;

                auto & a = expression.Mul(expression.GetP2(), expression.Immediate<float>(c10));
                auto & b = expression.Add(expression.Cast<float>(expression.GetP1()), a);

                auto function = expression.Compile(b);

                int p1 = 1;
                float p2 = 2.0;

                auto expected = static_cast<float>(p1) + (p2 * 10.0f);
                auto observed = function(p1, p2);

                EXPECT_EQ(expected, observed);
            }
        }


        // The Windows x64 ABI and the System V ABI handle parameter passing
        // differently. This test case verifies that a function with int and
        // float parameters works correctly.
        TEST_F(FunctionTest, FunctionThreeMixedParameters)
        {
            auto setup = GetSetup();

            {
                Function<float, int, float, int> expression(setup->GetAllocator(), setup->GetCode());

                float c10 = 10.0f;
                float c100 = 100.0f;

                auto & a = expression.Mul(expression.GetP2(), expression.Immediate<float>(c10));
                auto & b = expression.Mul(expression.Cast<float>(expression.GetP3()), expression.Immediate<float>(c100));
                auto & c = expression.Add(expression.Cast<float>(expression.GetP1()), a);
                auto & d = expression.Add(c, b);

                auto function = expression.Compile(d);

                int p1 = 1;
                float p2 = 2.0;
                int p3 = 3;

                auto expected = static_cast<float>(p1) + (p2 * 10.0f) + (p3 * 100);
                auto observed = function(p1, p2, p3);

                EXPECT_EQ(expected, observed);
            }
        }


        // The Windows x64 ABI and the System V ABI handle parameter passing
        // differently. This test case verifies that a function with int and
        // float parameters works correctly.
        TEST_F(FunctionTest, FunctionFourMixedParameters)
        {
            auto setup = GetSetup();

            {
                Function<float, int, float, int, float> expression(setup->GetAllocator(), setup->GetCode());

                float c10 = 10.0f;
                float c100 = 100.0f;
                float c1000 = 1000.0f;

                auto & a = expression.Mul(expression.GetP2(), expression.Immediate<float>(c10));
                auto & b = expression.Mul(expression.Cast<float>(expression.GetP3()), expression.Immediate<float>(c100));
                auto & c = expression.Mul(expression.GetP4(), expression.Immediate<float>(c1000));

                auto & d = expression.Add(expression.Cast<float>(expression.GetP1()), a);
                auto & e = expression.Add(d, b);
                auto & f = expression.Add(e, c);

                auto function = expression.Compile(f);

                int p1 = 1;
                float p2 = 2.0;
                int p3 = 3;
                float p4 = 4.0;

                auto expected = static_cast<float>(p1) + (p2 * 10.0f) + (p3 * 100) + (p4 * 1000.0f);
                auto observed = function(p1, p2, p3, p4);

                EXPECT_EQ(expected, observed);
            }
        }


        //
        // Calling C functions with 0, 1, 2, 3, and 4 parameters.
        //

        TEST_F(FunctionTest, CallZeroParameters)
        {
            auto setup = GetSetup();

            {
                Function<int> expression(setup->GetAllocator(), setup->GetCode());

                typedef int (*F)();
                auto & sampleFunction = expression.Immediate<F>(SampleFunction0);
                auto & a = expression.Call(sampleFunction);
                auto function = expression.Compile(a);

                auto expected = SampleFunction0();

                s_sampleFunctionCalls = 0;
                auto observed = function();

                EXPECT_EQ(expected, observed);
                EXPECT_EQ(1, s_sampleFunctionCalls);
            }
        }


        TEST_F(FunctionTest, CallOneParameter)
        {
            auto setup = GetSetup();

            {
                Function<int, char> expression(setup->GetAllocator(), setup->GetCode());

                typedef int (*F)(char);
                auto & sampleFunction = expression.Immediate<F>(SampleFunction1);
                auto & a = expression.Call(sampleFunction, expression.GetP1());
                auto function = expression.Compile(a);

                const char p1 = 0x73;
                auto expected = SampleFunction1(p1);

                // Anything other than p1 will do.
                s_charParameter1 = p1 + 1;
                s_sampleFunctionCalls = 0;
                auto observed = function(p1);

                EXPECT_EQ(expected, observed);
                EXPECT_EQ(1, s_sampleFunctionCalls);
                EXPECT_EQ(s_charParameter1, p1);
            }
        }


        TEST_F(FunctionTest, CallTwoParameters)
        {
            auto setup = GetSetup();

            {
                Function<int, int, int> expression(setup->GetAllocator(), setup->GetCode());

                typedef int (*F)(int, int);
                auto & sampleFunction = expression.Immediate<F>(SampleFunction2);
                auto & a = expression.Call(sampleFunction, expression.GetP2(), expression.GetP1());
                auto function = expression.Compile(a);

                const int p1 = 12340000;
                const int p2 = 5678;

                auto expected = SampleFunction2(p1, p2);

                // Anything other than p1/p2 will do.
                s_intParameter1 = p1 + 1;
                s_intParameter2 = p2 + 1;
                s_sampleFunctionCalls = 0;
                auto observed = function(p2, p1);

                EXPECT_EQ(expected, observed);
                EXPECT_EQ(1, s_sampleFunctionCalls);
                EXPECT_EQ(s_intParameter1, p1);
                EXPECT_EQ(s_intParameter2, p2);
            }
        }


        TEST_F(FunctionTest, CallTwoParametersDifferentTypes)
        {
            auto setup = GetSetup();

            {
                Function<int, int, char> expression(setup->GetAllocator(), setup->GetCode());

                typedef int (*F)(char, int);
                auto & sampleFunction = expression.Immediate<F>(SampleFunction2DifferentTypes);
                auto & a = expression.Call(sampleFunction, expression.GetP2(), expression.GetP1());
                auto function = expression.Compile(a);

                const char p1 = 0x74;
                const int p2 = 5678;

                auto expected = SampleFunction2DifferentTypes(p1, p2);

                // Anything other than p1/p2 will do.
                s_charParameter1 = p1 + 1;
                s_intParameter2 = p2 + 1;
                s_sampleFunctionCalls = 0;
                auto observed = function(p2, p1);

                EXPECT_EQ(expected, observed);
                EXPECT_EQ(1, s_sampleFunctionCalls);
                EXPECT_EQ(s_charParameter1, p1);
                EXPECT_EQ(s_intParameter2, p2);
            }
        }


        TEST_F(FunctionTest, CallThreeParameters)
        {
            auto setup = GetSetup();

            {
                Function<int64_t, int64_t, int, char> expression(setup->GetAllocator(), setup->GetCode());

                typedef int64_t (*F)(char, int, int64_t);
                auto & sampleFunction = expression.Immediate<F>(SampleFunction3);
                auto & a = expression.Call(sampleFunction,
                                            expression.GetP3(),
                                            expression.GetP2(),
                                            expression.GetP1());
                auto function = expression.Compile(a);

                const char p1 = 0x73;
                const int p2 = 5678;
                const int64_t p3 = 12340000ll;

                auto expected = SampleFunction3(p1, p2, p3);

                // Anything other than p1/p2/p3 will do.
                s_charParameter1 = p1 + 1;
                s_intParameter2 = p2 + 1;
                s_int64Parameter3 = p3 + 1;
                s_sampleFunctionCalls = 0;
                auto observed = function(p3, p2, p1);

                EXPECT_EQ(expected, observed);
                EXPECT_EQ(1, s_sampleFunctionCalls);
                EXPECT_EQ(s_charParameter1, p1);
                EXPECT_EQ(s_intParameter2, p2);
                EXPECT_EQ(s_int64Parameter3, p3);
            }
        }


        TEST_F(FunctionTest, CallFourParameters)
        {
            auto setup = GetSetup();

            {
                Function<int64_t, bool, int64_t, int, char> expression(setup->GetAllocator(), setup->GetCode());

                typedef int64_t (*F)(char, int, int64_t, bool);
                auto & sampleFunction = expression.Immediate<F>(SampleFunction4);
                auto & a = expression.Call(sampleFunction,
                                            expression.GetP4(),
                                            expression.GetP3(),
                                            expression.GetP2(),
                                            expression.GetP1());
                auto function = expression.Compile(a);

                const char p1 = 0x73;
                const int p2 = 5678;
                const int64_t p3 = 12340000;
                const bool p4 = true;

                auto expected = SampleFunction4(p1, p2, p3, p4);

                // Anything other than p1/p2/p3/p4 will do.
                s_charParameter1 = p1 + 1;
                s_intParameter2 = p2 + 1;
                s_int64Parameter3 = p3 + 1;
                s_boolParameter4 = !p4;
                s_sampleFunctionCalls = 0;
                auto observed = function(p4, p3, p2, p1);

                EXPECT_EQ(expected, observed);
                EXPECT_EQ(1, s_sampleFunctionCalls);
                EXPECT_EQ(s_charParameter1, p1);
                EXPECT_EQ(s_intParameter2, p2);
                EXPECT_EQ(s_int64Parameter3, p3);
                EXPECT_EQ(s_boolParameter4, p4);
            }
        }


        // Verifies that the references to stack variables are in a sane
        // memory range.
        // The *Internal method is needed because GTest requires a void method
        // and NativeJIT requires a method which returns a value (which is ignored
        // for this test).
        static void VerifyStackVariableAddressesInternal(uint32_t& intRef,
                                                         float& floatRef,
                                                         void const * addressOfReturnAddress)
        {
            // Since the stack pointer moves downwards, the upper limit of
            // this function's stack range represents the lower limit of
            // the previous function's stack range in the call stack.
            // The previous function is the jitted function whose stack
            // variable address we are trying to verify. The size of its
            // stack range is small (64 bytes or so), but more buffer
            // is added to account for different compilers and settings.
            // Note: it would also be possible to define the range as the
            // value between _AddressOfReturnAddress() as returned here
            // and _AddressOfReturnAddress() as returned by the caller of the
            // jitted function (that value would be passed as an argument).
            auto const bufferStart = static_cast<uint8_t const *>(addressOfReturnAddress);
            auto const bufferLimit = bufferStart + 1024;

            auto const intPtr = reinterpret_cast<unsigned char const *>(&intRef);
            auto const floatPtr = reinterpret_cast<unsigned char const *>(&floatRef);

            ASSERT_TRUE(intPtr >= bufferStart && intPtr < bufferLimit) <<
              "Expected range: [" <<
              bufferStart <<
              "," <<
              bufferLimit <<
              ") found:" <<
              intPtr;
            ASSERT_TRUE(floatPtr >= bufferStart && floatPtr < bufferLimit) <<
              "Expected range: [" <<
              bufferStart <<
              "," <<
              bufferLimit <<
              ") found:" <<
              floatPtr;
        }


        static int VerifyStackVariableAddresses(uint32_t& intRef, float& floatRef)
        {
            VerifyStackVariableAddressesInternal(intRef, floatRef, _AddressOfReturnAddress());

            return 1;
        }


        TEST_F(FunctionTest, StackVariableAddressRange)
        {
            auto setup = GetSetup();
            Function<int> e(setup->GetAllocator(), setup->GetCode());

            auto & sampleFunction = e.Immediate(VerifyStackVariableAddresses);

            auto & intVariable = e.StackVariable<uint32_t>();
            auto & floatVariable = e.StackVariable<float>();

            auto & call = e.Call(sampleFunction, intVariable, floatVariable);

            auto function = e.Compile(call);
            function();
        }


        // Verifies that pointer and reference arguments refer to the same
        // memory location and that it contains the expected value.
        // The *Internal method is needed because GTest requires a void method
        // and NativeJIT requires a method which returns a value (which is ignored
        // for this test).
        static void VerifyPointerVsReferenceInternal(uint32_t* intPtr,
                                                     uint32_t& intRef,
                                                     unsigned int expectedValue)
        {
            EXPECT_EQ(intPtr, &intRef) <<
                "Pointer and reference should have referred to the same address";
            EXPECT_EQ(expectedValue, intRef) << "Unexpected target value";
        }

        static int VerifyPointerVsReference(uint32_t* intPtr,
                                            uint32_t& intRef,
                                            unsigned int expectedValue)
        {
            VerifyPointerVsReferenceInternal(intPtr, intRef, expectedValue);

            return 1;
        }


        TEST_F(FunctionTest, PointerToReferenceConversion)
        {
            auto setup = GetSetup();
            Function<int> e(setup->GetAllocator(), setup->GetCode());

            uint32_t testValue = 7;

            auto & testFunction = e.Immediate(VerifyPointerVsReference);
            auto & intPtrArgument = e.Immediate(&testValue);
            auto & intRefArgument = e.AsReference(intPtrArgument);

            auto & call = e.Call(testFunction,
                                 intPtrArgument,
                                 intRefArgument,
                                 e.Immediate(testValue));

            auto function = e.Compile(call);

            function();
        }


        static int& Return10ByReference()
        {
            static int ten = 10;

            return ten;
        }


        TEST_F(FunctionTest, ReturnReference)
        {
            auto setup = GetSetup();
            Function<int&> e(setup->GetAllocator(), setup->GetCode());

            auto & return10 = e.Immediate(Return10ByReference);
            auto & call = e.Call(return10);
            auto function = e.Compile(call);

            int& result = function();

            EXPECT_EQ(10, result);
        }


        // Verify that the return register is preserved accross two
        // consecutive calls.

        TEST_F(FunctionTest, PreserveReturnRegisterInt)
        {
            auto setup = GetSetup();
            Function<int> e(setup->GetAllocator(), setup->GetCode());

            auto & return7 = e.Call(e.Immediate(ReturnInt<7>));
            auto & return8 = e.Call(e.Immediate(ReturnInt<8>));

            // Ensure that 7 is preserved before the call that would overwrite
            // it with 8.
            auto & sum = e.Add(return7, return8);
            auto function = e.Compile(sum);
            int result = function();

            EXPECT_EQ(7 + 8, result);
        }


        TEST_F(FunctionTest, PreserveReturnRegisterFloat)
        {
            auto setup = GetSetup();
            Function<float> e(setup->GetAllocator(), setup->GetCode());

            auto & return7Point7 = e.Call(e.Immediate(Return7Point7));
            auto & return3Point3 = e.Call(e.Immediate(Return3Point3));

            // Ensure that 7.7 is preserved before the call that would overwrite
            // it with 3.3.
            auto & sum = e.Add(return7Point7, return3Point3);
            auto function = e.Compile(sum);
            float result = function();

            EXPECT_EQ(7.7 + 3.3, result);
        }


        // Verify that the return register is treated volatile only when it's
        // actually used (i.e. don't treat EAX as volatile and presereve it
        // if a function returning in XMM0 is called).
        TEST_F(FunctionTest, SaveOtherReturnRegisterIntFloat)
        {
            s_regPreserveTestFuncCallCount = 0;

            auto setup = GetSetup();
            Function<int> e(setup->GetAllocator(), setup->GetCode());

            // Call the function returning 10 in EAX first, followed by
            // a function returning 1.3 in XMM0s. Since EAX will be referenced
            // later and it's not used for calling the float function, it
            // must be preserved accross that call (i.e. not treated as volatile).
            auto & return10 = e.Immediate(Return10<1>);
            auto & call10 = e.Call(return10);

            auto & return1Point3 = e.Immediate(Return1Point3<2>);
            auto & call1Point3 = e.Dependent(e.Call(return1Point3),
                                             call10);

            // Convert 1.3 to 1. This is to ensure that the call is referenced
            // and the value is then used since it's convenient.
            auto & int1 = e.Cast<int>(call1Point3);

            // Sum 10, 1 and 10, expecting to get 21. If the cached value
            // of 10 is not preserved correctly accross the call, the result
            // will be incorrect.
            auto & sum = e.Add(e.Add(call10, int1), call10);
            auto function = e.Compile(sum);
            int result = function();

            EXPECT_EQ(21, result);
        }


        // Verify that return register is treated volatile only when it's
        // actually used (i.e. don't treat EAX as volatile and preserve it
        // if a function returning in XMM0 is called).
        TEST_F(FunctionTest, SaveOtherReturnRegisterFloatInt)
        {
            s_regPreserveTestFuncCallCount = 0;

            auto setup = GetSetup();
            Function<float> e(setup->GetAllocator(), setup->GetCode());

            // Call the function returning 1.3 in XMM0s first, followed by
            // a function returning 10 in EAX. Since XMM0s will be referenced
            // later and it's not used for calling the int function, it
            // must be preserved accross that call (i.e. not treated as volatile).
            auto & return1Point3 = e.Immediate(Return1Point3<1>);
            auto & call1Point3 = e.Call(return1Point3);

            auto & return10 = e.Immediate(Return10<2>);
            auto & call10 = e.Dependent(e.Call(return10),
                                        call1Point3);

            // Convert 10 to 10.0. This is to ensure that the call is referenced
            // and the value is then used since it's convenient.
            auto & float10 = e.Cast<float>(call10);

            // Sum 1.3, 10.0 and 1.3, expecting to get 12.6. If the cached value
            // of 1.3 is not preserved correctly accross the call, the result
            // will be incorrect.
            auto & sum = e.Add(e.Add(call1Point3, float10), call1Point3);
            auto function = e.Compile(sum);
            float result = function();

            ASSERT_TRUE(std::abs(12.6 - result) < 0.01) <<
                "Result should be around 12.6, but found " <<
                result;
        }


        // These two functions are used to ensure zero is placed inside
        // ECX/XMM1s when they are called. It is important that they
        // are not inlined to achieve this.

#ifdef NATIVEJIT_PLATFORM_WINDOWS
        __declspec(noinline)
        static void TakeZeroIntArg(int arg)
#else
        static void __attribute__ ((noinline)) TakeZeroIntArg(int arg)
#endif
        {
            EXPECT_EQ(0, arg);
        }


#ifdef NATIVEJIT_PLATFORM_WINDOWS
        __declspec(noinline)
        static void TakeZeroFloatArg(float /* arg1 */, float arg2)
#else
        static void __attribute__ ((noinline)) TakeZeroFloatArg(float /* arg1 */, float arg2)
#endif
        {
            EXPECT_EQ(0.0f, arg2);
        }


        // These two helper functions return their argument and, as a
        // side-effect, ensure that zero is placed inside ECX/XMM0s to
        // demonstrate the volatility of these registers.

        static int ZeroRcxAndReturnArg(int arg)
        {
            TakeZeroIntArg(0);

            return arg;
        }


        static float ZeroXmm1AndReturnArg1(float arg1, float /* arg2 */)
        {
            TakeZeroFloatArg(0.0, 0.0);

            return arg1;
        }


        // When function parameter happens to be in the desired register,
        // if there are other references to it (f. ex. later in the expression),
        // it should be preserved.

        TEST_F(FunctionTest, VerifyFunctionParameterReuseInt)
        {
            auto setup = GetSetup();
            Function<int, int> e(setup->GetAllocator(), setup->GetCode());

            auto & zeroRcxAndReturnArg = e.Immediate(ZeroRcxAndReturnArg);

            // Param1 is in ECX which happens to be the register where CALL
            // needs it to be, but it needs to be re-used later as well.
            // Since RCX is volatile, the other instance must be preserved
            // before the call for the test to succeed.
            auto & param1 = e.GetP1();
            auto & call = e.Call(zeroRcxAndReturnArg, param1);

            auto & sum = e.Add(call, param1);

            auto function = e.Compile(sum);
            int result = function(7);

            // The first 7 comes from the return value from the call and the
            // other 7 comes from the preserved value of ECX.
            EXPECT_EQ(7 + 7, result);
        }


        TEST_F(FunctionTest, VerifyFunctionParameterReuseFloat)
        {
            auto setup = GetSetup();
            Function<float, float, float> e(setup->GetAllocator(), setup->GetCode());

            auto & zeroXmm1AndReturnArg1 = e.Immediate(ZeroXmm1AndReturnArg1);

            // Param2 is in XMM1s which happens to be the register where CALL
            // needs it to be, but it needs to be re-used later as well.
            // Since XMM1s is volatile, the other instance must be preserved
            // before the call for the test to succeed.
            auto & param2 = e.GetP2();
            auto & call = e.Call(zeroXmm1AndReturnArg1, e.GetP1(), param2);

            auto & sum = e.Add(call, param2);

            auto function = e.Compile(sum);
            float result = function(2.5f, 7.5f);

            // The 2.5 value comes from the return value from the call and
            // 7.5 comes from the preserved value of XMM1s.
            EXPECT_EQ(2.5f + 7.5f, result);
        }


        // Verify that no asserts are hit when using a function which doesn't
        // reference some of its parameters.
        TEST_F(FunctionTest, FunctionWithUnusedParameter)
        {
            auto setup = GetSetup();
            Function<int64_t, int64_t, int64_t> expression(setup->GetAllocator(), setup->GetCode());

            auto function = expression.Compile(expression.GetP2());

            int64_t p1Unused = 0;
            int64_t p2 = 123;

            auto observed = function(p1Unused, p2);

            EXPECT_EQ(p2, observed);
        }


        TEST_F(FunctionTest, ExecuteOnlyIf)
        {
            auto setup = GetSetup();

            Function<float, uint64_t> e(setup->GetAllocator(), setup->GetCode());

            // Regular value is argument + 2.5.
            auto & regularValue = e.Add(e.Cast<float>(e.GetP1()),
                                        e.Immediate(2.5f));

            // Execute the regular expression only if argument is less than
            // 7, otherwise abort and return 0.
            auto & argLessThan7 = e.Compare<JccType::JB>(e.GetP1(), e.Immediate(static_cast<uint64_t>(7)));
            e.AddExecuteOnlyIfStatement(argLessThan7, e.Immediate(0.0f));

            auto function = e.Compile(regularValue);

            // Argument is less than 7, the regular value should be returned.
            auto observed = function(5);
            EXPECT_EQ(5.0f + 2.5f, observed);

            // Argument is not less than 7, 0 should be returned.
            observed = function(10);

            EXPECT_EQ(0.0f, observed);
        }

        TEST_CASES_END

        int FunctionTest::s_sampleFunctionCalls;

        int FunctionTest::s_intParameter1;
        int FunctionTest::s_intParameter2;

        char FunctionTest::s_charParameter1;

        int64_t FunctionTest::s_int64Parameter3;

        bool FunctionTest::s_boolParameter4;

        unsigned FunctionTest::s_regPreserveTestFuncCallCount;
    }
}
