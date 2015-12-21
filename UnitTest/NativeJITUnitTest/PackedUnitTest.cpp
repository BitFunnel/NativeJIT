#include "stdafx.h"


#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "NativeJIT/Model.h"
#include "NativeJIT/Packed.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"
#include "TestSetup.h"


namespace NativeJIT
{
    //*************************************************************************
    //
    // This file contains temporary unit test used during development of
    // partially implemented features. The intention is that this file will
    // eventually be deleted.
    //
    //*************************************************************************
    namespace PackedUnitTest
    {
        TestClass(FunctionTest), private TestClassSetup
        {
        public:
            TestCase(ModelApply)
            {
                auto setup = GetSetup();

                {
                    typedef Packed<3, Packed<4, Packed<5>>> PackedType;
                    typedef Model<PackedType> ModelType;

                    Function<float, ModelType*, PackedType> expression(setup->GetAllocator(), setup->GetCode());

                    auto & a = expression.ApplyModel(expression.GetP1(), expression.GetP2());

                    auto function = expression.Compile(a);

                    ModelType model;
                    auto packed = Packed<>::Push<5>(5).Push<4>(6).Push<3>(7);

                    auto expected = model.Apply(packed);
                    auto observed = function(&model, packed);

                    TestAssert(observed == expected);
                }
            }


            TestCase(PackedMax)
            {
                auto setup = GetSetup();

                PackedType packed1 = MakePacked((1 << 3) - 1, 1, (1 << 5) - 1);
                PackedType packed2 = MakePacked(1, (1 << 4) - 1, 1);
                PackedType expected = MakePacked((1 << 3) - 1, (1 << 4) - 1, (1 << 5) - 1);

                Function<PackedType, PackedType, PackedType> expression(setup->GetAllocator(), setup->GetCode());

                auto & a = expression.PackedMax(expression.GetP1(), expression.GetP2());;
                auto function = expression.Compile(a);

                auto observed = function(packed1, packed2);

                TestEqual(expected.m_fields, observed.m_fields);
            }


            TestCase(PackedMin)
            {
                auto setup = GetSetup();

                PackedType packed1 = MakePacked((1 << 3) - 1, 1, (1 << 5) - 1);
                PackedType packed2 = MakePacked(1, (1 << 4) - 1, 1);
                PackedType expected = MakePacked(1, 1, 1);

                Function<PackedType, PackedType, PackedType> expression(setup->GetAllocator(), setup->GetCode());

                auto & a = expression.PackedMin(expression.GetP1(), expression.GetP2());;
                auto function = expression.Compile(a);

                auto observed = function(packed1, packed2);

                TestEqual(expected.m_fields, observed.m_fields);
            }


        private:
            typedef Packed<3, Packed<4, Packed<5>>> PackedType;

            PackedType MakePacked(unsigned __int8 threeBitValue,
                                  unsigned __int8 fourBitValue,
                                  unsigned __int8 fiveBitValue)
            {
                return Packed<>::Push<5>(fiveBitValue).Push<4>(fourBitValue).Push<3>(threeBitValue);
            }
        };
    }
}
