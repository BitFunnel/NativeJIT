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


#include "NativeJIT/BitOperations.h"
#include "Temporary/Allocator.h"
#include "TestSetup.h"

#include <iostream>


namespace NativeJIT
{
    namespace BitOperationsUnitTest
    {
        TEST(BitOperations, NonZeroBitCount)
        {
            ASSERT_EQ(0, BitOp::GetNonZeroBitCount(0u));
            ASSERT_EQ(0, BitOp::GetNonZeroBitCount(static_cast<uint64_t>(0)));

            ASSERT_EQ(1, BitOp::GetNonZeroBitCount(1u));
            ASSERT_EQ(1, BitOp::GetNonZeroBitCount(0x80000000u));
            ASSERT_EQ(1, BitOp::GetNonZeroBitCount(static_cast<uint64_t>(0x8000000000000000)));
            ASSERT_EQ(32, BitOp::GetNonZeroBitCount(0xFFFFFFFFu));

            ASSERT_EQ(64, BitOp::GetNonZeroBitCount(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFF)));
        }

        void VerifyLowestBitSet(uint64_t testValue, unsigned expected)
        {
            ASSERT_NE(0u, testValue);

            unsigned actual;
            const bool foundBit = BitOp::GetLowestBitSet(testValue, &actual);

            ASSERT_TRUE(foundBit) << "Expected to find a bit in " << testValue;
            ASSERT_EQ(expected, actual) <<
                "Mismatched expected and actual bit for " << testValue;
        }

        TEST(BitOperations, LowestBitSet)
        {
            unsigned unused;
            ASSERT_FALSE(BitOp::GetLowestBitSet(0, &unused));

            VerifyLowestBitSet(1, 0);
            VerifyLowestBitSet(0xFFFFFFFFFFFFFFFF, 0);
            VerifyLowestBitSet(0x8000000000000000, 63);
        }


        void VerifyHighestBitSet(uint64_t testValue, unsigned expected)
        {
            ASSERT_NE(0u, testValue);

            unsigned actual;
            const bool foundBit = BitOp::GetHighestBitSet(testValue, &actual);

            ASSERT_TRUE(foundBit) << "Expected to find a bit in " << testValue;
            ASSERT_EQ(expected, actual) <<
                "Mismatched expected and actual bit for " << testValue;
        }


        TEST(BitOperations, HighestBitSet)
        {
            unsigned unused;
            ASSERT_FALSE(BitOp::GetHighestBitSet(0, &unused));

            VerifyHighestBitSet(1, 0);
            VerifyHighestBitSet(0x8000000000000000, 63);
            VerifyHighestBitSet(0xFFFFFFFFFFFFFFFF, 63);
        }


        TEST(BitOperations, TestBit)
        {
            const uint64_t bits62and63 = 0xC000000000000000;

            ASSERT_FALSE(BitOp::TestBit(bits62and63, 0));
            ASSERT_FALSE(BitOp::TestBit(bits62and63, 1));
            ASSERT_TRUE(BitOp::TestBit(bits62and63, 62));
            ASSERT_TRUE(BitOp::TestBit(bits62and63, 63));

            const uint64_t allZeros = 0;
            const uint64_t allOnes = 0xFFFFFFFFFFFFFFFF;

            for (unsigned int bit = 0; bit < 64; ++bit)
            {
                ASSERT_FALSE(BitOp::TestBit(allZeros, bit));
                ASSERT_TRUE(BitOp::TestBit(allOnes, bit));
            }
        }


        TEST(BitOperations, SetBit)
        {
            uint64_t currentValue = 0;
            uint64_t allOnes = 0xFFFFFFFFFFFFFFFF;

            for (unsigned int bit = 0; bit < 64; ++bit)
            {
                ASSERT_FALSE(BitOp::TestBit(currentValue, bit));
                BitOp::SetBit(&currentValue, bit);
                ASSERT_TRUE(BitOp::TestBit(currentValue, bit));

                ASSERT_TRUE(BitOp::TestBit(allOnes, bit));
                BitOp::SetBit(&allOnes, bit);
                ASSERT_TRUE(BitOp::TestBit(allOnes, bit));
            }
        }


        TEST(BitOperations, ClearBit)
        {
            uint64_t currentValue = 0xFFFFFFFFFFFFFFFF;
            uint64_t allZeros = 0;

            for (unsigned int bit = 0; bit < 64; ++bit)
            {
                ASSERT_TRUE(BitOp::TestBit(currentValue, bit));
                BitOp::ClearBit(&currentValue, bit);
                ASSERT_FALSE(BitOp::TestBit(currentValue, bit));

                ASSERT_FALSE(BitOp::TestBit(allZeros, bit));
                BitOp::ClearBit(&allZeros, bit);
                ASSERT_FALSE(BitOp::TestBit(allZeros, bit));
            }
        }
    }
}
