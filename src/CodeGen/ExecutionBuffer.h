#pragma once


namespace NativeJIT
{
    template <typename FUNCTION, unsigned SLOTS, unsigned REGISTERS, bool ISLEAF>
    class FunctionBuffer
    {
    public:
        // Create CodeBuffer from memory in ExecutionBuffer.
        // Generate prologue.
        // Generate epilogue.
        // Generate unwind information. (Any possibility of unwinding multiple functions?)
        // Register unwind information.
        FunctionBuffer(IAllocator& allocator, unsigned capacity);

        // Unregister unwind informatio.
        // Return CodeBuffer memory to ExecutionBuffer (optional scenario).
        ~FunctionBuffer();

        CodeBuffer& GetCodeBuffer();    // Idea: CodeBuffer has two constructors - one which takes a char* buffer and size.
        FUNCTION* GetEntryPoint();


        // Some method to fill code buffer. This can be operator= on CodeBuffer or some byte copy method on CodeBuffer.

        void EmitEpilogueAndFinalize();

    private:
        IAllocator& allocator;
        CodeBuffer m_code;
    };

    //class ExecutionBuffer
    //{
    //public:
    //    // Virtual alloc buffer with data execution prevention disabled.
    //    ExecutionBuffer(unsigned capacity);
    //    ~ExecutionBuffer();

    //    //template <typename FUNCTION>
    //    //Function<FUNCTION>& AddFunction(unsigned size);
    //    void* Allocate(unsigned size);

    //};
}
