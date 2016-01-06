#pragma once

#include <cstdint>


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
}
