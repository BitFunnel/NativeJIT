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

    // In the context of immediates that are emitted as parts of x64 instructions,
    // there are three categories of types:
    //
    // 1. NonImmediate types cannot be immediates at all (void).
    // 2. RIPRelativeImmediate types must be represented as RIP-relative indirects
    //    because there are no x64 instructions that support them as immediates
    //    (floating point values) or most x64 instructions don't support them as
    //    immediates (quadwords, including function pointers, things that decay
    //    to pointers such as arrays etc).
    // 3. InlineImmediate types can be used directly as immediates in x64 instructions.
    enum class ImmediateCategory { NonImmediate, RIPRelativeImmediate, InlineImmediate };

    // Classifies types that belong to the RIPRelativeImmediate category.
    template <typename T>
    struct MustBeEncodedAsRIPRelative
        : std::integral_constant<bool,
                                 RegisterStorage<T>::c_isFloat
                                 || RegisterStorage<T>::c_size == 8
          >
    {
    };

    template <>
    struct MustBeEncodedAsRIPRelative<void> : std::false_type {};


    // Classifies types that belong to the InlineImmediate category in the list above.
    template <typename T>
    struct CanBeInImmediateStorage
        : std::integral_constant<
            bool,
            std::is_same<typename RegisterStorage<T>::UnderlyingType,
                         typename std::remove_cv<T>::type>::value
                && !MustBeEncodedAsRIPRelative<T>::value
          >
    {
    };

    template <>
    struct CanBeInImmediateStorage<void> : std::false_type {};


    // Classifies types by ImmediateCategory.
    template <typename T>
    struct ImmediateCategoryOf
        : std::integral_constant<ImmediateCategory,
                                 CanBeInImmediateStorage<T>::value
                                    ? ImmediateCategory::InlineImmediate
                                    : (MustBeEncodedAsRIPRelative<T>::value
                                        ? ImmediateCategory::RIPRelativeImmediate
                                        : ImmediateCategory::NonImmediate)
          >
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
