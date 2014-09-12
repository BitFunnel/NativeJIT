#include "stdafx.h"


#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "NativeJIT/Packed.h"
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
    namespace PackedUnitTest2
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


            TestCase(PackedMax)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    typedef Packed<3, Packed<4, Packed<5>>> PackedType;
                    size_t x = sizeof(PackedType);
                    x += 0;

                    Function<PackedType, PackedType, PackedType> expression(m_allocator, *m_code);

                    auto & a = expression.MinMax(expression.GetP1(), expression.GetP2());

                    auto function = expression.Compile(a);

                    auto left = Packed<>::Push<5>(5).Push<4>(6).Push<3>(7);
                    auto right = Packed<>::Push<5>(2).Push<4>(9).Push<3>(1);

                    auto expected = Packed<5>::Create(5).Push<4>(9).Push<3>(7);
                    auto observed = function(left, right);

                    TestAssert(observed.GetBits() == expected.GetBits());
                }
            }


        private:
            Allocator m_allocator;
            ExecutionBuffer m_executionBuffer;
            std::unique_ptr<FunctionBuffer> m_code;
        };
    }
}
