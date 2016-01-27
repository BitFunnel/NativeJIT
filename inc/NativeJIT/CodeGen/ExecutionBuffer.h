#pragma once

#include "NativeJIT/CodeGen/CodeBuffer.h"       // Embedded class.
#include "Temporary/IAllocator.h"


namespace NativeJIT
{
    class CodeBuffer;
    struct UnwindInfo;
    struct UnwindCode;

    class ExecutionBuffer : public Allocators::IAllocator
    {
    public:
        ExecutionBuffer(size_t bufferSize);

        virtual ~ExecutionBuffer() override;


        //
        // IAllocator methods
        //

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
        unsigned char* m_buffer;
    };
}
