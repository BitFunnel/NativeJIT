#pragma once

#include <Windows.h>            // For RUNTIME_FUNCTION


#include "..\NativeJIT\IAllocator.h"    // TODO: Move this file to public headers.
#include "..\NativeJIT\NonCopyable.h"   // TODO: Move this file to public headers.
#include "CodeBuffer.h"     // Embedded class.

namespace Allocators
{
    class IAllocator;
}


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


    class FunctionBufferBase : public NonCopyable
    {
    public:
        // Create CodeBuffer from memory in ExecutionBuffer.
        // Generate prologue.
        // Generate epilogue.
        // Generate unwind information. (Any possibility of unwinding multiple functions?)
        // Register unwind information.
        FunctionBufferBase(Allocators::IAllocator& allocator,
                           size_t capacity,
                           unsigned slotCount,
                           unsigned registerSaveMask,
                           bool isLeaf);

        // Unregister unwind informatio.
        // Return CodeBuffer memory to ExecutionBuffer (optional scenario).
        ~FunctionBufferBase();

        CodeBuffer& GetCodeBuffer();    // Idea: CodeBuffer has two constructors - one which takes a char* buffer and size.

        unsigned char * GetEntryPoint();


    protected:
        void EmitPrologue(unsigned slotCount,
                          unsigned registerSaveMask,
                          bool isLeaf);


        // Creates an UnwindInfo structure in the code buffer.
        void AllocateUnwindInfo(unsigned unwindCodeCount);

        void ProloguePushNonVolatile(unsigned __int8 r);
        void PrologueStackAllocate(unsigned __int8 slots);

        void PrologueSetFrameRegister(unsigned __int8 offset);

        void EmitUnwindCode(const UnwindCode& c);

        void EpilogueUndoStackAllocate(__int8 slots);
        void EpilogueUndoFrameRegister(int frameRegister, __int8 offset);
        void EpiloguePopNonVolatile(unsigned __int8 r);

        // Debugging function to help track down problems in unwind info.
        // This function exists, mainly to document all of the requirements that must
        // be satisfied in order to get X64 stack unwinding to work.
        bool UnwindInfoIsValid(std::ostream& out, RUNTIME_FUNCTION& runtimeFunction);

        Label m_entryPoint;

        // Structures used to register stack unwind information with Windows.
        UnwindInfo* m_unwindInfo;
        UnwindCode* m_unwindCodePtr;
        RUNTIME_FUNCTION m_runtimeFunction;

        // Buffer offset of first instruction in the prologue.
        size_t m_prologueStart;

        // Buffer offset of first instruction in function after the prologue.
        size_t m_bodyStart;

        // Total amount of stack used for return address, saved registers,
        // locals, etc.
        size_t m_slotsAllocated;

        // Offset of first stack local relative to frame pointer RBP.
        size_t m_stackLocalsBase;

        // Leaf functions do not call other functions or allocate space on the
        // stack.
        bool m_isLeaf;


        Allocators::IAllocator& m_allocator;
        CodeBuffer m_code;
    };


    template <typename FUNCTION>
    class FunctionBuffer : public FunctionBufferBase
    {
    public:
        // Create CodeBuffer from memory in ExecutionBuffer.
        // Generate prologue.
        // Generate epilogue.
        // Generate unwind information. (Any possibility of unwinding multiple functions?)
        // Register unwind information.
        FunctionBuffer(Allocators::IAllocator& allocator,
                       size_t capacity,
                       unsigned slotCount,
                       unsigned registerSaveMask,
                       bool isLeaf);

        FUNCTION* GetFunction();
    };


    //*************************************************************************
    //
    // FunctionBuffer template definitions.
    //
    //*************************************************************************
    template <typename FUNCTION>
    FunctionBuffer<FUNCTION>::FunctionBuffer(Allocators::IAllocator& allocator,
                                             size_t capacity,
                                             unsigned slotCount,
                                             unsigned registerSaveMask,
                                             bool isLeaf)
        : FunctionBufferBase(allocator,
                             capacity,
                             slotCount,
                             registerSaveMask,
                             isLeaf)
    {
        // Generate prologue.
        // Generate epilogue.
        // Generate unwind information. (Any possibility of unwinding multiple functions?)
        // Register unwind information.
    }


    template <typename FUNCTION>
    FUNCTION* FunctionBuffer<FUNCTION>::GetFunction()
    {
        // TODO: implement.
        // TODO: Should this return FUNCTION or FUNCTION*?
        return reinterpret_cast<FUNCTION*>(GetEntryPoint());
    }
}
