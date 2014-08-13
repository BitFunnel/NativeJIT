#include "stdafx.h"

#include <stdexcept>

#include "NativeJIT/CodeBuffer.h"
#include "NativeJIT/JumpTable.h"
#include "Temporary/Allocator.h"
#include "Temporary/Assert.h"


namespace NativeJIT
{
    CodeBuffer::CodeBuffer(Allocators::IAllocator& allocator, 
                           unsigned capacity,
                           unsigned maxLabels,
                           unsigned maxCallSites)
        : m_allocator(allocator),
          m_bufferStart(nullptr),
          m_capacity(capacity),
          m_localJumpTable(new JumpTable(maxLabels, maxCallSites))
    {
        //m_bufferStart(allocator.Allocate(capacity)),
        //m_bufferEnd(buffer + capacity),
        //m_current(buffer),
        m_bufferStart = static_cast<unsigned __int8*>(allocator.Allocate(capacity));
        m_bufferEnd = m_bufferStart + capacity;
        m_current = m_bufferStart;
    }


    CodeBuffer::~CodeBuffer()
    {
        if (m_bufferStart != nullptr)
        {
            m_allocator.Deallocate(m_bufferStart);
        }
    }



    // Allocating and resolving jump labels.
    //   Use AllocateLabel() at any time to allocated a label representing a jump target.
    //   Use PlaceLabel() to associate the current buffer position with a label.
    //   Once all code generation is done, invoke Finalize() to patch all of the call 
    //   sites with jump targets. Note that all allocated labels must be placed before 
    //   calling Finalize().
    Label CodeBuffer::AllocateLabel()
    {
        return m_localJumpTable->AllocateLabel();
    }


//    Label CodeBuffer::AllocateGlobalLabel()

    void CodeBuffer::PlaceLabel(Label label)
    {
        m_localJumpTable->PlaceLabel(label, m_current);
    }


    // Writes a byte to the current position in the buffer.
    void CodeBuffer::Emit8(unsigned __int8 x)
    {
        // DESIGN NOTE: Intentionally omitting check for buffer overrun.
        // This code relies on a protected guard page immedidately after the buffer
        // to detect overruns.
        //
        // Temporarily add an if check to make sure if buffer overrun happens, a C++
        // exception is thrown so that the upper layer component can catch the exception.
        // TFS 10628 is opened to evaluate the performance and the necessity of this change
        // in the near future. The DESIGN NOTE comment is intentionally left untouched
        // because a final decision of what should be done to handle buffer overrun will be
        // evaluated when TFS 10628 is resolved.
        if (m_current < m_bufferEnd)
        {
            *m_current++ = x;
        }
        else
        {
            throw std::runtime_error("NativeJIT code buffer overrun.");
        }
    }


    // WARNING: Non portable. Assumes little endian machine architecture.
    void CodeBuffer::Emit16(unsigned __int16 x)
    {
        // WARNING: Non portable. Assumes little endian machine architecture.

        // DESIGN NOTE: Intentionally omitting check for buffer overrun.
        // This code relies on a protected guard page immedidately after the buffer
        // to detect overruns.
        //
        // Temporarily add an if check to make sure if buffer overrun happens, a C++
        // exception is thrown so that the upper layer component can catch the exception.
        // TFS 10628 is opened to evaluate the performance and the necessity of this change
        // in the near future. The DESIGN NOTE comment is intentionally left untouched
        // because a final decision of what should be done to handle buffer overrun will be
        // evaluated when TFS 10628 is resolved.
        if (m_current + 1 < m_bufferEnd)
        {
            *((unsigned __int16*)m_current) = x;
            m_current = (unsigned char*)((unsigned __int16*)m_current + 1);
        }
        else
        {
            throw std::runtime_error("X64 code generator code buffer overrun.");
        }
    }


    // WARNING: Non portable. Assumes little endian machine architecture.
    void CodeBuffer::Emit32(unsigned __int32 x)
    {
        // WARNING: Non portable. Assumes little endian machine architecture.

        // DESIGN NOTE: Intentionally omitting check for buffer overrun.
        // This code relies on a protected guard page immedidately after the buffer
        // to detect overruns.
        //
        // Temporarily add an if check to make sure if buffer overrun happens, a C++
        // exception is thrown so that the upper layer component can catch the exception.
        // TFS 10628 is opened to evaluate the performance and the necessity of this change
        // in the near future. The DESIGN NOTE comment is intentionally left untouched
        // because a final decision of what should be done to handle buffer overrun will be
        // evaluated when TFS 10628 is resolved.
        if (m_current + 3 < m_bufferEnd)
        {
            *((unsigned __int32*)m_current) = x;
            m_current = (unsigned char*)((unsigned __int32*)m_current + 1);
        }
        else
        {
            throw std::runtime_error("X64 code generator code buffer overrun.");
        }
    }


    // WARNING: __forceinline is essential to system performance. Please see
    // note on constant folding in X64CodeGenerator.h.
    void CodeBuffer::Emit64(unsigned __int64 x)
    {
        // WARNING: Non portable. Assumes little endian machine architecture.

        // DESIGN NOTE: Intentionally omitting check for buffer overrun.
        // This code relies on a protected guard page immedidately after the buffer
        // to detect overruns.
        //
        // Temporarily add an if check to make sure if buffer overrun happens, a C++
        // exception is thrown so that the upper layer component can catch the exception.
        // TFS 10628 is opened to evaluate the performance and the necessity of this change
        // in the near future. The DESIGN NOTE comment is intentionally left untouched
        // because a final decision of what should be done to handle buffer overrun will be
        // evaluated when TFS 10628 is resolved.
        if (m_current + 7 < m_bufferEnd)
        {
            *((unsigned __int64*)m_current) = x;
            m_current = (unsigned char*)((unsigned __int64*)m_current + 1);
        }
        else
        {
            throw std::runtime_error("X64 code generator code buffer overrun.");
        }
    }


    // Return the size of the buffer, in bytes.
    unsigned CodeBuffer::BufferSize() const
    {
        return m_capacity;
    }

    // Returns the address of the start of the buffer.
    // WARNING: Do not use this function to get the address of your generated code.
    // The buffer may contain unwind information before the code starts, and Finalize()
    // must be called before executing the code. 
    // Use X64FunctionGenerator:EntryPoint() instead.
    // TODO: Make protected. Currently available as a public function to enable unit tests.
    unsigned __int8* CodeBuffer::BufferStart() const
    {
        return m_bufferStart;
    }

    // Return the offset of the current write position in the buffer.
    unsigned CodeBuffer::CurrentPosition() const
    {
        return static_cast<unsigned>(m_current - m_bufferStart);
    }


    void CodeBuffer::Reset(unsigned position)
    {
        Assert(position < m_capacity, "Invalid position.");
        m_current = m_bufferStart + position;
        m_localJumpTable->Clear();
    }


    // Advances the current write position by byteCount and returns a pointer to the write position
    // before advancing.
    unsigned char* CodeBuffer::Advance(int byteCount)
    {
        // TODO: Add check for buffer overflow.

        unsigned char* start = m_current;
        m_current += byteCount;
        return start;
    }


    void CodeBuffer::Fill(unsigned start, unsigned length, unsigned char value)
    {
        memset(m_bufferStart + start, value, length);
    }


    void CodeBuffer::PatchCallSites()
    {
        m_localJumpTable->PatchCallSites();
    }


    void CodeBuffer::EmitCallSite(Label label, unsigned size)
    {
        m_localJumpTable->AddCallSite(label, m_current, size);

#ifdef _DEBUG
        while (size-- > 0)
        {
            Emit8(0);
        }
#else
        Advance(size);
#endif
    }
}
