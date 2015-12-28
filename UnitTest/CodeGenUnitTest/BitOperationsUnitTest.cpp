#include "stdafx.h"

#include "NativeJIT/BitOperations.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"


namespace NativeJIT
{
    namespace BitOperationsUnitTest
    {
        TestCase(NonZeroBitCount)
        {
            TestEqual(0, BitOp::GetNonZeroBitCount(0u));
            TestEqual(0, BitOp::GetNonZeroBitCountFallback(0u));

            TestEqual(0, BitOp::GetNonZeroBitCount(0ull));
            TestEqual(0, BitOp::GetNonZeroBitCountFallback(0ull));

            TestEqual(1, BitOp::GetNonZeroBitCount(1u));
            TestEqual(1, BitOp::GetNonZeroBitCountFallback(1u));

            TestEqual(1, BitOp::GetNonZeroBitCount(0x80000000u));
            TestEqual(1, BitOp::GetNonZeroBitCountFallback(0x80000000u));

            TestEqual(1, BitOp::GetNonZeroBitCount(0x8000000000000000));
            TestEqual(1, BitOp::GetNonZeroBitCountFallback(0x8000000000000000));

            TestEqual(32, BitOp::GetNonZeroBitCount(0xFFFFFFFFu));
            TestEqual(32, BitOp::GetNonZeroBitCountFallback(0xFFFFFFFFu));

            TestEqual(64, BitOp::GetNonZeroBitCount(0xFFFFFFFFFFFFFFFF));
            TestEqual(64, BitOp::GetNonZeroBitCountFallback(0xFFFFFFFFFFFFFFFF));
        }


        void VerifyLowestBitSet(uint64_t testValue, unsigned expected)
        {
            TestNotEqual(0, testValue);

            unsigned actual;
            const bool foundBit = BitOp::GetLowestBitSet(testValue, &actual);

            TestEqual(true, foundBit, "Expected to find a bit in %I64u", testValue);
            TestEqual(expected, actual, "Mismatched expected and actual bit for %I64u", testValue);
        }


        TestCase(LowestBitSet)
        {
            unsigned unused;
            TestEqual(false, BitOp::GetLowestBitSet(0, &unused));

            VerifyLowestBitSet(1, 0);
            VerifyLowestBitSet(0xFFFFFFFFFFFFFFFF, 0);
            VerifyLowestBitSet(0x8000000000000000, 63);
        }


        void VerifyHighestBitSet(uint64_t testValue, unsigned expected)
        {
            TestNotEqual(0, testValue);

            unsigned actual;
            const bool foundBit = BitOp::GetHighestBitSet(testValue, &actual);

            TestEqual(true, foundBit, "Expected to find a bit in %I64u", testValue);
            TestEqual(expected, actual, "Mismatched expected and actual bit for %I64u", testValue);
        }


        TestCase(HighestBitSet)
        {
            unsigned unused;
            TestEqual(false, BitOp::GetHighestBitSet(0, &unused));

            VerifyHighestBitSet(1, 0);
            VerifyHighestBitSet(0x8000000000000000, 63);
            VerifyHighestBitSet(0xFFFFFFFFFFFFFFFF, 63);
        }


        TestCase(TestBit)
        {
            const uint64_t bits62and63 = 0xC000000000000000;

            TestEqual(false, BitOp::TestBit(bits62and63, 0));
            TestEqual(false, BitOp::TestBit(bits62and63, 1));
            TestEqual(true, BitOp::TestBit(bits62and63, 62));
            TestEqual(true, BitOp::TestBit(bits62and63, 63));

            const uint64_t allZeros = 0;
            const uint64_t allOnes = 0xFFFFFFFFFFFFFFFF;

            for (unsigned int bit = 0; bit < 64; ++bit)
            {
                TestEqual(false, BitOp::TestBit(allZeros, bit));
                TestEqual(true, BitOp::TestBit(allOnes, bit));
            }
        }


        TestCase(TestAndSetBit)
        {
            uint64_t currentValue = 0;
            uint64_t allOnes = 0xFFFFFFFFFFFFFFFF;

            for (unsigned int bit = 0; bit < 64; ++bit)
            {
                TestEqual(false, BitOp::TestAndSetBit(&currentValue, bit));
                TestEqual(true, BitOp::TestBit(currentValue, bit));

                TestEqual(true, BitOp::TestAndSetBit(&allOnes, bit));
                TestEqual(true, BitOp::TestBit(allOnes, bit));
            }
        }


        // SetBit intentionally not tested since it's just TestAndSetBit with
        // return value ignored.


        TestCase(TestAndClearBit)
        {
            uint64_t currentValue = 0xFFFFFFFFFFFFFFFF;
            uint64_t allZeros = 0;

            for (unsigned int bit = 0; bit < 64; ++bit)
            {
                TestEqual(true, BitOp::TestAndClearBit(&currentValue, bit));
                TestEqual(false, BitOp::TestBit(currentValue, bit));

                TestEqual(false, BitOp::TestAndClearBit(&allZeros, bit));
                TestEqual(false, BitOp::TestBit(allZeros, bit));
            }
        }
    }
}
