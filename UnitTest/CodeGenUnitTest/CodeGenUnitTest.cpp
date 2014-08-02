// CodeGenUnitTest.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "iostream"             // TODO: Temporary - for debugging.

#include "ExecutionBuffer.h"
#include "FunctionBuffer.h"
#include "SuiteCpp/UnitTest.h"

namespace NativeJIT
{
    namespace CodeGenUnitTest
    {
        TestCase(SampleTest)
        {
            TestAssert(true, "Test passed.");
        }


        TestCase(ExecutionBuffer)
        {
            ExecutionBuffer allocator(1000);
            FunctionBufferBase buffer(allocator, 200, 3, 0, false);

            std::cout << "Add" << std::endl;

            buffer.Emit<OpCode::Add>(rax, rbx);
            std::cout << std::endl;

            buffer.Emit<OpCode::Add>(al, bl);
            std::cout << std::endl;

            buffer.Emit<OpCode::Add>(rax, rbx, 0x12);
            std::cout << std::endl;

            buffer.Emit<OpCode::Add>(rax, rbx, 0x12345678);
            std::cout << std::endl;

            buffer.Emit<OpCode::Add>(al, static_cast<__int8>(0x12));
            std::cout << std::endl;

            buffer.Emit<OpCode::Add>(rax, 0x12);
            std::cout << std::endl;

            buffer.Emit<OpCode::Add>(rax, 0x12345678);
            std::cout << std::endl;



            std::cout << "Sub/Cmp" << std::endl;

            buffer.Emit<OpCode::Sub>(rax, rbx);
            std::cout << std::endl;

            buffer.Emit<OpCode::Cmp>(rax, r9);
            std::cout << std::endl;



            std::cout << "Lea" << std::endl;

            buffer.Emit<OpCode::Lea>(rax, rbx, 0x12);
            std::cout << std::endl;



            std::cout << "Pop/Push" << std::endl;

            buffer.Emit<OpCode::Pop>(rax);
            std::cout << std::endl;

            buffer.Emit<OpCode::Pop>(r8);
            std::cout << std::endl;

            buffer.Emit<OpCode::Push>(rax);
            std::cout << std::endl;

            buffer.Emit<OpCode::Push>(r8);
            std::cout << std::endl;

            buffer.Emit<OpCode::Ret>();
            std::cout << std::endl;
        }
    }
}

