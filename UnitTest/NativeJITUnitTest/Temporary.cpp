#include "stdafx.h"


#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"



namespace NativeJIT
{
    //*************************************************************************
    //
    // This file contains temporary unit test used during development of
    // partially implemented features. The intention is that this file will
    // eventually be deleted.
    //
    //*************************************************************************
    namespace TemporaryUnitTest2
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


            TestCase(Temporary)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    Function<__int64> expression(m_allocator, *m_code);

                    auto x = expression.Direct<int>();
                    auto y = expression.Direct<float>();

                    __int64 expected = 1234ll;
                    auto & a = expression.Immediate(expected);
                    auto function = expression.Compile(a);

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
