#include "stdafx.h"


#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "NativeJIT/Packed.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"

#include "NativeJIT/Model.h"


namespace NativeJIT
{
    //*************************************************************************
    //
    // This file contains temporary unit test used during development of
    // partially implemented features. The intention is that this file will
    // eventually be deleted.
    //
    //*************************************************************************
    namespace PackedUnitTest6
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


            //TestCase(Model)
            //{
            //    typedef Packed<3, Packed<4, Packed<5>>> PackedType;
            //    auto a = Packed<>::Push<5>(5).Push<4>(6).Push<3>(7);
            //    Model<PackedType> model;

            //    float x = model.Apply(a);
            //    std::cout << x << std::endl;
            //}


            TestCase(ModelApply)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    typedef Packed<3, Packed<4, Packed<5>>> PackedType;
                    typedef Model<PackedType> ModelType;

                    Function<float, ModelType*, PackedType> expression(m_allocator, *m_code);

                    auto & a = expression.ApplyModel(expression.GetP1(), expression.GetP2());

                    auto function = expression.Compile(a);

                    ModelType model;
                    auto packed = Packed<>::Push<5>(5).Push<4>(6).Push<3>(7);

                    auto expected = model.Apply(packed);
                    auto observed = function(&model, packed);

                    TestAssert(observed == expected);
                }
            }


            //TestCase(PackedMax)
            //{
            //    AutoResetAllocator reset(m_allocator);

            //    {
            //        typedef Packed<3, Packed<4, Packed<5>>> PackedType;
            //        size_t x = sizeof(PackedType);
            //        x += 0;

            //        Function<PackedType, PackedType, PackedType> expression(m_allocator, *m_code);

            //        auto & a = expression.PackedMax(expression.GetP1(), expression.GetP2());

            //        auto function = expression.Compile(a);

            //        auto left = Packed<>::Push<5>(5).Push<4>(6).Push<3>(7);
            //        auto right = Packed<>::Push<5>(2).Push<4>(9).Push<3>(1);

            //        auto expected = Packed<5>::Create(5).Push<4>(9).Push<3>(7);
            //        auto observed = function(left, right);

            //        TestAssert(observed.GetBits() == expected.GetBits());
            //    }
            //}


            //TestCase(PackedMin)
            //{
            //    AutoResetAllocator reset(m_allocator);

            //    {
            //        typedef Packed<3, Packed<4, Packed<5>>> PackedType;
            //        size_t x = sizeof(PackedType);
            //        x += 0;

            //        Function<PackedType, PackedType, PackedType> expression(m_allocator, *m_code);

            //        auto & a = expression.PackedMin(expression.GetP1(), expression.GetP2());

            //        auto function = expression.Compile(a);

            //        auto left = Packed<>::Push<5>(5).Push<4>(6).Push<3>(7);
            //        auto right = Packed<>::Push<5>(2).Push<4>(9).Push<3>(1);

            //        auto expected = Packed<5>::Create(2).Push<4>(6).Push<3>(1);
            //        auto observed = function(left, right);

            //        TestAssert(observed.GetBits() == expected.GetBits());
            //    }
            //}


        private:
            Allocator m_allocator;
            ExecutionBuffer m_executionBuffer;
            std::unique_ptr<FunctionBuffer> m_code;
        };
    }
}
