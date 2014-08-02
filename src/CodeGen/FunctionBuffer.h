#pragma once

#include <ostream>                          // ostream& paramter.
#include <Windows.h>                        // RUNTIME_FUNCTION embedded.


#include "NativeJIT/X64CodeGenerator.h"     // Inherits from X64CodeGenerator.


namespace Allocators
{
    class IAllocator;
}


namespace NativeJIT
{
    struct UnwindInfo;
    struct UnwindCode;

    class FunctionBufferBase : public X64CodeGenerator
    {
    public:
        FunctionBufferBase(Allocators::IAllocator& allocator,
                           unsigned capacity,
                           unsigned slotCount,
                           unsigned registerSaveMask,
                           bool isLeaf);

        ~FunctionBufferBase();

        unsigned char const * GetEntryPoint() const;


    private:
        void EmitUnwindInfo(unsigned char slotCount,
                            unsigned registerSaveMask,
                            bool isLeaf);

        void EmitEpilogue();
        void EmitPrologue();
        void RegisterUnwindInfo();

        // Creates an UnwindInfo structure in the code buffer.
        void AllocateUnwindInfo(unsigned unwindCodeCount);

        void ProloguePushNonVolatile(Register<8, false> r);
        void PrologueStackAllocate(unsigned __int8 slots);
        void PrologueSetFrameRegister(unsigned __int8 offset);

        void EmitUnwindCode(const UnwindCode& code);

        // Debugging function to help track down problems in unwind info.
        // This function exists, mainly to document all of the requirements that must
        // be satisfied in order to get X64 stack unwinding to work.
        bool UnwindInfoIsValid(std::ostream& out, RUNTIME_FUNCTION& runtimeFunction);

        void FillWithBreakCode(unsigned start, unsigned length);

        Label m_epilogue;
        unsigned char const * m_entryPoint;

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
//        bool m_isLeaf;


        Allocators::IAllocator& m_allocator;
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
                       unsigned capacity,
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
                                             unsigned capacity,
                                             unsigned slotCount,
                                             unsigned registerSaveMask,
                                             bool isLeaf)
        : FunctionBufferBase(allocator,
                             capacity,
                             slotCount,
                             registerSaveMask,
                             isLeaf)
    {
    }


    template <typename FUNCTION>
    FUNCTION* FunctionBuffer<FUNCTION>::GetFunction()
    {
        // TODO: implement.
        // TODO: Should this return FUNCTION or FUNCTION*?
        return reinterpret_cast<FUNCTION*>(const_cast<unsigned char *>(GetEntryPoint()));
    }
}
