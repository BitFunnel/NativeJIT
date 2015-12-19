#pragma once

// TODO: what if some other file in the library includes <Windows.h>, but doesn't
// define _AMD64_? Should probably define _AMD64_ in the .props file for the project.
#ifndef _AMD64_
#define _AMD64_
#endif

#include <ostream>
#include <Windows.h>

namespace NativeJIT
{
    namespace Windows
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
        // TODO: Convert to enum class.
        typedef enum {
            UWOP_PUSH_NONVOL = 0, // info == register number
            UWOP_ALLOC_LARGE,     // no info, alloc size in next 2 slots
            UWOP_ALLOC_SMALL,     // info == size of allocation / 8 - 1
            UWOP_SET_FPREG,       // no info, FP = RSP + UNWIND_INFO.FPRegOffset*16
            UWOP_SAVE_NONVOL,     // info == register number, offset in next slot
            UWOP_SAVE_NONVOL_FAR, // info == register number, offset in next 2 slots
            UWOP_SAVE_XMM128,     // info == XMM reg number, offset in next slot
            UWOP_SAVE_XMM128_FAR, // info == XMM reg number, offset in next 2 slots
            UWOP_PUSH_MACHFRAME   // info == 0: no error-code, 1: error-code
        } UnwindCodeOps;


        struct UnwindCode
        {
            UnwindCode();
            UnwindCode(unsigned __int8 offset, unsigned __int8 op, unsigned __int8 info);
            void Print(std::ostream& out) const;

            unsigned __int8 m_codeOffset;
            unsigned __int8 m_unwindOp : 4;
            unsigned __int8 m_opInfo : 4;
        };


        struct UnwindInfo
        {
            UnwindInfo();
            void Print(std::ostream& out) const;

            unsigned __int8 m_version : 3;
            unsigned __int8 m_flags : 5;
            unsigned __int8 m_sizeOfProlog;
            unsigned __int8 m_countOfCodes;
            unsigned __int8 m_frameRegister : 4;
            unsigned __int8 m_frameOffset : 4;

            // TODO: Consider making this array have one element and allow codes to run past end of array.
            static const int c_maxUnwindCodes = 20;
            UnwindCode m_unwindCodes[c_maxUnwindCodes];
        };
    }
}
