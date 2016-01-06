#include "stdafx.h"

#include "NativeJIT/CodeGen/ValuePredicates.h"


namespace NativeJIT
{
    unsigned Size(uint8_t value)
    {
        return (value == 0) ? 0 : 1;
    }


    unsigned Size(uint16_t value)
    {
        return (value == 0) ? 0 : ((value <= 0xff) ? 1 : 2);
    }


    unsigned Size(uint32_t value)
    {
        return (value == 0) ? 0 : ((value <= 0xff) ? 1 : ((value <= 0xffff) ? 2 : 4));
    }


    unsigned Size(uint64_t value)
    {
        return (value == 0) ? 0 : ((value <= 0xff)? 1 : ((value <= 0xffff) ? 2 : ((value <= 0xffffffff) ? 4 : 8)));
    }


    unsigned Size(int8_t value)
    {
        return (value == 0) ? 0 : 1;
    }


    unsigned Size(int16_t value)
    {
        return (value == 0) ? 0 : ((value >= -0x80 && value <= 0x7f)? 1 : 2);
    }


    unsigned Size(int32_t value)
    {
        return (value == 0)? 0 : ((value >= -0x80 && value <= 0x7f)? 1 : ((value >= -0x8000 && value <= 0x7fff) ? 2 : 4));
    }


    unsigned Size(int64_t value)
    {
        return (value == 0) ? 0 : ((value >= -0x80LL && value <= 0x7fLL)? 1 : ((value >= -0x8000LL && value <= 0x7fffLL) ? 2 : ((value >= -0x80000000LL && value <= 0x7fffffffLL) ? 4 : 8)));
    }
}
