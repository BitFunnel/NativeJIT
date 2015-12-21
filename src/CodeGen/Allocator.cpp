#include "stdafx.h"

#include "Temporary/Allocator.h"
#include "Temporary/Assert.h"


namespace NativeJIT
{
    //*************************************************************************
    //
    // Allocator
    //
    //*************************************************************************
    Allocator::Allocator(size_t bufferSize)
        : m_bufferSize(bufferSize),
          m_bytesAllocated(0),
          m_buffer(new char[bufferSize])
    {
        DebugInitialize();
    }


    Allocator::~Allocator()
    {
    }


    void* Allocator::Allocate(size_t size)
    {
        Assert(m_bytesAllocated + size <= m_bufferSize, "Out of memory");

        void* result = static_cast<void*>(m_buffer.get() + m_bytesAllocated);
        m_bytesAllocated += size;

        return result;
    }


    void Allocator::Deallocate(void* block)
    {
        Assert(static_cast<char*>(block) >= m_buffer.get() &&
               static_cast<char*>(block) < m_buffer.get() + m_bytesAllocated,
               "Attempting to deallocate memory not owned by this allocator.");

        // Intentional NOP
    }


    size_t Allocator::MaxSize() const
    {
        return m_bufferSize;
    }


    void Allocator::Reset()
    {
        m_bytesAllocated = 0;
        DebugInitialize();
    }


    void Allocator::DebugInitialize()
    {
        memset(m_buffer.get(), 0xcc, m_bufferSize);
    }


    //*************************************************************************
    //
    // AutoResetAllocator
    //
    //*************************************************************************

    AutoResetAllocator::AutoResetAllocator(Allocators::IAllocator& allocator)
        : m_allocator(allocator)
    {
    }


    AutoResetAllocator::~AutoResetAllocator()
    {
        m_allocator.Reset();
    }

}
