#include "stdafx.h"

#include "NativeJIT/CodeGen/Register.h"
#include "UnwindCode.h"


namespace NativeJIT
{
    //*************************************************************************
    //
    // UnwindCode
    //
    //*************************************************************************
    UnwindCode::UnwindCode() 
    {
    }


    UnwindCode::UnwindCode(uint16_t frameOffset) 
        : m_frameOffset(frameOffset)
    {
    }


    UnwindCode::UnwindCode(uint8_t codeOffset,
                           UnwindCodeOp op,
                           uint8_t info) 
    {
        // Note: cannot use initializer list as VC++ doesn't support it (error
        // C2797 "List initialization inside member initializer list or
        // non-static data member initializer is not implemented."
        m_operation.m_codeOffset = codeOffset;
        m_operation.m_unwindOp = static_cast<uint8_t>(op);
        m_operation.m_opInfo = info;
    }

    namespace UnwindUtils
    {
        DWORD64 MakeFunctionTableIdentifier(void* objectAddress)
        {
            // Per MSDN RtlInstallFunctionTableCallback() documentation:
            // "The two low-order bits must be set. For example, BaseAddress|0x3."
            return reinterpret_cast<DWORD64>(objectAddress) | 3;
        }
    }
}
