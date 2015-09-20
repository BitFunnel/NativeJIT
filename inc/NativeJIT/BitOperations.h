#pragma once

#include <intrin.h> // Intrinsic instructions.
#include <type_traits>


namespace NativeJIT
{
    namespace BitOp
    {
        extern const bool c_isPopCntSupported;
        extern char const * const c_bitsSetInByte;

        // Reinterprets the cast from FROM* to TO*, ensuring that FROM and
        // TO are indeed of the same size.
        // This is necessary since bit intrinsics take f. ex. pointer to long
        // and won't accept pointer to __int32 (i.e. int) even though long and
        // int have the same size. The cast doesn't verify that signedness is
        // the same since bit intrinsics work the same way regardless of sign.
        template <typename TO, typename FROM>
        __forceinline
        TO* SameTargetSizeCast(FROM* pointer)
        {
            static_assert(sizeof(FROM) == sizeof(TO),
                          "The pointers must point to types of the same size");

            return reinterpret_cast<TO*>(pointer);
        }


        // Fallback routine for counting the number of 1 bits in a value if
        // the popcnt intrinsic is not supported.
        template <typename T>
        /* __forceinline */
        unsigned __int8 GetNonZeroBitCountFallback(T value)
        {
            static_assert(std::is_integral<T>::value, "The type must be integral");

            unsigned __int8 numBitsSet = 0;
            const auto valueBytes = reinterpret_cast<unsigned __int8 const *>(&value);

            for (unsigned i = 0; i < sizeof(T); ++i)
            {
                numBitsSet += c_bitsSetInByte[valueBytes[i]];
            }

            return numBitsSet;
        }


        // Returns the count of 1 bits in the value.
        // Requires SSE4 support.
        // See https://en.wikipedia.org/wiki/SSE4#POPCNT_and_LZCNT
        // Note that processors from around 2008 and onwards support POPCNT, the
        // fallback method is implemented only for unit test runs in potentially
        // old lab machines/virtual machines.
        __forceinline
        unsigned __int8 GetNonZeroBitCount(unsigned __int32 value)
        {
            return c_isPopCntSupported
                ? static_cast<unsigned __int8>(_mm_popcnt_u32(value))
                : GetNonZeroBitCountFallback(value);
        }


        // Returns the count of 1 bits in the value.
        // Requires SSE4 support.
        // See https://en.wikipedia.org/wiki/SSE4#POPCNT_and_LZCNT
        __forceinline
        unsigned __int8 GetNonZeroBitCount(unsigned __int64 value)
        {
            return c_isPopCntSupported
                ? static_cast<unsigned __int8>(_mm_popcnt_u64(value))
                : GetNonZeroBitCountFallback(value);
        }


        // Stores the index of the lowest 1 bit and returns true. Returns
        // false and leaves lowestBitSetIndex in an underterminate state
        // if the value has no bits set.
        // Uses BSF instruction. See http://felixcloutier.com/x86/BSF.html
        __forceinline
        bool GetLowestBitSet(unsigned __int64 value, unsigned* lowestBitSetIndex)
        {
            return _BitScanForward64(SameTargetSizeCast<unsigned long>(lowestBitSetIndex),
                                     value)
                   ? true
                   : false;
        }

        // Stores the index of the highest 1 bit and returns true. Returns
        // false and leaves highestBitSetIndex in an underterminate state
        // if the value has no bits set.
        // Uses BSR instruction. See http://felixcloutier.com/x86/BSR.html
        __forceinline
        bool GetHighestBitSet(unsigned __int64 value, unsigned* highestBitSetIndex)
        {
            return _BitScanReverse64(SameTargetSizeCast<unsigned long>(highestBitSetIndex),
                                     value)
                   ? true
                   : false;
        }


        // Returns a boolean indicating whether the specified bit is set or not.
        __forceinline
        bool TestBit(unsigned __int64 value, unsigned bitIndex)
        {
            return _bittest64(SameTargetSizeCast<const long long>(&value),
                              bitIndex)
                   ? true
                   : false;
        }


        // Returns a boolean indicating whether the specified bit is set or not.
        template <typename T>
        __forceinline
        bool TestBit(T value, unsigned bitIndex)
        {
            static_assert(std::is_integral<T>::value, "Value must be integral");
            static_assert(sizeof(T) <= sizeof(unsigned __int32), "Value must not be larger than 32 bits");

            const unsigned __int32 value32 = value;
            return _bittest(SameTargetSizeCast<const long>(&value32),
                            bitIndex)
                   ? true
                   : false;
        }


        // Sets the specified bit to 1 and returns whether the bit was previously
        // set or not.
        __forceinline
        bool TestAndSetBit(unsigned __int32* value, unsigned bitIndex)
        {
            return _bittestandset(SameTargetSizeCast<long>(value),
                                  bitIndex)
                   ? true
                   : false;
        }


        // Sets the specified bit to 1 and returns whether the bit was previously
        // set or not.
        __forceinline
        bool TestAndSetBit(unsigned __int64* value, unsigned bitIndex)
        {
            return _bittestandset64(SameTargetSizeCast<long long>(value),
                                    bitIndex)
                   ? true
                   : false;
        }


        // A (naming) convenience wrapper for TestAndSetBit.
        template <typename T>
        __forceinline
        void SetBit(T* value, unsigned bitIndex)
        {
            TestAndSetBit(value, bitIndex);
        }


        // Sets the specified bit to 0 and returns whether the bit was previously
        // set or not.
        __forceinline
        bool TestAndClearBit(unsigned __int32* value, unsigned bitIndex)
        {
            return _bittestandreset(SameTargetSizeCast<long>(value),
                                    bitIndex)
                   ? true
                   : false;
        }


        // Sets the specified bit to 1 and returns whether the bit was previously
        // set or not.
        __forceinline
        bool TestAndClearBit(unsigned __int64* value, unsigned bitIndex)
        {
            return _bittestandreset64(SameTargetSizeCast<long long>(value),
                                      bitIndex)
                   ? true
                   : false;
        }


        // A (naming) convenience wrapper for TestAndClearBit.
        template <typename T>
        __forceinline
        void ClearBit(T* value, unsigned bitIndex)
        {
            TestAndClearBit(value, bitIndex);
        }
    }
}
