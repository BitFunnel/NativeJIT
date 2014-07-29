#include "stdafx.h"

#include <stdexcept>

#include "CodeBuffer.h"
#include "JumpTable.h"


namespace NativeJIT
{
    CodeBuffer::CodeBuffer(unsigned __int8* buffer, 
                           unsigned capacity,
                           unsigned maxLabels,
                           unsigned maxCallSites)
        : m_bufferStart(buffer),
          m_bufferEnd(buffer + capacity),
          m_current(buffer),
          m_capacity(capacity),
          m_localJumpTable(new JumpTable(maxLabels, maxCallSites))
    {
    }

    //CodeBuffer::~CodeBuffer()
    //{
    //}

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

    //// WARNING: Non portable. Assumes little endian machine architecture.
    //void CodeBuffer::Emit32(unsigned __int32 x);

    //// WARNING: Non portable. Assumes little endian machine architecture.
    //void CodeBuffer::Emit64(unsigned __int64 x);

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
}
