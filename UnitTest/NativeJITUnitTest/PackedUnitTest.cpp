#include "stdafx.h"


#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/Function.h"
#include "NativeJIT/Model.h"
#include "NativeJIT/Packed.h"
#include "Temporary/Allocator.h"
#include "TestSetup.h"


namespace NativeJIT
{
    namespace PackedUnitTest
    {
        TEST_FIXTURE_START(PackedTest)
        protected:
            typedef Packed<3, 4, 5> PackedType;

            PackedType MakePacked(uint8_t threeBitValue,
                                  uint8_t fourBitValue,
                                  uint8_t fiveBitValue)
            {
                return PackedType::FromComponents(threeBitValue, fourBitValue, fiveBitValue);
            }

        TEST_FIXTURE_END_TEST_CASES_BEGIN


        TEST_CASE_F(PackedTest, Basic)
        {
            auto setup = GetSetup();

            {
                // Initialization from bits.
                const auto packed1FromBits = Packed<8>::FromBits(0xE0);
                const auto packed2FromBits = Packed<4, 8>::FromBits(0x2E0);

                TestEqual(0xE0u, packed1FromBits.m_bits);
                TestEqual(0x2E0u, packed2FromBits.m_bits);

                // Operations on a single-component packed.
                const auto packed1 = Packed<8>::FromComponents(0xE0);

                static_assert(1 == packed1.c_componentCount, "Invalid component count");
                static_assert(8 == packed1.c_totalBitCount, "Invalid total bit count");
                static_assert(8 == packed1.c_leftmostBitCount, "Invalid leftmost bit count");
                static_assert(8 == packed1.c_rightmostBitCount, "Invalid rightmost bit count");

                TestEqual(0xE0u, packed1.m_bits);
                TestEqual(0xE0u, packed1.Leftmost());
                TestEqual(0xE0u, packed1.Rightmost());

                // No WithoutLeftmost() operation available.
                TestEqual(0x1E0u, packed1.InsertLeftmost<4>(0x1).m_bits);
                TestEqual(0xE03u, packed1.InsertRightmost<4>(0x3).m_bits);

                // Operations on two-component packed.
                const auto packed2 = Packed<4, 8>::FromComponents(0x2, 0xE0);

                static_assert(2 == packed2.c_componentCount, "Invalid component count");
                static_assert(12 == packed2.c_totalBitCount, "Invalid total bit count");
                static_assert(4 == packed2.c_leftmostBitCount, "Invalid leftmost bit count");
                static_assert(8 == packed2.c_rightmostBitCount, "Invalid rightmost bit count");

                TestEqual(0x2E0u, packed2.m_bits);
                TestEqual(0x2u, packed2.Leftmost());
                TestEqual(0xE0u, packed2.Rightmost());

                TestEqual(0xE0u, packed2.WithoutLeftmost());
                TestEqual(0x12E0u, packed2.InsertLeftmost<4>(0x1).m_bits);
                TestEqual(0x2E01u, packed2.InsertRightmost<4>(0x1).m_bits);
            }
        }


        TEST_CASE_F(PackedTest, ModelApply)
        {
            auto setup = GetSetup();

            {
                typedef Model<PackedType> ModelType;

                Function<float, ModelType*, PackedType> expression(setup->GetAllocator(), setup->GetCode());

                auto & a = expression.ApplyModel(expression.GetP1(), expression.GetP2());

                auto function = expression.Compile(a);

                ModelType model;
                auto packed = MakePacked(7, 6, 5);

                auto expected = model.Apply(packed);
                auto observed = function(&model, packed);

                TestAssert(observed == expected);
            }
        }


        TEST_CASE_F(PackedTest, PackedMax)
        {
            auto setup = GetSetup();

            PackedType packed1 = MakePacked((1 << 3) - 1, 1, (1 << 5) - 1);
            PackedType packed2 = MakePacked(1, (1 << 4) - 1, 1);
            PackedType expected = MakePacked((1 << 3) - 1, (1 << 4) - 1, (1 << 5) - 1);

            Function<PackedType, PackedType, PackedType> expression(setup->GetAllocator(), setup->GetCode());

            auto & a = expression.PackedMax(expression.GetP1(), expression.GetP2());;
            auto function = expression.Compile(a);

            auto observed = function(packed1, packed2);

            TestEqual(expected.m_bits, observed.m_bits);
        }


        TEST_CASE_F(PackedTest, PackedMin)
        {
            auto setup = GetSetup();

            PackedType packed1 = MakePacked((1 << 3) - 1, 1, (1 << 5) - 1);
            PackedType packed2 = MakePacked(1, (1 << 4) - 1, 1);
            PackedType expected = MakePacked(1, 1, 1);

            Function<PackedType, PackedType, PackedType> expression(setup->GetAllocator(), setup->GetCode());

            auto & a = expression.PackedMin(expression.GetP1(), expression.GetP2());;
            auto function = expression.Compile(a);

            auto observed = function(packed1, packed2);

            TestEqual(expected.m_bits, observed.m_bits);
        }

        TEST_CASES_END
    }
}
