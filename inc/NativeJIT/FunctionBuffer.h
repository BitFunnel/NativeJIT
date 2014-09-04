#pragma once

#include <ostream>                          // ostream& paramter.
#include <vector>                           // std::vector embedded.
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

    // TODO: Add methods to copy data in.
    // TODO: Add methods to extract data. EntryPoint and Size. Is prologue sent over the wire?
    //       Perhaps unwind data should be sent as well and verified.
    // TODO: Verify that generating and executing function buffers use same parameters.
    // (or at least that numbers of slots, labels, and call sites are compatible.
    class FunctionBuffer : public X64CodeGenerator
    {
    public:
        FunctionBuffer(Allocators::IAllocator& allocator,
                       unsigned capacity,
                       unsigned maxLabels,
                       unsigned maxCallSites,
                       unsigned slotCount,
                       unsigned registerSaveMask,
                       bool isLeaf);

        unsigned char const * GetEntryPoint() const;

        void EmitPrologue();
        void EmitEpilogue();

        void SaveVolatileRegisters();
        void RestoreVolatileRegisters();

        void Reset();


    private:
        void EmitUnwindInfo(unsigned char slotCount,
                            unsigned registerSaveMask,
                            bool isLeaf);

        void CreatePrologue();
        void CreateEpilogue();

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

        std::vector<unsigned __int8> m_prologueCode; 
        std::vector<unsigned __int8> m_epilogueCode; 
        unsigned char const * m_entryPoint;

        // Structures used to register stack unwind information with Windows.
        UnwindInfo* m_unwindInfo;
        UnwindCode* m_unwindCodePtr;
        RUNTIME_FUNCTION m_runtimeFunction;

        // Buffer offset of first instruction in the prologue.
        size_t m_prologueStart;

        // Buffer offset of the first byte after the unwind info.
        unsigned m_codeGenStart;

        // Total amount of stack used for return address, saved registers,
        // locals, etc.
        size_t m_slotsAllocated;

        // Offset of first stack local relative to frame pointer RBP.
        size_t m_stackLocalsBase;

        // Leaf functions do not call other functions or allocate space on the
        // stack.
//        bool m_isLeaf;
    };
}
