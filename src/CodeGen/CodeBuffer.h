#pragma once


namespace NativeJIT
{
    typedef unsigned Label; // TODO: Signed or unsigned?

    class CodeBuffer
    {
    public:
        CodeBuffer(unsigned capacity, unsigned maxLabels, unsigned maxCallSites);
        ~CodeBuffer();

        // Allocating and resolving jump labels.
        //   Use AllocateLabel() at any time to allocated a label representing a jump target.
        //   Use PlaceLabel() to associate the current buffer position with a label.
        //   Once all code generation is done, invoke Finalize() to patch all of the call 
        //   sites with jump targets. Note that all allocated labels must be placed before 
        //   calling Finalize().
        Label AllocateLabel();
        Label AllocateGlobalLabel();
        void PlaceLabel(Label label);

        // Writes a byte to the current position in the buffer.
        void Emit8(unsigned __int8 x);

        // WARNING: Non portable. Assumes little endian machine architecture.
        void Emit32(unsigned __int32 x);

        // WARNING: Non portable. Assumes little endian machine architecture.
        void Emit64(unsigned __int64 x);

        // Return the size of the buffer, in bytes.
        int BufferSize() const;

        // Returns the address of the start of the buffer.
        // WARNING: Do not use this function to get the address of your generated code.
        // The buffer may contain unwind information before the code starts, and Finalize()
        // must be called before executing the code. 
        // Use X64FunctionGenerator:EntryPoint() instead.
        // TODO: Make protected. Currently available as a public function to enable unit tests.
        const unsigned char* BufferStart() const;

        // Return the offset of the current write position in the buffer.
        int CurrentPosition() const;
    };
}
