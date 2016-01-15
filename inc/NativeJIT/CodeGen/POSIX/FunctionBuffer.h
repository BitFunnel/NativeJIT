#pragma once

//#include <windows.h>                                // RUNTIME_FUNCTION embedded.

#include "NativeJIT/CodeGen/X64CodeGenerator.h"     // Inherits from X64CodeGenerator.

typedef uint32_t DWORD;

typedef struct
{
    DWORD BeginAddress;
    DWORD EndAddress;
    union {
        DWORD UnwindInfoAddress;
        DWORD UnwindData;
    } DUMMYUNIONNAME;
} RUNTIME_FUNCTION;


namespace NativeJIT
{
    class FunctionSpecification;

    class FunctionBufferBase : public X64CodeGenerator
    {
    public:
        // Sets up a code buffer with specified capacity and registers a
        // callback to facilitate stack unwinding on exception. See the
        // CodeBuffer constructor for more details on allocators.
        FunctionBufferBase(Allocators::IAllocator& codeAllocator,
                           unsigned capacity,
                           Allocators::IAllocator& generalAllocator);

        // Deregisters the stack unwinding callback.
        ~FunctionBufferBase();

        // Returns the entry point to the function, i.e. untyped function
        // pointer.
        void const * GetEntryPoint() const;

        // The following functions return the information about the contents
        // of the buffer. The offsets are relative to the beginning of the
        // buffer. The start and end offset describe the [start, end) range
        // containing the function's code (including prolog and epilog). The
        // unwind info offset specifies where UnwinInfo is located.
        // These calls are valid only after the function body has been generated.
        unsigned GetFunctionCodeStartOffset() const;
        unsigned GetFunctionCodeEndOffset() const;
        unsigned GetUnwindInfoStartOffset() const;

        // Called by clients to mark that generation of function's body has
        // begun. If function specification is known even before the function
        // body is generated, the exact space needed for unwind information and
        // function prolog is reserved in front of the function body. Otherwise,
        // the maximum allowed space is reserved. In both cases, no data is
        // written for unwind info and prolog at this point.
        void BeginFunctionBodyGeneration(FunctionSpecification const & spec);
        void BeginFunctionBodyGeneration();

        // Called by clients to mark that generation of function's body has
        // completed. At this point, unwind info and prolog are filled in
        // the space previously reserved by BeginFunctionBodyGeneration().
        // Then, epilog is written after the function body and all call sites
        // patched with the actual values.
        void EndFunctionBodyGeneration(FunctionSpecification const & spec);

        // Resets the buffer to the same state it had after its construction.
        virtual void Reset() override;

    private:
        // Structure used to register stack unwind information with Windows.
        RUNTIME_FUNCTION m_runtimeFunction;

        // Temporary values used during compilation of the function. Offsets
        // are relative to buffer start. When using BeginFunctionBodyGeneration()
        // without parameters, the length values represent the reserved (maximum
        // available) length for the respective section.
        unsigned m_unwindInfoStartOffset;
        unsigned m_unwindInfoByteLength;
        unsigned m_prologStartOffset;
        unsigned m_prologLength;
        bool m_isCodeGenerationCompleted;

        // The callback function for RtlInstallFunctionTableCallback. Context
        // is a poiner to a FunctionBuffer.
//        static RUNTIME_FUNCTION*
//        WindowsGetRuntimeFunctionCallback(DWORD64 controlPc, void* context);

        // A helper method used to implement the two public flavors of the method.
        void BeginFunctionBodyGeneration(unsigned reservedUnwindInfoLength,
                                         unsigned reservedPrologLength);
    };
}
