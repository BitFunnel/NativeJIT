#include "stdafx.h"

#include <cstring>
#include <stdexcept>

#include "NativeJIT/CodeBuffer.h"
#include "NativeJIT/JumpTable.h"
#include "Temporary/Assert.h"
#include "Temporary/IAllocator.h"


namespace NativeJIT
{
    CodeBuffer::CodeBuffer(Allocators::IAllocator& codeAllocator,
                   unsigned capacity,
                   Allocators::IAllocator& generalAllocator)
        : m_codeAllocator(codeAllocator),
          m_bufferStart(nullptr),
          m_capacity(capacity),
          m_localJumpTable(generalAllocator)
    {
        m_bufferStart = static_cast<uint8_t*>(codeAllocator.Allocate(capacity));
        m_bufferEnd = m_bufferStart + capacity;
        m_current = m_bufferStart;
    }


    CodeBuffer::~CodeBuffer()
    {
        m_codeAllocator.Deallocate(m_bufferStart);
    }


    Label CodeBuffer::AllocateLabel()
    {
        return m_localJumpTable.AllocateLabel();
    }


    void CodeBuffer::PlaceLabel(Label label)
    {
        m_localJumpTable.PlaceLabel(label, m_current);
    }


    void CodeBuffer::EmitBytes(uint8_t const *data, unsigned length)
    {
        VerifyNoBufferOverflow(length);

        memcpy(m_current, data, length);
        m_current += length;
    }


    void CodeBuffer::Emit8(uint8_t x)
    {
        EmitBytes(x);
    }


    void CodeBuffer::Emit16(uint16_t x)
    {
        EmitBytes(x);
    }


    void CodeBuffer::Emit32(uint32_t x)
    {
        EmitBytes(x);
    }


    void CodeBuffer::Emit64(uint64_t x)
    {
        EmitBytes(x);
    }


    void CodeBuffer::ReplaceBytes(unsigned startPosition,
                                  uint8_t const *data,
                                  unsigned length)
    {
        Assert(startPosition + length <= CurrentPosition(),
               "Cannot replace parts of the buffer that have not been populated "
               "(populated: [0, %u), wanted [%u, %u))",
               CurrentPosition(),
               startPosition,
               startPosition + length);

        memmove(&m_bufferStart[startPosition], data, length);
    }


    unsigned CodeBuffer::BufferSize() const
    {
        return m_capacity;
    }


    uint8_t* CodeBuffer::BufferStart() const
    {
        return m_bufferStart;
    }


    unsigned CodeBuffer::CurrentPosition() const
    {
        return static_cast<unsigned>(m_current - m_bufferStart);
    }


    void CodeBuffer::Reset()
    {
        m_current = m_bufferStart;
        m_localJumpTable.Clear();
    }


    uint8_t* CodeBuffer::Advance(int byteCount)
    {
        VerifyNoBufferOverflow(byteCount);

        uint8_t* start = m_current;
        m_current += byteCount;
        return start;
    }


    void CodeBuffer::Fill(unsigned start, unsigned length, uint8_t value)
    {
        VerifyNoBufferOverflow(length);
        memset(m_bufferStart + start, value, length);
    }


    void CodeBuffer::PatchCallSites()
    {
        m_localJumpTable.PatchCallSites();
    }


    void CodeBuffer::EmitCallSite(Label label, unsigned size)
    {
        m_localJumpTable.AddCallSite(label, m_current, size);

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
