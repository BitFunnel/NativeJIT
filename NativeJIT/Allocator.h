#pragma once

#include <memory>

#include "IAllocator.h"

namespace NativeJIT
{
    class Allocator : public Allocators::IAllocator
    {
    public:
        Allocator(size_t bufferSize);

        virtual ~Allocator() override;

        // Allocates a block of a specified byte size.
        virtual void* Allocate(size_t size) override;

        // Frees a block.
        virtual void Deallocate(void* block) override;

        // Returns the maximum legal allocation size in bytes.
        virtual size_t MaxSize() const override;

        // Frees all blocks that have been allocated since construction or the
        // last call to Reset().
        virtual void Reset() override;

    private:
        void DebugInitialize();

        size_t m_bufferSize;
        size_t m_bytesAllocated;
        std::unique_ptr<char> m_buffer;
    };
}
