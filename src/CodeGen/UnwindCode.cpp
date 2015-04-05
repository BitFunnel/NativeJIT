#include "stdafx.h"

#include "NativeJIT/Register.h"
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


    UnwindCode::UnwindCode(unsigned __int16 frameOffset) 
        : m_frameOffset(frameOffset)
    {
    }


    UnwindCode::UnwindCode(unsigned __int8 codeOffset,
                           UnwindCodeOp op,
                           unsigned __int8 info) 
    {
        // Note: cannot use initializer list as VC++ doesn't support it (error
        // C2797 "List initialization inside member initializer list or
        // non-static data member initializer is not implemented."
        m_operation.m_codeOffset = codeOffset;
        m_operation.m_unwindOp = static_cast<unsigned __int8>(op);
        m_operation.m_opInfo = info;
    }

    namespace UnwindUtils
    {
        DWORD64 MakeFunctionTableIdentifier(void* objectAddress)
        {
            // The lowest 3 bits must be set.
            return reinterpret_cast<DWORD64>(objectAddress) | 3;
        }
    }
}
