#include "stdafx.h"

#include <iostream>         // TODO: Remove - temporary for debugging.

#include <stdexcept>
#include <Windows.h>

#include "NativeJIT/FunctionBuffer.h"
#include "Temporary/IAllocator.h"
#include "UnwindCode.h"


namespace NativeJIT
{
    //*************************************************************************
    //
    // FunctionBuffer
    //
    //*************************************************************************
    FunctionBuffer::FunctionBuffer(Allocators::IAllocator& allocator,
                                           unsigned capacity,
                                           unsigned maxLabels,
                                           unsigned maxCallSites,
                                           unsigned slotCount,
                                           unsigned registerSaveMask,
                                           bool isLeaf)
        : X64CodeGenerator(allocator, capacity, maxLabels, maxCallSites)
    {
        // TODO: This cannot be known in advance, only after the function is
        // compiled. For now, save all nonvolatile RXX registers to have the
        // retail unit tests pass. As part of TFS 17238, this will be addressed
        // properly, along with XMM registers. A unit test which puts garbage
        // into all nonvolatile registers should be added as well.
        registerSaveMask = 0xF0E8; // All RXX volatiles.

#ifdef _DEBUG
        FillWithBreakCode(0, BufferSize());
#endif
//        m_isLeaf = isLeaf;

        //
        // Newer initialization sequence.
        //
        EmitUnwindInfo(static_cast<unsigned char>(slotCount), registerSaveMask, isLeaf);
        CreatePrologue();
        RegisterUnwindInfo();
        CreateEpilogue();
        Reset();
    }


    unsigned char const * FunctionBuffer::GetEntryPoint() const
    {
        return m_entryPoint;
    }


    void FunctionBuffer::Reset()
    {
        CodeBuffer::Reset(static_cast<unsigned>(m_codeGenStart));
    }


    void FunctionBuffer::EmitPrologue()
    {
        m_entryPoint = BufferStart() + CurrentPosition();
        for (unsigned i = 0 ; i < m_prologueCode.size(); ++i)
        {
            Emit8(m_prologueCode[i]);
        }
    }


    void FunctionBuffer::EmitEpilogue()
    {
        for (unsigned i = 0 ; i < m_epilogueCode.size(); ++i)
        {
            Emit8(m_epilogueCode[i]);
        }
    }


    void FunctionBuffer::EmitUnwindInfo(unsigned char slotCount,
                                            unsigned registerSaveMask,
                                            bool isLeaf)
    {
        // TODO: Is the leaf functionality worthwhile or would it be better to require the user
        // to allocate the space before calling out? The current functionality isn't useful for
        // X64CompileEngine because its recursion pushes temporary values on the stack, forcing
        // another 4 slots to be allocated before any call out to C or C++.
        if (!isLeaf)
        {
            // The X64 calling convention requires that non-leaf functions (those that call other functions)
            // reserve 4 stack slots for the callee to use as home locations for paramters passed by register.
            slotCount += 4;
        }

        m_slotsAllocated = 1;               // First slot is return address pushed by caller.

        // Ensure that the frame register is saved.
        registerSaveMask |= rbp.GetMask();

        //
        // Compute number of unwind codes needed.
        //

        // First count the number of register saves.
        unsigned registerCount = 0;
        unsigned r = registerSaveMask;
        while (r != 0)
        {
            if (r & 1)
            {
                ++registerCount;
            }
            r = r >> 1;
        }

        // One UWOP_PUSH_NONVOL per register pushed.
        unsigned unwindCodeCount = registerCount;

        if (slotCount > 0)
        {
            // One UWOP_ALLOC_SMALL
            ++unwindCodeCount;
        }
        else if ((registerCount & 1) == 0)
        {
            // Even number of registers plus one slot for return address => dummy push to align RSP to 16-byte boundary.
            // One more UWOP_PUSH_NONVOL
            ++unwindCodeCount;
        }

        // One UWOP_SET_FPREG
        ++unwindCodeCount;

        //
        // Initialize member variables used in prologue generation.
        //
        m_runtimeFunction.UnwindData = CurrentPosition();

        AllocateUnwindInfo(unwindCodeCount);
        m_unwindInfo->m_countOfCodes = static_cast<unsigned char>(unwindCodeCount);
        m_unwindInfo->m_version = 1;
        m_unwindInfo->m_flags = 0;

        m_runtimeFunction.BeginAddress = CurrentPosition();
        m_runtimeFunction.EndAddress = BufferSize();            // TODO: Is this correct or do we want BufferSize() + start?

        m_prologueStart = CurrentPosition();

        // Push non-volatile registers
        // TODO: investigate whether pushing volatiles causes problems for stack unwinding.
        r = registerSaveMask;
        for (unsigned char i = 0 ; i < 16; ++i)
        {
            if (r & 1)
            {
                ProloguePushNonVolatile(Register<8, false>(i));
            }
            r = r >> 1;
        }

        if (slotCount > 0)
        {
            if (((m_slotsAllocated + slotCount) & 1) != 0)
            {
                // Allocate one extra slot to ensure that RSP is aligned to a 16 byte boundary.
                slotCount++;
            }
            PrologueStackAllocate(slotCount);
        }
        else if ((m_slotsAllocated & 1) != 0)
        {
            // Do a dummy push to ensure that RSP is aligned to 16-byte boundary.
            ProloguePushNonVolatile(rbp);
        }

        PrologueSetFrameRegister(slotCount);

        // Set up m_stackLocalsBase which is the offset of the first local relative to frame
        // register. 
        m_stackLocalsBase = -m_unwindInfo->m_frameOffset * 16;
        if (!isLeaf)
        {
            // If this function is not a leaf (meaning it calls other functions), the stack
            // local variables start after 4 slots required for home locations for register parameters.
            m_stackLocalsBase += 4 * sizeof(__int64);
        }
    }


    void FunctionBuffer::CreatePrologue()
    {
        unsigned start = CurrentPosition();

        // Generate prologue code from unwind information.
        for (int i = m_unwindInfo->m_countOfCodes - 1; i >= 0 ; --i)
        {
            UnwindCode code = m_unwindInfo->m_unwindCodes[i];
            switch (code.m_unwindOp)
            {
            case UWOP_PUSH_NONVOL:
                Emit<OpCode::Push>(Register<8, false>(code.m_opInfo));
                break;
            case UWOP_ALLOC_SMALL:
                EmitImmediate<OpCode::Sub>(rsp, (code.m_opInfo + 1) * 8);
                break;
            case UWOP_SET_FPREG:
                Emit<OpCode::Lea>(rbp, rsp, m_unwindInfo->m_frameOffset * 16);
                break;
            default:
                throw std::runtime_error("Invalid unwind data.");
            }
        }

        unsigned end = CurrentPosition();
        unsigned size = end - start;

        Assert(size < 256, "Prologue cannot exceed 255 bytes.");
        m_unwindInfo->m_sizeOfProlog = static_cast<unsigned __int8>(size);

        // Save a copy of the prologue.
        m_prologueCode.resize(size);
        memcpy(&m_prologueCode[0], BufferStart() + start, size);
    }


    void FunctionBuffer::CreateEpilogue()
    {
        unsigned start = CurrentPosition();

        // Generate epilogue code from unwind information.
        for (int i = 0; i < m_unwindInfo->m_countOfCodes; ++i)
        {
            UnwindCode code = m_unwindInfo->m_unwindCodes[i];
            switch (code.m_unwindOp)
            {
            case UWOP_PUSH_NONVOL:
                Emit<OpCode::Pop>(Register<8, false>(code.m_opInfo));
                break;
            case UWOP_ALLOC_SMALL:
                EmitImmediate<OpCode::Add>(rsp, (code.m_opInfo + 1) * 8);
                break;
            case UWOP_SET_FPREG:
                Emit<OpCode::Lea>(rsp, rbp, -m_unwindInfo->m_frameOffset * 16);
                break;
            default:
                throw std::runtime_error("Invalid unwind data.");
            }
        }

        Emit<OpCode::Ret>();

        // Save a copy of the epilogue.
        unsigned size = CurrentPosition() - start;
        m_epilogueCode.resize(size);
        memcpy(&m_epilogueCode[0], BufferStart() + start, size);

        // Now that epilogue has been saved, reset buffer to end of prologue.
        Reset();
    }


    void FunctionBuffer::RegisterUnwindInfo()
    {
        if (!RtlAddFunctionTable(&m_runtimeFunction, 1, (DWORD64)BufferStart()))
        {
            throw std::runtime_error("RtlAddFunctionTable failed.");
        }
    }


    // TODO: Allocate on heap. Is this right? 
    //         Perhaps we should send the UnwindInfo and entry point over the wire.
    //         Unwind info would be compared with receiver unwind info.
    // TODO: Document reverse order fillings.
    // Creates an UnwindInfo structure in the code buffer.
    void FunctionBuffer::AllocateUnwindInfo(unsigned unwindCodeCount)
    {
        if (unwindCodeCount > UnwindInfo::c_maxUnwindCodes)
        {
            throw std::runtime_error("Too many unwind codes.");
        }
        m_unwindInfo = (UnwindInfo*)Advance(sizeof(UnwindInfo));
        m_unwindCodePtr = m_unwindInfo->m_unwindCodes + unwindCodeCount;
        m_codeGenStart = CurrentPosition();
    }


    void FunctionBuffer::ProloguePushNonVolatile(Register<8, false> r)
    {
        // TODO: Review this static_cast. Is there a better way to do this?
        EmitUnwindCode(UnwindCode(static_cast<unsigned char>(CurrentPosition() - m_prologueStart),
                                  UWOP_PUSH_NONVOL,
                                  static_cast<unsigned char>(r.GetId())));
        ++m_slotsAllocated;
    }


    void FunctionBuffer::PrologueStackAllocate(unsigned __int8 slots)
    {
        // Compare with 16 (vs 15) is correct since UWOP_ALLOC_SMALL encodes 1..16 as 0..15.
        if (slots > 16)
        {
            throw std::runtime_error("PrologStackAllocate: slot overflow.");
        }
        else if (slots == 0)
        {
            throw std::runtime_error("PrologStackAllocate: slots cannot be 0.");
        }

        // TODO: Review this static_cast. Is there a bette way to do this?
        EmitUnwindCode(UnwindCode(static_cast<unsigned char>(CurrentPosition() - m_prologueStart),
                                  UWOP_ALLOC_SMALL,
                                  slots - 1));
        m_slotsAllocated += slots;
    }


    void FunctionBuffer::PrologueSetFrameRegister(unsigned __int8 slots)
    {
        unsigned desiredOffset = slots / 2;     // Want offset to put frame pointer in the middle of the slots.
        if (desiredOffset %2 == 1)              // Need an offset that is divisible by 16.
        {
            --desiredOffset;
        }

        EmitUnwindCode(UnwindCode(static_cast<unsigned char>(CurrentPosition() - m_prologueStart), UWOP_SET_FPREG, 0));

        m_unwindInfo->m_frameRegister = rbp.GetId();
        m_unwindInfo->m_frameOffset = desiredOffset / 2;
    }


    void FunctionBuffer::EmitUnwindCode(const UnwindCode& code)
    {
        // TODO: Buffer overflow handling. Perhaps this should use Emit8 instead of its own m_unwindCodePtr.
        *--m_unwindCodePtr = code;
    }


    bool FunctionBuffer::UnwindInfoIsValid(std::ostream& out,
                                           RUNTIME_FUNCTION& runtimeFunction)
    {
        bool success = true;

        if ((size_t)(&runtimeFunction) % 4 != 0)
        {
            out << "Error: runtimeFunction must be DWORD aligned.\n";
            success = false;
        }

        if (runtimeFunction.BeginAddress > runtimeFunction.EndAddress)
        {
            out << "Error: runtimeFunction begin address after end address.";
        }

        if (runtimeFunction.UnwindData + BufferStart() != (unsigned char*)m_unwindInfo)
        {
            out << "Error: runtimeFunction does not reference unwind data.\n";
            success = false;
        }

        if ((size_t)(m_unwindInfo) % 4 != 0)
        {
            out << "Error: m_unwindInfo must be DWORD aligned.\n";
            success = false;
        }

        if (m_unwindInfo->m_version != 1)
        {
            out << "Error: m_unwindInfo has bad version number.\n";
            success = false;
        }

        if (m_unwindInfo->m_flags != 0)
        {
            out << "Error: m_unwindInfo flags are not 0.\n";
            success = false;
        }

        if (m_unwindInfo->m_countOfCodes > 1)
        {
            for (int i = 1 ; i < m_unwindInfo->m_countOfCodes; ++i)
            {
                if (m_unwindInfo->m_unwindCodes[i].m_codeOffset >= m_unwindInfo->m_unwindCodes[i-1].m_codeOffset)
                {
                    out << "Error: unwind code offsets must be in decreasing order.\n";
                    success = false;
                }
            }
        }

        for (int i = 0; i < m_unwindInfo->m_countOfCodes; ++i)
        {
            int code = m_unwindInfo->m_unwindCodes[i].m_unwindOp;
            if (code != UWOP_PUSH_NONVOL && code != UWOP_ALLOC_SMALL && code != UWOP_SET_FPREG)
            {
                out << "Error: unsupported unwind code (" << code << ").\n";
                success = false;
            }
        }

        // TODO: Ensure that UWOP_SET_FPREG is coded to RBP.

        return success;
    }


    void FunctionBuffer::FillWithBreakCode(unsigned start, unsigned length)
    {
        Fill(start, length, 0xcc);
    }
}
