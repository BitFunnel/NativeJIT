#include "stdafx.h"

#include <iostream>         // TODO: Remove - temporary for debugging.

#include <stdexcept>
#include <Windows.h>

#include "FunctionBuffer.h"
#include "UnwindCode.h"


namespace NativeJIT
{
    const unsigned RBP = 4;

    void TODO()
    {
        throw 0;
    }


    //*************************************************************************
    //
    // FunctionBufferBase
    //
    //*************************************************************************
    FunctionBufferBase::FunctionBufferBase(Allocators::IAllocator& allocator,
                                           unsigned capacity,
                                           unsigned slotCount,
                                           unsigned registerSaveMask,
                                           bool isLeaf)
        : m_allocator(allocator),
          X64CodeGenerator(std::cout, static_cast<unsigned __int8*>(allocator.Allocate(capacity)), capacity, 0, 0)
    {
        // TODO: Need to deallocate buffer passed to X64CodeGenerator constructor.

#ifdef _DEBUG
        FillWithBreakCode(0, BufferSize());
#endif
//        m_isLeaf = isLeaf;

        EmitPrologue(static_cast<unsigned char>(slotCount), registerSaveMask, isLeaf);
        m_bodyStart = CurrentPosition();

        // TODO: Consider moving this into EmitPrologue?
        if (!RtlAddFunctionTable(&m_runtimeFunction, 1, (DWORD64)BufferStart()))
        {
            throw std::runtime_error("RtlAddFunctionTable failed.");
        }
    }


    FunctionBufferBase::~FunctionBufferBase()
    {
        // TODO: Need to deallocate buffer passed to m_code constructor.
    }


    CodeBuffer& FunctionBufferBase::GetCodeBuffer()
    {
        return *this;
    }


    void FunctionBufferBase::EmitPrologue(unsigned char slotCount,
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
        m_entryPoint = AllocateLabel();

        // Ensure that the frame register is saved.
        registerSaveMask |= RBP;

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

        // TODO: Move this functionality to an unwind init method?
        //
        // Initialize member variables used in prologue generation.
        //
        m_runtimeFunction.UnwindData = CurrentPosition();

        AllocateUnwindInfo(unwindCodeCount);
        m_unwindInfo->m_countOfCodes = static_cast<unsigned char>(unwindCodeCount);
        m_unwindInfo->m_version = 1;
        m_unwindInfo->m_flags = 0;

        m_runtimeFunction.BeginAddress = CurrentPosition();
        m_runtimeFunction.EndAddress = BufferSize();

        m_prologueStart = CurrentPosition();
        PlaceLabel(m_entryPoint);

        // Push non-volatile registers
        // TODO: investigate whether pushing volatiles causes problems for stack unwinding.
        r = registerSaveMask;
        for (unsigned char i = 0 ; i < 16; ++i)
        {
            if (r & 1)
            {
                ProloguePushNonVolatile(i);
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
//            ProloguePushNonVolatile(RBP.Register());
            ProloguePushNonVolatile(RBP);
        }

        PrologueSetFrameRegister(slotCount);

      m_unwindInfo->m_sizeOfProlog = static_cast<unsigned char>(CurrentPosition() - m_prologueStart);

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


    // Creates an UnwindInfo structure in the code buffer.
    void FunctionBufferBase::AllocateUnwindInfo(unsigned unwindCodeCount)
    {
        if (unwindCodeCount > UnwindInfo::c_maxUnwindCodes)
        {
            throw std::runtime_error("Too many unwind codes.");
        }
        m_unwindInfo = (UnwindInfo*)Advance(sizeof(UnwindInfo));
        m_unwindCodePtr = m_unwindInfo->m_unwindCodes + unwindCodeCount;
    }


    void FunctionBufferBase::ProloguePushNonVolatile(unsigned __int8 r)
    {
        if (r <= 7)
        {
            Emit8(0x50u + r);
        }
        else
        {
            Emit8(0x49);
            Emit8(0x50 + (r & 7));
        }

        // TODO: Review this static_cast. Is there a bette way to do this?
        EmitUnwindCode(UnwindCode(static_cast<char>(CurrentPosition() - m_prologueStart),
                                  UWOP_PUSH_NONVOL,
                                  r));
        ++m_slotsAllocated;
    }


    void FunctionBufferBase::PrologueStackAllocate(unsigned __int8 slots)
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

        Op(OpCode::Sub, rsp, slots * 8);

        // TODO: Review this static_cast. Is there a bette way to do this?
        EmitUnwindCode(UnwindCode(static_cast<char>(CurrentPosition() - m_prologueStart),
                                  UWOP_ALLOC_SMALL,
                                  slots - 1));
        m_slotsAllocated += slots;
    }


    void FunctionBufferBase::PrologueSetFrameRegister(unsigned __int8 /*offset*/)
    {
        // TODO: Implement
        TODO();
    }


    void FunctionBufferBase::EmitUnwindCode(const UnwindCode& code)
    {
        // TODO: Buffer overflow handling. Perhaps this should use Emit8 instead of its own m_unwindCodePtr.
        *--m_unwindCodePtr = code;
    }


    void FunctionBufferBase::EpilogueUndoStackAllocate(unsigned __int8 /*slots*/)
    {
        // TODO: Implement
        TODO();
    }


    void FunctionBufferBase::EpilogueUndoFrameRegister(int /*frameRegister*/, unsigned __int8 /*offset*/)
    {
        // TODO: Implement
        TODO();
    }


    void FunctionBufferBase::EpiloguePopNonVolatile(unsigned __int8 /*r*/)
    {
        // TODO: Implement
        TODO();
    }


    bool FunctionBufferBase::UnwindInfoIsValid(std::ostream& /*out*/,
                                               RUNTIME_FUNCTION& /*runtimeFunction*/)
    {
        // TODO: Implement
        TODO();
        return false;
    }


    void FunctionBufferBase::FillWithBreakCode(unsigned start, unsigned length)
    {
        Fill(start, length, 0xcc);
    }
}
