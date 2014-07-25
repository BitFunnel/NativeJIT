#include "stdafx.h"

#include <stdexcept>
#include <Windows.h>

#include "ExecutionBuffer.h"
#include "ExecutionBufferInternal.h"


namespace NativeJIT
{
    //*************************************************************************
    //
    // ExecutionBuffer
    //
    //*************************************************************************

    // Rounds x up to the nearest size, where size is a power of 2.
    size_t RoundUp(size_t  x, size_t powerOfTwo)
    {
        return (x + powerOfTwo - 1) & (~(powerOfTwo - 1));
    }


    ExecutionBuffer::ExecutionBuffer(size_t bufferSize)
        : m_buffer(nullptr),
          m_bytesAllocated(0)
    {
        SYSTEM_INFO systemInfo;
        GetSystemInfo(&systemInfo);

        m_bufferSize = RoundUp(bufferSize, systemInfo.dwPageSize);

        // Allocate m_bufferSize bytes plus one extra page that will act as 
        // a write-guard to detect buffer overruns.
        m_buffer = (unsigned char*)VirtualAlloc(NULL, m_bufferSize + systemInfo.dwPageSize, 
                                                MEM_COMMIT, PAGE_EXECUTE_READWRITE);

        if (m_buffer == NULL)
        {
            throw new std::runtime_error("CodeBuffer: out of memory.");
        }


        // Set protection on the guard page.
        DWORD oldProtection;
        if (!VirtualProtect(m_buffer + m_bufferSize, systemInfo.dwPageSize, 
                            PAGE_NOACCESS, &oldProtection))
        {
            throw new std::runtime_error("CodeBuffer: failed to set protection on guard page.");
        }


        //m_bufferEnd = m_bufferStart + size;
        //m_current = m_bufferStart;
    }


    ExecutionBuffer::~ExecutionBuffer()
    {
        if (m_buffer != nullptr)
        {
            if (VirtualFree(m_buffer, 0, MEM_RELEASE) == 0)
            {
                throw new std::runtime_error("CodeBuffer: VirtualFree failed.");
            }
        }
    }


    //
    // IAllocator methods
    //

    // Allocates a block of a specified byte size.
    void* ExecutionBuffer::Allocate(size_t size)
    {
        // Assert(m_bytesAllocated + size <= m_bufferSize, "Out of memory");

        void* result = static_cast<void*>(m_buffer + m_bytesAllocated);
        m_bytesAllocated += size;

        return result;
    }


    // Frees a block.
    void ExecutionBuffer::Deallocate(void* /*block*/)
    {
        //Assert(static_cast<char*>(block) > m_buffer.get() &&
        //       static_cast<char*>(block) < m_buffer.get() + m_bytesAllocated,
        //       "Attempting to deallocate memory not owned by this allocator.");

        // Intentional NOP
    }


    // Returns the maximum legal allocation size in bytes.
    size_t ExecutionBuffer::MaxSize() const
    {
        return m_bufferSize;
    }


    // Frees all blocks that have been allocated since construction or the
    // last call to Reset().
    void ExecutionBuffer::Reset()
    {
        m_bytesAllocated = 0;
        DebugInitialize();
    }


    void ExecutionBuffer::DebugInitialize()
    {
        memset(m_buffer, 0xcc, m_bufferSize);
    }


    //*************************************************************************
    //
    // FunctionBufferBase
    //
    //*************************************************************************
    FunctionBufferBase::FunctionBufferBase(Allocators::IAllocator& allocator,
                                           size_t capacity,
                                           unsigned slotCount,
                                           unsigned registerSaveMask,
                                           bool isLeaf)
        : m_allocator(allocator),
          m_code(capacity, 0, 0)
    {
        EmitPrologue(slotCount, registerSaveMask, isLeaf);
    }


    FunctionBufferBase::~FunctionBufferBase()
    {
    }


    CodeBuffer& FunctionBufferBase::GetCodeBuffer()
    {
        return m_code;
    }


    //void FunctionBufferBase::EmitPrologue(unsigned slotCount,
    //                                      unsigned registerSaveMask,
    //                                      bool isLeaf)
    //{
    //}


    // Creates an UnwindInfo structure in the code buffer.
    void FunctionBufferBase::AllocateUnwindInfo(unsigned unwindCodeCount)
    {
        if (unwindCodeCount > UnwindInfo::c_maxUnwindCodes)
        {
            throw std::runtime_error("Too many unwind codes.");
        }
        m_unwindInfo = (UnwindInfo*)m_code.Advance(sizeof(UnwindInfo));
        m_unwindCodePtr = m_unwindInfo->m_unwindCodes + unwindCodeCount;
    }


    void FunctionBufferBase::ProloguePushNonVolatile(unsigned __int8 r)
    {
        if (r <= 7)
        {
            m_code.Emit8(0x50u + r);
        }
        else
        {
            m_code.Emit8(0x49);
            m_code.Emit8(0x50 + (r & 7));
        }

        // TODO: Review this static_cast. Is there a bette way to do this?
        EmitUnwindCode(UnwindCode(static_cast<char>(m_code.CurrentPosition() - m_prologueStart),
                                  UWOP_PUSH_NONVOL,
                                  r));
        ++m_slotsAllocated;
    }


    void TODO()
    {
        throw 0;
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

        TODO();
//        SUB(RSP, slots * 8);
        // TODO: Review this static_cast. Is there a bette way to do this?
        EmitUnwindCode(UnwindCode(static_cast<char>(m_code.CurrentPosition() - m_prologueStart),
                                  UWOP_ALLOC_SMALL,
                                  slots - 1));
        m_slotsAllocated += slots;
    }


    //void FunctionBufferBase::PrologueSetFrameRegister(unsigned __int8 offset)
    //{
    //}


    //void FunctionBufferBase::EmitUnwindCode(const UnwindCode& c)
    //{
    //}


    //void FunctionBufferBase::EpilogueUndoStackAllocate(__int8 slots)
    //{
    //}


    //void FunctionBufferBase::EpilogueUndoFrameRegister(int frameRegister, __int8 offset)
    //{
    //}


    //void FunctionBufferBase::EpiloguePopNonVolatile(int r)
    //{
    //}


    //bool FunctionBufferBase::UnwindInfoIsValid(std::ostream& out,
    //                                           RUNTIME_FUNCTION& runtimeFunction)
    //{
    //}
}
