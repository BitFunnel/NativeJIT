#include "stdafx.h"

#ifdef _MSC_VER

#include "UnwindCode_Windows.h"


namespace NativeJIT
{
    namespace Windows
    {
        //*************************************************************************
        //
        // UnwindCode
        //
        //*************************************************************************
        UnwindCode::UnwindCode()
        {
        }


        UnwindCode::UnwindCode(unsigned __int8 offset,
                               unsigned __int8 op,
                               unsigned __int8 info)
            : m_codeOffset(offset),
              m_unwindOp(op),
              m_opInfo(info)
        {
        }


        void UnwindCode::Print(std::ostream& out) const
        {
            static const char* g_registerNames[] =
            {
                "RAX",
                "RCX",
                "RDX",
                "RBX",
                "RSP",
                "RBP",
                "RSI",
                "RDI",
                "R8",
                "R9",
                "R10",
                "R11",
                "R12",
                "R13",
                "R14",
                "R15"
            };

            out << "offset(0x" << std::hex << (unsigned int)m_codeOffset << ") ";
            switch (m_unwindOp)
            {
            case UWOP_PUSH_NONVOL:
                out << "Push NonVolatile: " << g_registerNames[m_opInfo] << "\n";
                break;
            case UWOP_ALLOC_SMALL:
                out << "Alloc Small: " << std::dec << (unsigned int)m_opInfo << " ";
                out << "(0x" << std::hex << ((unsigned int)m_opInfo + 1) * 8 << " bytes)\n";
                break;
            case UWOP_SET_FPREG:
                out << "Set Frame Register\n";
                break;
            default:
                out << "Other: unwindOp(" << (unsigned int)m_unwindOp << ") opInfo(" << (unsigned int)m_opInfo << ")\n";
            }
        }
    }
}

#endif
