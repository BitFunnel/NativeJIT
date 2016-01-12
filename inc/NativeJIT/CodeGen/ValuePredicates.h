#pragma once

#include <cstdint>
#include <type_traits>


namespace NativeJIT
{
    // Function overloads returns the byte sizes of various types.
    // Used by X64CodeGenerator::Group1() template definition.
    unsigned Size(uint8_t);
    unsigned Size(uint16_t value);
    unsigned Size(uint32_t value);
    unsigned Size(uint64_t value);
    unsigned Size(int8_t);
    unsigned Size(int16_t value);
    unsigned Size(int32_t value);
    unsigned Size(int64_t value);


    // Cast using a static_cast for convertible immediates.
    template <typename TO, typename FROM>
    TO ForcedCast(FROM from,
                  typename std::enable_if<std::is_convertible<FROM, TO>::value>::type* = nullptr)
    {
        return static_cast<TO>(from);
    }


    // Cast using a reinterpret_cast for non-convertible immediates of the
    // same size.
    template <typename TO, typename FROM>
    TO ForcedCast(FROM from,
                  typename std::enable_if<!std::is_convertible<FROM, TO>::value>::type* = nullptr)
    {
        static_assert(sizeof(FROM) == sizeof(TO),
                      "Cannot do a forced cast between incompatible types of different sizes");

        return *reinterpret_cast<TO*>(&from);
    }
}
