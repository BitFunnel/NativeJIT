#pragma once

#include <type_traits>

#include "NativeJIT/Register.h"

namespace NativeJIT
{
    // A trait used to determine whether a type has any decay other than
    // const/volatile.
    template <typename T>
    struct HasMeaningfulDecay
        : std::integral_constant<
            bool,
            !std::is_same<typename std::remove_cv<T>::type,
                          typename std::decay<T>::type>
                        ::value>
    {
    };


    // Declares the properties of the type used for register storage.
    template <typename T>
    struct RegisterStorage
    {
        // The underlying type stored inside the register when passing the
        // storage as an argument to a function. F. ex. references and large
        // structures are passed as pointers. Any const/volatile qualifiers are
        // removed.
        // WARNING: Representation of a reference is implementation-specific.
        // However, (almost?) all compilers implement it as a pointer.
        typedef typename std::conditional<
            std::is_reference<T>::value
                || (sizeof(T) > RegisterBase::c_maxSize),
            typename std::add_pointer<typename std::remove_cv<T>::type>::type,
            typename std::conditional<
                HasMeaningfulDecay<T>::value,
                typename std::decay<T>::type,
                typename std::remove_cv<T>::type
            >::type
        >::type UnderlyingType;

        static const unsigned c_size = sizeof(UnderlyingType);
        static const bool c_isFloat = std::is_floating_point<UnderlyingType>::value;

        typedef Register<c_size, c_isFloat> RegisterType;
    };


    // Determines whether a type is valid as an immediate from Storage's perspective.
    // Invalid types include void, float and anything that decays its type
    // (references, structures larger than the full register, arrays, etc).
    template <typename T>
    struct IsValidImmediate
        : std::integral_constant<
            bool,
            !std::is_void<T>::value
                && !std::is_floating_point<T>::value
                && std::is_same<typename RegisterStorage<T>::UnderlyingType,
                                typename std::remove_cv<T>::type>::value>
    {
    };


    // Determines the canonical C++ type that maps to a register type.
    template <typename T>
    struct CanonicalRegisterType;

    template <> struct CanonicalRegisterType<Register<1, false>> { typedef unsigned __int8 Type; };
    template <> struct CanonicalRegisterType<Register<2, false>> { typedef unsigned __int16 Type; };
    template <> struct CanonicalRegisterType<Register<4, false>> { typedef unsigned __int32 Type; };
    template <> struct CanonicalRegisterType<Register<8, false>> { typedef unsigned __int64 Type; };

    template <> struct CanonicalRegisterType<Register<4, true>> { typedef float Type; };
    template <> struct CanonicalRegisterType<Register<8, true>> { typedef double Type; };


    // Determines the canonical C++ type that maps to a register storage needed
    // for a type. F. ex. void*, a function pointer etc. map to unsigned __int64.
    template <typename T>
    struct CanonicalRegisterStorageType
    {
        typedef typename CanonicalRegisterType<
            typename RegisterStorage<T>::RegisterType
        >::Type Type;
    };
}
