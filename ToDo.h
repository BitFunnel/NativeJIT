/*
JITExample1 is showing redundant adds when collapsing field pointers.
        === Pass3 ===
        add r9, 1234
        add r9, [r8]
        // release r8
        mul rdx, 40
        add rcx, rdx
        // release rdx
    ==> add rcx, 16
        mov rcx, [rcx + 8]
    ==> add rcx, 16
        add r9, [rcx + 8]
        // release rcx
        mov rax, r9
        ret
        // release r9


. Implement BinaryNode operation.

RegisterFile should exclude certain machine-specific registerrs (e.g. RSP).
x ExpressionTree::GetAvailableRegisterCount needs to handle all types of registers.
x Implement and use ExpressionTree::ReleaseRegister()
x Use templates to simplify implementation of ExpressionTree::AllocateRegister().
Implement ExpressionNodeFactory with arena allocator.
Implement factory method for add integer to pointer.
Implement call node with various parameters.
Implement support for doubles.
Implement Prologue and Epilogue with patching for locals.
Implement register dumping and local variables.
Implement unwind information.
Unit test that executes generated code.
Pack and unpack operations.
Comprehensive unit tests.
Examples.
Implement real code generator.

Conditional nodes
  Boolean expressions need special flag mode optimization so that boolean value doesn't have to be generated.

Call node
  ParameterHome
  ParameterHomeSequence
    Determine amount of parameter home space required and allocate on stack
    Evaluate each parameter home expression and transfer to correct register
    Sometimes need to save and restore a register that was bumped for a parameter home
    Make the call
    Restore bumped registers
    Deallocate space

Sequence node - like C++ comma operator.

Rename Immediate.h ==> ImmediateNode.h
Rename FieldPointer.h ==> FieldPointerNode.h
Rename Parameter.h ==> ParameterNode.h
Rename Return.h ==> ReturnNode.h
Rename other node classes
Are Deref and Indirect always the same? (or does Factory::FieldPointer() sometimes need to create Indirect?

*/
