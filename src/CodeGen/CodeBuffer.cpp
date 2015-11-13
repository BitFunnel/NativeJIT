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
          m_localJumpTable(maxLabels, maxCallSites)
    {
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


    Label CodeBuffer::AllocateLabel()
    {
        return m_localJumpTable.AllocateLabel();
    }


    void CodeBuffer::PlaceLabel(Label label)
    {
        m_localJumpTable.PlaceLabel(label, m_current);
    }


    template <typename T>
    void CodeBuffer::EmitBytes(T x)
    {
        static_assert(std::is_trivially_copyable<T>::value, "Invalid variable type.");
        const size_t varSize = sizeof(T);

        if (m_current + (varSize - 1) >= m_bufferEnd)
        {
            throw std::runtime_error("NativeJIT code buffer overrun.");
        }

        *reinterpret_cast<T*>(m_current) = x;
        m_current += varSize;
    }


    void CodeBuffer::Emit8(unsigned __int8 x)
    {
        EmitBytes(x);
    }


    void CodeBuffer::Emit16(unsigned __int16 x)
    {
        EmitBytes(x);
    }


    void CodeBuffer::Emit32(unsigned __int32 x)
    {
        EmitBytes(x);
    }


    void CodeBuffer::Emit64(unsigned __int64 x)
    {
        EmitBytes(x);
    }


    // TODO: Remove this method and make EmitBytes public if __forceinline needs
    // to be used for all Emit*() methods. This (non-forceinline) method exists
    // so that the outside clients don't get the code inlined at the call site but
    // rather make a regular function call. Also, users from within
    // CodeBuffer.cpp such as Emit8() call the private forceinline EmitBytes()
    // method instead and do get its contents inlined, as it is currently intended.
    template <typename T>
    void CodeBuffer::EmitValueBytes(T x)
    {
        EmitBytes(x);
    }


    // Only a limited number of instantiations available until the __forceinline
    // decision is made.
    template void CodeBuffer::EmitValueBytes(float x);
    template void CodeBuffer::EmitValueBytes(double x);
    template void CodeBuffer::EmitValueBytes(__int64 x);
    template void CodeBuffer::EmitValueBytes(unsigned __int64 x);


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
        m_localJumpTable.Clear();
    }


    // Advances the current write position by byteCount and returns a pointer to the write position
    // before advancing.
    unsigned __int8* CodeBuffer::Advance(int byteCount)
    {
        if (m_current + byteCount - 1 >= m_bufferEnd)
        {
            throw std::runtime_error("NativeJIT code buffer overrun.");
        }

        unsigned __int8* start = m_current;
        m_current += byteCount;
        return start;
    }


    void CodeBuffer::Fill(unsigned start, unsigned length, unsigned __int8 value)
    {
        if (m_bufferStart + start + length - 1 >= m_bufferEnd)
        {
            throw std::runtime_error("NativeJIT code buffer overrun.");
        }

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
