#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/Function.h"
#include "Temporary/Allocator.h"

#include <iostream>

using NativeJIT::Allocator;
using NativeJIT::ExecutionBuffer;
using NativeJIT::Function;
using NativeJIT::FunctionBuffer;

///////////////////////////////////////////////////////////////////////////////
//
// This example JITs a function that computes the area of a circle, given
// its radius.
//
// Here is a disassembly of the generated code.
//
//
// PI_CONSTANT:
//     db 0f 49 40                              ; PI constant is stored in memory.
// ENTRY_POINT:
//     sub    rsp, 28h                          ; Standard function prolog
//     mov    qword ptr[rsp], rbp
//     movaps xmmword ptr[rsp + 10h], xmm15
//     lea    rbp, [rsp + 28h]
//
//     movss  xmm15, xmm0                       ; Load radius from first parameter register.
//     mulss  xmm15, xmm0                       ; Multiply by radius.
//     mulss  xmm15, dword ptr[PI_CONSTANT]     ; Multiply by PI.
//     movss  xmm0, xmm15                       ; Return value goes in xmm0.
//
//     movaps xmm15, xmmword ptr[rsp + 10h]     ; Standard function epilog.
//     mov    rbp, qword ptr[rsp]
//     add    rsp, 28h
//     ret
//
// TODO: There seems to be a bug in the regster allocator. The generated
// code never should have used xmm15. It should have been
//     mulss xmm0, xmm0
//     mulss xmm0, dword ptr[PI_CONSTANT]
//
///////////////////////////////////////////////////////////////////////////////
int main()
{
    // Create allocator and buffers for pre-compiled and post-compiled code.
    ExecutionBuffer codeAllocator(8192);
    Allocator allocator(8192);
    FunctionBuffer code(codeAllocator, 8192);

    // Create the factory for expression nodes.
    // Our area expression will take a single float parameter and return a float.
    Function<float, float> expression(allocator, code);

    // Multiply input parameter by itself to get radius squared.
    auto & rsquared = expression.Mul(expression.GetP1(), expression.GetP1());

    // Multiply by PI.
    const float  PI = 3.14159265358979f;
    auto & area = expression.Mul(rsquared, expression.Immediate(PI));

    // Compile expression into a function.
    auto function = expression.Compile(area);

    // Now run our expression!
    float radius = 2.0;
    std::cout << "The area of a circle with radius " << radius
              << " is " << function(radius) << "." << std::endl;

    return 0;
}
