#pragma once


namespace NativeJIT
{
    // Function overloads returns the byte sizes of various types.
    // Used by X64CodeGenerator::Group1() template definition.
    unsigned Size(unsigned __int8);
    unsigned Size(unsigned __int16 value);
    unsigned Size(unsigned __int32 value);
    unsigned Size(unsigned __int64 value);
    unsigned Size(__int8);
    unsigned Size(__int16 value);
    unsigned Size(__int32 value);
    unsigned Size(__int64 value);
}
