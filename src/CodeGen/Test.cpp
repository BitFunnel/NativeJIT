#include "stdafx.h"

#include <iostream>

#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"


namespace NativeJIT
{
    typedef int (*F)(int, int);


    template <typename R, typename P1>
    unsigned ParameterCount(R (*)(P1))
    {
        return 1;
    }


    template <typename R, typename P1, typename P2>
    unsigned ParameterCount(R (*)(P1, P2))
    {
        return 2;
    }


    int A(int, int)
    {
        return 0;
    }

    int B(int)
    {
        return 0;
    }

    void Test()
    {
        std::cout << ParameterCount(&A);
        std::cout << ParameterCount(&B);

        ExecutionBuffer buffer(1000);
        FunctionBuffer<F> function(buffer, 500, 1, 1, false);

        (*function.GetFunction())(1,2);

        //function.Emit(OpCode::Add, rax, rbx);
        //function.Emit(OpCode::Add, xmm0, xmm1);
    }
}
