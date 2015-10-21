#include "stdafx.h"

#include <memory>

#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"

#include "CallNode.h"

namespace NativeJIT
{
    namespace FunctionUnitTest
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


            //
            // TODO:
            //   Test register save/restore across call.
            //      Caller save volatiles
            //      Callee save non-volatiles
            //   Test call register indirect.
            //   Test throws from within generated code.
            //   Test that tries all permutations of parameter from Function to Call.


            //
            // JIT Functions with 0, 1, 2, 3, and 4 parameters.
            //

            TestCase(FunctionZeroParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<__int64> expression(m_allocator, *m_code);

                    __int64 expected = 1234ll;
                    auto & a = expression.Immediate(expected);
                    auto function = expression.Compile(a);

                    auto observed = function();

                    TestAssert(observed == expected);
                }
            }


            TestCase(FunctionOneParameter)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<__int64, __int64> expression(m_allocator, *m_code);

                    auto & a = expression.GetP1();
                    auto function = expression.Compile(a);

                    __int64 p1 = 1234ll;

                    auto expected = p1;
                    auto observed = function(p1);

                    TestAssert(observed == expected);
                }
            }


            TestCase(FunctionTwoParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<__int64, __int64, __int64> expression(m_allocator, *m_code);

                    auto & a = expression.Add(expression.GetP1(), expression.GetP2());
                    auto function = expression.Compile(a);

                    __int64 p1 = 12340000ll;
                    __int64 p2 = 5678ll;

                    auto expected = p1 + p2;
                    auto observed = function(p1, p2);

                    TestAssert(observed == expected);
                }
            }


            TestCase(FunctionThreeParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<int, int, int, int> expression(m_allocator, *m_code);

                    auto & a = expression.Sub(expression.GetP2(), expression.GetP1());
                    auto & b = expression.Add(expression.GetP3(), a);
                    auto function = expression.Compile(b);

                    int p1 = 0x12340000;
                    int p2 = 0x5678;
                    int p3 = 0x11111111;

                    auto expected = p2 - p1 + p3;
                    auto observed = function(p1, p2, p3);

                    TestAssert(observed == expected);
                }
            }


            TestCase(FunctionFourParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<char, char, char, char, char> expression(m_allocator, *m_code);

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

                    TestAssert(observed == expected);
                }
            }


            //
            // Calling C functions with 0, 1, 2, 3, and 4 parameters.
            //

            static int s_sampleFunctionCalls;
            static int s_intParameter1;
            static int s_intParameter2;

            static char s_charParameter1;

            static __int64 s_int64Parameter3;

            static bool s_boolParameter4;


            static int SampleFunction0()
            {
                ++FunctionTest::s_sampleFunctionCalls;
                return 12345;
            }


            static int SampleFunction1(char p1)
            {
                TestAssert(p1 == s_charParameter1);
                ++FunctionTest::s_sampleFunctionCalls;
                return p1 + 3;
            }


            static int SampleFunction2(int p1, int p2)
            {
                TestAssert(p1 == s_intParameter1);
                TestAssert(p2 == s_intParameter2);
                ++FunctionTest::s_sampleFunctionCalls;
                return p1 + p2;
            }


            static __int64 SampleFunction3(char p1, int p2, __int64 p3)
            {
                TestAssert(p1 == s_charParameter1);
                TestAssert(p2 == s_intParameter2);
                TestAssert(p3 == s_int64Parameter3);
                ++FunctionTest::s_sampleFunctionCalls;
                return p1 + p2 + p3;
            }


            static __int64 SampleFunction3(char p1, int p2, __int64 p3, bool p4)
            {
                TestAssert(p1 == s_charParameter1);
                TestAssert(p2 == s_intParameter2);
                TestAssert(p3 == s_int64Parameter3);
                TestAssert(p4 == s_boolParameter4);
                ++FunctionTest::s_sampleFunctionCalls;
                return p1 + p2 + p3 + (p4 ? 123 : 456);
            }


            TestCase(CallZeroParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<int> expression(m_allocator, *m_code);

                    typedef int (*F)();
                    auto & sampleFunction = expression.Immediate<F>(SampleFunction0);
                    auto & a = expression.Call(sampleFunction);
                    auto function = expression.Compile(a);

                    auto expected = SampleFunction0();

                    FunctionTest::s_sampleFunctionCalls = 0;
                    auto observed = function();

                    TestAssert(observed == expected);
                    TestAssert(FunctionTest::s_sampleFunctionCalls == 1);
                }
            }


            TestCase(CallOneParameter)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<int, char> expression(m_allocator, *m_code);

                    typedef int (*F)(char);
                    auto & sampleFunction = expression.Immediate<F>(SampleFunction1);
                    auto & a = expression.Call(sampleFunction, expression.GetP1());
                    auto function = expression.Compile(a);

                    char& p1 = s_charParameter1;;

                    p1 = 0x73;

                    auto expected = SampleFunction1(p1);

                    FunctionTest::s_sampleFunctionCalls = 0;
                    auto observed = function(p1);

                    TestAssert(observed == expected);
                    TestAssert(FunctionTest::s_sampleFunctionCalls == 1);
                }
            }


            TestCase(CallTwoParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<int, int, int> expression(m_allocator, *m_code);

                    typedef int (*F)(int, int);
                    auto & sampleFunction = expression.Immediate<F>(SampleFunction2);
                    auto & a = expression.Call(sampleFunction, expression.GetP2(), expression.GetP1());
                    auto function = expression.Compile(a);

                    int& p1 = s_intParameter2;
                    int& p2 = s_intParameter1;

                    p1 = 12340000ll;
                    p2 = 5678ll;
                    p1 = 0x73;

                    auto expected = SampleFunction2(p2, p1);

                    FunctionTest::s_sampleFunctionCalls = 0;
                    auto observed = function(p1, p2);

                    TestAssert(observed == expected);
                    TestAssert(FunctionTest::s_sampleFunctionCalls == 1);
                }
            }


            TestCase(CallThreeParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<__int64, __int64, int, char> expression(m_allocator, *m_code);

                    typedef __int64 (*F)(char, int, __int64);
                    auto & sampleFunction = expression.Immediate<F>(SampleFunction3);
                    auto & a = expression.Call(sampleFunction,
                                               expression.GetP3(),
                                               expression.GetP2(),
                                               expression.GetP1());
                    auto function = expression.Compile(a);

                    char& p1 = s_charParameter1;
                    int& p2 = s_intParameter2;
                    __int64& p3 = s_int64Parameter3;

                    p1 = 0x73;
                    p2 = 5678;
                    p3 = 12340000ll;

                    auto expected = SampleFunction3(p1, p2, p3);

                    FunctionTest::s_sampleFunctionCalls = 0;
                    auto observed = function(p3, p2, p1);

                    TestAssert(observed == expected);
                    TestAssert(FunctionTest::s_sampleFunctionCalls == 1);
                }
            }


            TestCase(CallFourParameters)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<__int64, bool, __int64, int, char> expression(m_allocator, *m_code);

                    typedef __int64 (*F)(char, int, __int64, bool);
                    auto & sampleFunction = expression.Immediate<F>(SampleFunction3);
                    auto & a = expression.Call(sampleFunction,
                                               expression.GetP4(),
                                               expression.GetP3(),
                                               expression.GetP2(),
                                               expression.GetP1());
                    auto function = expression.Compile(a);

                    char& p1 = s_charParameter1;
                    int& p2 = s_intParameter2;
                    __int64& p3 = s_int64Parameter3;
                    bool& p4 = s_boolParameter4;

                    p1 = 0x73;
                    p2 = 5678;
                    p3 = 12340000ll;
                    p4 = true;

                    auto expected = SampleFunction3(p1, p2, p3, p4);

                    FunctionTest::s_sampleFunctionCalls = 0;
                    auto observed = function(p4, p3, p2, p1);

                    TestAssert(observed == expected);
                    TestAssert(FunctionTest::s_sampleFunctionCalls == 1);
                }
            }


            // Verifies that the references to stack variables are in a sane
            // memory range.
            static int VerifyStackVariableAddresses(unsigned __int32& intRef, float& floatRef)
            {
                // Since the stack pointer moves downwards, the upper limit of
                // this function's stack range represents the lower limit of
                // the previous function's stack range in the call stack.
                // The previous function is the jitted function whose stack
                // variable address we are trying to verify. The size of its
                // stack range is roughly estimated to be 64 bytes.
                // Note: it would also be possible to define the range as the
                // value between _AddressOfReturnAddress() as returned here
                // and _AddressOfReturnAddress() as returned by the caller of the
                // jitted function (that value would be passed as an argument).
                auto bufferStart = static_cast<unsigned __int8 const *>(_AddressOfReturnAddress());
                auto bufferLimit = bufferStart + 96;

                auto intPtr = reinterpret_cast<unsigned char*>(&intRef);
                auto floatPtr = reinterpret_cast<unsigned char*>(&floatRef);

                TestAssert(intPtr >= bufferStart && intPtr < bufferLimit,
                           "Expected range: [%I64x, %I64x), found: %I64x",
                           bufferStart,
                           bufferLimit,
                           intPtr);
                TestAssert(floatPtr >= bufferStart && floatPtr < bufferLimit,
                           "Expected range: [%I64x, %I64x), found: %I64x",
                           bufferStart,
                           bufferLimit,
                           floatPtr);

                return 1;
            }


            TestCase(StackVariableAddressRange)
            {
                AutoResetAllocator reset(m_allocator);
                Function<int> e(m_allocator, *m_code);

                auto & sampleFunction = e.Immediate(VerifyStackVariableAddresses);

                auto & intVariable = e.StackVariable<unsigned __int32>();
                auto & floatVariable = e.StackVariable<float>();

                auto & call = e.Call(sampleFunction, intVariable, floatVariable);

                auto function = e.Compile(call);
                function();
            }


            // Verifies that pointer and reference arguments refer to the same
            // memory location and that it contains the expected value.
            static int VerifyPointerVsReference(unsigned __int32* intPtr,
                                                unsigned __int32& intRef,
                                                unsigned int expectedValue)
            {
                TestEqual(intPtr, &intRef, "Pointer and reference should have referred to the same address");
                TestEqual(expectedValue, intRef, "Unexpected target value");

                return 1;
            }


            TestCase(PointerToReferenceConversion)
            {
                AutoResetAllocator reset(m_allocator);
                Function<int> e(m_allocator, *m_code);

                unsigned __int32 testValue = 7;

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


            TestCase(ReturnReference)
            {
                AutoResetAllocator reset(m_allocator);
                Function<int&> e(m_allocator, *m_code);

                auto & return10 = e.Immediate(Return10ByReference);
                auto & call = e.Call(return10);
                auto function = e.Compile(call);

                int& result = function();

                TestEqual(10, result);
            }


            static unsigned s_regPreserveTestFuncCallCount;

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

            template <unsigned CALLNUMBER>
            static int Return10()
            {
                ++s_regPreserveTestFuncCallCount;
                TestEqual(CALLNUMBER, s_regPreserveTestFuncCallCount,
                          "Return10 must be call #%u", CALLNUMBER);

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
                TestEqual(CALLNUMBER, s_regPreserveTestFuncCallCount,
                          "Return1Point3 must be call #%u", CALLNUMBER);

                // Call any function which returns an integer other than 10 used
                // in the function above to make sure eax is overwritten.
                // This will ensure that the test will succeed only if the caller
                // which needs to re-use the original value correctly preserved it.
                ReturnInt<999>();

                return 1.3f;
            }


            // Verify that the return register is preserved accross two
            // consecutive calls.

            TestCase(PreserveReturnRegisterInt)
            {
                AutoResetAllocator reset(m_allocator);
                Function<int> e(m_allocator, *m_code);

                auto & return7 = e.Call(e.Immediate(ReturnInt<7>));
                auto & return8 = e.Call(e.Immediate(ReturnInt<8>));

                // Ensure that 7 is preserved before the call that would overwrite
                // it with 8.
                auto & sum = e.Add(return7, return8);
                auto function = e.Compile(sum);
                int result = function();

                TestEqual(7 + 8, result);
            }


            TestCase(PreserveReturnRegisterFloat)
            {
                AutoResetAllocator reset(m_allocator);
                Function<float> e(m_allocator, *m_code);

                auto & return7Point7 = e.Call(e.Immediate(Return7Point7));
                auto & return3Point3 = e.Call(e.Immediate(Return3Point3));

                // Ensure that 7.7 is preserved before the call that would overwrite
                // it with 3.3.
                auto & sum = e.Add(return7Point7, return3Point3);
                auto function = e.Compile(sum);
                float result = function();

                TestEqual(7.7 + 3.3, result);
            }


            // Verify that the return register is treated volatile only when it's
            // actually used (i.e. don't treat EAX as volatile and presereve it
            // if a function returning in XMM0 is called).
            TestCase(SaveOtherReturnRegisterIntFloat)
            {
                s_regPreserveTestFuncCallCount = 0;

                AutoResetAllocator reset(m_allocator);
                Function<int> e(m_allocator, *m_code);

                // Call the function returning 10 in EAX first, followed by
                // a function returning 1.3 in XMM0s. Since EAX will be referenced
                // later and it's not used for calling the float function, it
                // must be preserved accross that call (i.e. not treated as volatile).
                auto & return10 = e.Immediate(Return10<1>);
                auto & call10 = e.Call(return10);

                auto & return1Point3 = e.Immediate(Return1Point3<2>);
                auto & call1Point3 = e.Call(return1Point3);

                // Convert 1.3 to 1. This is to ensure that the call is referenced
                // and the value is then used since it's convenient.
                auto & int1 = e.Cast<int>(call1Point3);

                // Sum 10, 1 and 10, expecting to get 21. If the cached value
                // of 10 is not preserved correctly accross the call, the result
                // will be incorrect.
                auto & sum = e.Add(e.Add(call10, int1), call10);
                auto function = e.Compile(sum);
                int result = function();

                TestEqual(21, result);
            }


            // Verify that return register is treated volatile only when it's
            // actually used (i.e. don't treat EAX as volatile and preserve it
            // if a function returning in XMM0 is called).
            TestCase(SaveOtherReturnRegisterFloatInt)
            {
                s_regPreserveTestFuncCallCount = 0;

                AutoResetAllocator reset(m_allocator);
                Function<float> e(m_allocator, *m_code);

                // Call the function returning 1.3 in XMM0s first, followed by
                // a function returning 10 in EAX. Since XMM0s will be referenced
                // later and it's not used for calling the int function, it
                // must be preserved accross that call (i.e. not treated as volatile).
                auto & return1Point3 = e.Immediate(Return1Point3<1>);
                auto & call1Point3 = e.Call(return1Point3);

                auto & return10 = e.Immediate(Return10<2>);
                auto & call10 = e.Call(return10);

                // Convert 10 to 10.0. This is to ensure that the call is referenced
                // and the value is then used since it's convenient.
                auto & float10 = e.Cast<float>(call10);

                // Sum 1.3, 10.0 and 1.3, expecting to get 12.6. If the cached value
                // of 1.3 is not preserved correctly accross the call, the result
                // will be incorrect.
                auto & sum = e.Add(e.Add(call1Point3, float10), call1Point3);
                auto function = e.Compile(sum);
                float result = function();

                TestAssert(std::fabs(12.6 - result) < 0.01,
                           "Result should be around 12.6, but found %.2f",
                           result);
            }


            // These two functions are used to ensure zero is placed inside
            // ECX/XMM1s when they are called. It is important that they
            // are not inlined to achieve this.

            __declspec(noinline)
            static void TakeZeroIntArg(int arg)
            {
                TestEqual(0, arg);
            }


            __declspec(noinline)
            static void TakeZeroFloatArg(float /* arg1 */, float arg2)
            {
                TestEqual(0.0f, arg2);
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

            TestCase(VerifyFunctionParameterReuseInt)
            {
                AutoResetAllocator reset(m_allocator);
                Function<int, int> e(m_allocator, *m_code);

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
                TestEqual(7 + 7, result);
            }


            TestCase(VerifyFunctionParameterReuseFloat)
            {
                AutoResetAllocator reset(m_allocator);
                Function<float, float, float> e(m_allocator, *m_code);

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
                TestEqual(2.5f + 7.5f, result);
            }


            // Verify that no asserts are hit when using a function which doesn't
            // reference some of its parameters.
            TestCase(FunctionWithUnusedParameter)
            {
                AutoResetAllocator reset(m_allocator);
                Function<__int64, __int64, __int64> expression(m_allocator, *m_code);

                auto function = expression.Compile(expression.GetP2());

                __int64 p1Unused = 0;
                __int64 p2 = 123;

                auto observed = function(p1Unused, p2);

                TestEqual(p2, observed);
            }



        private:
            Allocator m_allocator;
            ExecutionBuffer m_executionBuffer;
            std::unique_ptr<FunctionBuffer> m_code;
        };

        int FunctionTest::s_sampleFunctionCalls;

        int FunctionTest::s_intParameter1;
        int FunctionTest::s_intParameter2;

        char FunctionTest::s_charParameter1;

        __int64 FunctionTest::s_int64Parameter3;

        bool FunctionTest::s_boolParameter4;

        unsigned FunctionTest::s_regPreserveTestFuncCallCount;
    }
}
