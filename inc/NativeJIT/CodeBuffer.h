#pragma once

#include <cstdint>

#include "NativeJIT/JumpTable.h"    // Label parameter and return value.
#include "Temporary/Assert.h"
#include "Temporary/NonCopyable.h"  // Base class.


namespace Allocators
{
    class IAllocator;
}


namespace NativeJIT
{
    class CodeBuffer : public NonCopyable
    {
    public:
        // Allocates a buffer from the code allocator. If the code inside
        // the buffer is to be executed (and not just f. ex. transferred over the
        // network), the allocator must return memory that is executable (see
        // ExecutionBuffer class for an example).
        //
        // The general allocator is used for allocating housekeeping data such
        // as labels, call sites etc. This allocator can be the same as the
        // codeBufferAllocator only when it's not allocating the executable
        // memory.
        CodeBuffer(Allocators::IAllocator& codeAllocator,
                   unsigned capacity,
                   Allocators::IAllocator& generalAllocator);

        // Frees the buffer.
        virtual ~CodeBuffer();

        // Allocating and resolving jump labels.
        //   Use AllocateLabel() at any time to allocated a label representing a jump target.
        //   Use PlaceLabel() to associate the current buffer position with a label.
        //   Once all code generation is done, invoke Finalize() to patch all of the call 
        //   sites with jump targets. Note that all allocated labels must be placed before 
        //   calling Finalize().
        Label AllocateLabel();
        virtual void PlaceLabel(Label label);

        // Writes a byte to the current position in the buffer.
        void Emit8(uint8_t x);

        // WARNING: Non portable. Assumes little endian machine architecture.
        void Emit16(uint16_t x);

        // WARNING: Non portable. Assumes little endian machine architecture.
        void Emit32(uint32_t x);

        // WARNING: Non portable. Assumes little endian machine architecture.
        void Emit64(uint64_t x);

        // Copies the provided data to the current position in the buffer.
        void EmitBytes(uint8_t const *data, unsigned length);

        // Writes the bits of the argument to the current position in the buffer.
        // WARNING: Non portable. Assumes little endian machine architecture.
        template <typename T>
        void EmitBytes(T x);

        // Replaces the contents of the buffer starting at startPosition and
        // length bytes long with specified data. The portion of the buffer that
        // will be changed with this call must already have been filled in
        // (i.e. CurrentPosition() <= startPosition + length). Besides the buffer
        // contents, no other CodeBuffer properties get modified. 
        void ReplaceBytes(unsigned startPosition, uint8_t const *data, unsigned length);

        // Return the size of the buffer, in bytes.
        // TODO: Rename GetCapacity()?
        unsigned BufferSize() const;

        // Returns the address of the start of the buffer.
        // WARNING: Depending on how this class is used, this may not be the
        // entry point to a function that's being built inside the buffer.
        // F. ex. FunctionBuffer provides the GetEntryPoint() method to retrieve
        // the function pointer.
        uint8_t* BufferStart() const;

        // Return the offset of the current write position in the buffer.
        unsigned CurrentPosition() const;

        virtual void Reset();

        // Advances the current write position by byteCount and returns a pointer to the write position
        // before advancing.
        uint8_t* Advance(int byteCount);

        template <typename T>
        void AdvanceToAlignment();

        void Fill(unsigned start, unsigned length, uint8_t value);

        // Patches each call site with the correct offset derived from its resolved label.
        void PatchCallSites();

    protected:
        void EmitCallSite(Label label, unsigned size);

    private:
        Allocators::IAllocator& m_codeAllocator;
        unsigned m_capacity;

        uint8_t* m_bufferStart;
        uint8_t* m_bufferEnd;
        uint8_t* m_current;

        JumpTable m_localJumpTable;    // Jumps within a single CodeBuffer.

        // Verifies that the specified length can be written to the buffer.
        // Throws if buffer overflow would occur.
        void VerifyNoBufferOverflow(unsigned length);
    };


    //*************************************************************************
    //
    // Template and inline definitions for CodeBuffer.
    //
    //*************************************************************************
    inline void CodeBuffer::VerifyNoBufferOverflow(unsigned length)
    {
        Assert(m_current + length - 1 < m_bufferEnd,
               "CodeBuffer overflow, wanted %u bytes, only %Iu out of %u bytes available",
               length,
               m_bufferEnd - m_current,
               m_capacity);
    }


    template <typename T>
    void CodeBuffer::EmitBytes(T x)
    {
        static_assert(std::is_trivially_copyable<T>::value, "Invalid variable type.");
        const size_t varSize = sizeof(T);

        VerifyNoBufferOverflow(varSize);

        *reinterpret_cast<T*>(m_current) = x;
        m_current += varSize;
    }


    template <typename T>
    void CodeBuffer::AdvanceToAlignment()
    {
        while ( (CurrentPosition() % sizeof(T)) != 0)
        {
            Emit8(0xaa);
        }
    }
}
