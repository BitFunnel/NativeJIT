#pragma once

#include <ostream>
#include <type_traits>
#include <Windows.h>

// TODO: What's the underlying reason for this requirement in this file?
#ifndef _AMD64_
#error This code must be compiled for _AMD64_
#endif

namespace NativeJIT
{
    //*************************************************************************
    //
    // Windows unwind structure definitions.
    // From MSDN: http://msdn.microsoft.com/en-us/library/ssa62fwe.aspx
    // Page title is "Unwind Data Definitions in C"
    //
    // Unwind information explanation: http://uninformed.org/index.cgi?v=4&a=1&p=13
    // Stack frame annotation explanation: http://uninformed.org/index.cgi?v=4&a=1&p=17
    //
    //*************************************************************************
    enum class UnwindCodeOp
    {
        UWOP_PUSH_NONVOL = 0, // info == register number
        UWOP_ALLOC_LARGE,     // no info, alloc size in next 2 slots
        UWOP_ALLOC_SMALL,     // info == size of allocation / 8 - 1
        UWOP_SET_FPREG,       // no info, FP = RSP + UNWIND_INFO.FPRegOffset*16
        UWOP_SAVE_NONVOL,     // info == register number, offset in next slot
        UWOP_SAVE_NONVOL_FAR, // info == register number, offset in next 2 slots
        UWOP_SAVE_XMM128,     // info == XMM reg number, offset in next slot
        UWOP_SAVE_XMM128_FAR, // info == XMM reg number, offset in next 2 slots
        UWOP_PUSH_MACHFRAME   // info == 0: no error-code, 1: error-code
    };


    // MSDN struct UNWIND_CODE: https://msdn.microsoft.com/en-us/library/ck9asaa9.aspx
    struct UnwindCode 
    {
        UnwindCode();
        UnwindCode(unsigned __int8 codeOffset, UnwindCodeOp op, unsigned __int8 info);
        UnwindCode(unsigned __int16 frameOffset);

        union
        {
            // Note: C++ doesn't allow nameless structures, so naming this one
            // m_operation.
            struct
            {
                // MSDN: Offset from the beginning of the prolog of the end of
                // the instruction that performs this operation, plus 1 (that is,
                // the offset of the start of the next instruction).
                unsigned __int8 m_codeOffset;

                unsigned __int8 m_unwindOp : 4;
                unsigned __int8 m_opInfo   : 4;
            } m_operation;

            unsigned __int16 m_frameOffset;
        };
    };

    static_assert(std::is_trivially_copyable<UnwindCode>::value, "UnwindCode must be trivially copyable");
    static_assert(sizeof(UnwindCode) == 2, "UnwindCode must be exactly two bytes");


    // MSDN struct UNWIND_INFO: https://msdn.microsoft.com/en-us/library/ddssxxy8.aspx
    // Notable points: "The UNWIND_INFO structure must be DWORD aligned in memory."
    // "For alignment purposes, [unwind codes] array will always have an even number
    // of entries, with the final entry potentially unused (in which case the array
    // will be one longer than indicated by the count of unwind codes field)."
    struct UnwindInfo
    {
        UnwindInfo();

        unsigned __int8 m_version       : 3;
        unsigned __int8 m_flags         : 5;
        unsigned __int8 m_sizeOfProlog;
        unsigned __int8 m_countOfCodes;
        unsigned __int8 m_frameRegister : 4;
        unsigned __int8 m_frameOffset   : 4;

        // Additional codes follow after the end of the structure.
        UnwindCode m_firstUnwindCode;
    };

    static_assert(std::is_trivially_copyable<UnwindInfo>::value,
                  "UnwindInfo must be trivially copyable");
    static_assert(sizeof(UnwindInfo) == 4 + sizeof(UnwindCode),
                  "Invalid UnwindInfo size, unexpected padding or extra members present");

    namespace UnwindUtils
    {
        // Creates a function table identifier for a dynamically generated function.
        // The identifier is used for RtlInstallFunctionTableCallback and
        // RtlDeleteFunctionTable calls. Per their requirements, the two low-order
        // bits of the identifier will be set.
        //
        // The provided address must uniquely identify the buffer containing the
        // function code. Some examples include the address of the buffer itself
        // or the address of a class instance that owns the buffer.
        DWORD64 MakeFunctionTableIdentifier(void* objectAddress);
    }
}
