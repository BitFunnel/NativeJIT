#pragma once

#include <cstdint>
#include <type_traits>

#ifdef NATIVEJIT_PLATFORM_WINDOWS
#include <intrin.h>
#else
#include <nmmintrin.h>
#include <smmintrin.h>
#include <string.h>         // For ffsll()
//#include <x86intrin.h> // Intrinsic instructions.
#endif

// http://stackoverflow.com/questions/2039861/how-to-get-gcc-to-generate-bts-instruction-for-x86-64-from-standard-c
// https://developer.apple.com/library/ios/documentation/System/Conceptual/ManPages_iPhoneOS/man3/atomic.3.html
// https://chessprogramming.wikispaces.com/x86-64

namespace NativeJIT
{
    #ifdef NATIVEJIT_PLATFORM_WINDOWS
      #ifndef __CYGWIN__
        typedef long BitTestType32;
        typedef long long BitTestType64;
        typedef unsigned long BitScanType32;
      #else
        typedef int BitTestType32;
        typedef long long BitTestType64;
        typedef unsigned int BitScanType32;
      #endif
      static_assert(sizeof(BitTestType32) == 4, "Invalid BitTestType32 size");
      static_assert(sizeof(BitTestType64) == 8, "Invalid BitTestType64 size");
      static_assert(sizeof(BitScanType32) == 4, "Invalid BitTestType size");
    #else
    //#include <x86intrin.h> // Intrinsic instructions.
    #endif


    // WARNING: the methods in this file are thin wrappers around compiler
    // intrinsic methods. As such, they are meant to have no or minimal added
    // overhead which is why only compile-time checks are done, but no runtime
    // checks for valid input range or similar are performed.
    namespace BitOp
    {
        extern const bool c_isPopCntSupported;
        extern char const * const c_bitsSetInByte;

        // Reinterprets the cast from FROM* to TO*, ensuring that FROM and
        // TO are indeed of the same size.
        // This is necessary since bit intrinsics take e.g. pointer to long
        // and won't accept pointer to int32_t (i.e. int) even though long and
        // int have the same size. The cast doesn't verify that signedness is
        // the same since bit intrinsics work the same way regardless of sign.
        template <typename TO, typename FROM>
        inline
        TO* SameTargetSizeCast(FROM* pointer)
        {
            static_assert(sizeof(FROM) == sizeof(TO),
                          "The pointers must point to types of the same size");

            return reinterpret_cast<TO*>(pointer);
        }


        // Fallback routine for counting the number of 1 bits in a value if
        // the popcnt intrinsic is not supported.
        template <typename T>
        inline
        uint8_t GetNonZeroBitCountFallback(T value)
        {
            static_assert(std::is_integral<T>::value, "The type must be integral");

            uint8_t numBitsSet = 0;
            auto const valueBytes = reinterpret_cast<uint8_t const *>(&value);

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
        inline
        uint8_t GetNonZeroBitCount(uint32_t value)
        {
            return c_isPopCntSupported
                ? static_cast<uint8_t>(_mm_popcnt_u32(value))
                : GetNonZeroBitCountFallback(value);
        }


        // Returns the count of 1 bits in the value.
        // Requires SSE4 support.
        // See https://en.wikipedia.org/wiki/SSE4#POPCNT_and_LZCNT
        inline
        uint8_t GetNonZeroBitCount(uint64_t value)
        {
            return c_isPopCntSupported
                ? static_cast<uint8_t>(_mm_popcnt_u64(value))
                : GetNonZeroBitCountFallback(value);
        }


        // Stores the index of the lowest 1 bit and returns true. Returns
        // false and leaves lowestBitSetIndex in an underterminate state
        // if the value has no bits set.
        // Uses BSF instruction. See http://felixcloutier.com/x86/BSF.html
        inline
        bool GetLowestBitSet(uint64_t value, unsigned* lowestBitSetIndex)
        {
#ifdef NATIVEJIT_PLATFORM_WINDOWS
            return _BitScanForward64(SameTargetSizeCast<BitScanType32>(lowestBitSetIndex),
                                     value)
                   ? true
                   : false;
#else
            *lowestBitSetIndex = ffsll(value) - 1;
            return *lowestBitSetIndex != -1;
#endif
        }

        // Stores the index of the highest 1 bit and returns true. Returns
        // false and leaves highestBitSetIndex in an underterminate state
        // if the value has no bits set.
        // Uses BSR instruction. See http://felixcloutier.com/x86/BSR.html
        inline
        bool GetHighestBitSet(uint64_t value, unsigned* highestBitSetIndex)
        {
#ifdef NATIVEJIT_PLATFORM_WINDOWS
	    // Windows provides _BitScanReverse64()
            return _BitScanReverse64(SameTargetSizeCast<BitScanType32>(highestBitSetIndex),
                                     value)
                   ? true
                   : false;
#elif (APPLE)
	    // OS X provides flsll().
	    *highestBitSetIndex = flsll(value) - 1;
            return *highestBitSetIndex != -1;
#else
	    // Linux + gcc provide __builtin_clzl().
	    if (value == 0)
	    {
	      // Special case for value == 0 since __builtin_clzl(0) is undefined.
              return false;
	    }
            *highestBitSetIndex = 63 - __builtin_clzl(value);
            return *highestBitSetIndex != -1;
#endif
        }


        // Returns a boolean indicating whether the specified bit is set or not.
        // WARNING: Does not verify that bitIndex is in valid range.
        inline
        bool TestBit(uint64_t value, unsigned bitIndex)
        {
#ifdef NATIVEJIT_PLATFORM_WINDOWS
            return _bittest64(SameTargetSizeCast<const BitTestType64>(&value),
                              bitIndex)
                   ? true
                   : false;
#else
            return (value & (static_cast<uint64_t>(1) << bitIndex)) ? true : false;
#endif
        }


        // Returns a boolean indicating whether the specified bit is set or not.
        // WARNING: Does not verify that bitIndex is in valid range.
        template <typename T>
        inline
        bool TestBit(T value, unsigned bitIndex)
        {
            static_assert(std::is_integral<T>::value, "Value must be integral");

#ifdef NATIVEJIT_PLATFORM_WINDOWS
            static_assert(sizeof(T) <= sizeof(BitTestType32), "Value must not be larger than 32 bits");
            const uint32_t value32 = value;

            return _bittest(SameTargetSizeCast<const BitTestType32>(&value32),
                            bitIndex)
                   ? true
                   : false;
#else
            return (value & (static_cast<T>(1) << bitIndex)) ? true : false;
#endif
        }


        // Sets a specific bit in the target value.
        // WARNING: Does not verify that bitIndex is in valid range.
        template <typename T>
        inline
        void SetBit(T* value, unsigned bitIndex)
        {
            *value |= static_cast<T>(1) << bitIndex;
        }


        // Clears a bit in the target value.
        // WARNING: Does not verify that bitIndex is in valid range.
        template <typename T>
        inline
        void ClearBit(T* value, unsigned bitIndex)
        {
            *value &= ~(static_cast<T>(1) << bitIndex);
        }
    }
}
