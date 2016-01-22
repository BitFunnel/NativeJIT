#pragma once

#include <cstdint>
#include <type_traits>


namespace NativeJIT
{
    typedef uint32_t PackedUnderlyingType;

    //*************************************************************************
    // DESIGN NOTE
    // 
    // The Packed class design makes some accomodations in order to simplify
    // the NativeJIT code. In VS 2012 and VS 2013, the compiler uses a more
    // complex calling convention when invoking a function whose return value
    // is not a POD type as determined by Microsoft's implementation of
    // std::is_pod.
    // 
    // When the return type is not a POD, the first parameter of the function
    // will be a pointer to caller-allocated storage for the return value, and
    // the caller will assume that the function will return this pointer in RAX.
    // 
    // When the return type is POD, its value is returned directly in RAX.
    // 
    // The Microsoft implementation of std::is_pod is more restrictive than
    // the C++11 definition. The Packed class design is impacted in, particular,
    // by Microsoft's restriction on the use of private data members, base
    // classes, and user defined constructors in POD type definitions.
    // 
    // A cleaner design of Packed would make m_field private, and probably
    // move it to a base class common to all of the template specializations.
    // The Create() methods would be replaced by constructors with the same
    // parameter list.
    // 
    // Down the road, we may consider implementing support for non-POD
    // return values in NativeJIT, but even if we do this, we may still want
    // to keep Packed as POD for performance reasons.
    // 
    //*************************************************************************

    // TODO: Use traits to ensure that REST is a Packed type (when not void).
    // TODO: This class and PackedMinMaxNode should probably move from NativeJIT to BitFunnel.Library.

    template <unsigned W = 0, typename REST = void>
    class Packed
    {
    public:
        static const unsigned c_fieldCount = 1 + REST::c_fieldCount;
        static const unsigned c_localBitCount = W;
        static const unsigned c_totalBitCount = c_localBitCount + REST::c_totalBitCount;
        typedef REST Rest;

        static_assert(c_totalBitCount <= sizeof(PackedUnderlyingType) * 8, "Too many bits in the packed.");

        static Packed Create(PackedUnderlyingType value)
        {
            Packed result;
            result.m_fields = value;
            return result;
        }

        // Note: the pushed X-bit value gets placed to the end of m_fields after
        // the existing bits are moved to the left. Thus the final order of
        // bits inside Packed<C, Packed<B, Packed<A>>> is ABC.
        template <unsigned X>
        Packed<X, Packed> Push(PackedUnderlyingType value) const
        {
            PackedUnderlyingType fields = (m_fields << X) | value;
            return Packed<X, Packed>::Create(fields);
        }

        REST Pop() const
        {
            return REST::Create(m_fields >> W);
        }

        PackedUnderlyingType Back() const
        {
            return m_fields & ((1 << W) - 1);
        }

        PackedUnderlyingType GetBits() const
        {
            return m_fields;
        }

        operator PackedUnderlyingType() const
        {
            return m_fields;
        }

        PackedUnderlyingType m_fields;
    };


    template <unsigned W>
    class Packed<W, void>
    {
    public:
        static const unsigned c_fieldCount = 1;
        static const unsigned c_totalBitCount = W;
        static const unsigned c_localBitCount = W;
        typedef void Rest;

        static_assert(c_totalBitCount <= sizeof(PackedUnderlyingType) * 8, "Too many bits in the packed.");

        static Packed Create(PackedUnderlyingType value)
        {
            Packed result;
            result.m_fields = value;
            return result;
        }

        template <unsigned X>
        Packed<X, Packed> Push(PackedUnderlyingType value)
        {
            PackedUnderlyingType fields = (m_fields << X) | value;
            return Packed<X, Packed>::Create(fields);
        }

        PackedUnderlyingType Back() const
        {
            return m_fields & ((1 << W) - 1);
        }

        PackedUnderlyingType GetBits() const
        {
            return m_fields;
        }

        operator PackedUnderlyingType() const
        {
            return m_fields;
        }

        PackedUnderlyingType m_fields;
    };


    template <>
    class Packed<0, void>
    {
    public:
        static const unsigned c_fieldCount = 0;
        static const unsigned c_totalBitCount = 0;
        static const unsigned c_localBitCount = 0;
        typedef void Rest;

        template <unsigned X>
        static Packed<X, void> Push(PackedUnderlyingType value)
        {
            return Packed<X>::Create(value);
        }
    };
}
