#include "stdafx.h"

#include <cctype>               // isxdigit().
#include <iostream>             // TODO: Temporary - for debugging.
#include <vector>

#include "ExecutionBuffer.h"
#include "FunctionBuffer.h"
#include "ML64Verifier.h"
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
            //ExecutionBuffer allocator(1000);
            //FunctionBufferBase buffer(allocator, 200, 3, 0, false);

            //std::cout << "Add" << std::endl;

            //buffer.Emit<OpCode::Add>(rax, rbx);
            //buffer.Emit<OpCode::Add>(al, bl);
            //buffer.Emit<OpCode::Add>(rax, rbx, 0x12);
            //buffer.Emit<OpCode::Add>(rax, rbx, 0x12345678);
            //buffer.Emit<OpCode::Add>(al, static_cast<__int8>(0x12));
            //buffer.Emit<OpCode::Add>(rax, 0x12);
            //buffer.Emit<OpCode::Add>(rax, 0x12345678);

            //std::cout << std::endl;
            //std::cout << "Sub/Cmp" << std::endl;

            //buffer.Emit<OpCode::Sub>(rax, rbx);
            //buffer.Emit<OpCode::Cmp>(rax, r9);

            //std::cout << std::endl;
            //std::cout << "Lea" << std::endl;
            //buffer.Emit<OpCode::Lea>(rax, rbx, 0x12);

            //std::cout << std::endl;
            //std::cout << "Pop/Push" << std::endl;
            //buffer.Emit<OpCode::Pop>(rax);
            //buffer.Emit<OpCode::Pop>(r8);
            //buffer.Emit<OpCode::Push>(rax);
            //buffer.Emit<OpCode::Push>(r8);

            //std::cout << std::endl;
            //std::cout << "Ret" << std::endl;
            //buffer.Emit<OpCode::Ret>();
        }


        TestCase(OpCodes)
        {
            ExecutionBuffer allocator(5000);
            FunctionBufferBase buffer(allocator, 2000, 3, 0, false);


            // Mod/RM special cases for r12.
            std::cout << "Mod/RM special cases for r12." << std::endl;
            buffer.Emit<OpCode::Sub>(rbx, r12, 0);
            buffer.Emit<OpCode::Sub>(rdi, r12, 0x12);       // Found: 49 2B 5C 24 12 Expected: 49/ 2B 7C 24 12
            buffer.Emit<OpCode::Sub>(rbp, r12, 0x1234);     // Found: 49 2B 9C 24 34 12 00 00 Expected 49/ 2B AC 24 ...
            buffer.Emit<OpCode::Sub>(r10, r12, 0x12345678);


            // direct-direct
            std::cout << "direct-direct" << std::endl;
            buffer.Emit<OpCode::Add>(al, cl);
            buffer.Emit<OpCode::Add>(bx, dx);
            buffer.Emit<OpCode::Add>(esi, eax);
            buffer.Emit<OpCode::Add>(rax, rbx);
            buffer.Emit<OpCode::Add>(r8, r9);
            buffer.Emit<OpCode::Add>(rsp, r12);


            // direct-indirect with zero, byte, word, and double word offsets
            std::cout << "direct-indirect with zero, byte, word, and double word offsets" << std::endl;
            buffer.Emit<OpCode::Add>(cl, rax, 0);
            buffer.Emit<OpCode::Add>(bl, rcx, 0x12);
            buffer.Emit<OpCode::Add>(r9b, rsi, 0x100);
            buffer.Emit<OpCode::Add>(r15b, rdi, 0x12345678);

            buffer.Emit<OpCode::Cmp>(dl, rdx, 0);
            buffer.Emit<OpCode::Cmp>(cx, rcx, 0x12);
            buffer.Emit<OpCode::Cmp>(r9w, rsi, 0x1234);
            buffer.Emit<OpCode::Cmp>(r11w, rdi, 0x12345678);

            buffer.Emit<OpCode::Or>(esp, r9, 0);
            buffer.Emit<OpCode::Or>(edx, rcx, 0x12);
            buffer.Emit<OpCode::Or>(esi, rsi, 0x1234);
            buffer.Emit<OpCode::Or>(r11d, rdi, 0x12345678);

            buffer.Emit<OpCode::Sub>(rbx, r12, 0);
            buffer.Emit<OpCode::Sub>(rdi, rcx, 0x12);
            buffer.Emit<OpCode::Sub>(rbp, rsi, 0x1234);
            buffer.Emit<OpCode::Sub>(r10, rdi, 0x12345678);


            // direct-immediate register 0 case
            std::cout << "direct-immediate register 0 case" << std::endl;
            buffer.Emit<OpCode::Or>(al, 0x34);
            buffer.Emit<OpCode::Or>(ax, 0x56);
            buffer.Emit<OpCode::Or>(ax, 0x5678);
            buffer.Emit<OpCode::Or>(eax, 0x12);
            buffer.Emit<OpCode::Or>(eax, 0x1234);
            buffer.Emit<OpCode::Or>(eax, 0x12345678);
            buffer.Emit<OpCode::Or>(rax, 0x12);
            buffer.Emit<OpCode::Or>(rax, 0x1234);
            buffer.Emit<OpCode::Or>(rax, 0x12345678);


            // direct-immediate general purpose register case
            std::cout << "direct-immediate general purpose register case" << std::endl;
            buffer.Emit<OpCode::And>(bl, 0x34);
            buffer.Emit<OpCode::And>(cx, 0x56);
            buffer.Emit<OpCode::And>(dx, 0x5678);
            buffer.Emit<OpCode::And>(ebp, 0x12);
            buffer.Emit<OpCode::And>(ebp, 0x1234);
            buffer.Emit<OpCode::And>(ebp, 0x12345678);
            buffer.Emit<OpCode::And>(r12, 0x12);
            buffer.Emit<OpCode::And>(r12, 0x1234);
            buffer.Emit<OpCode::And>(r12, 0x12345678);
        }



        //*********************************************************************
        //
        // 1. Use ML64 assember to generate expected opcodes:
        //   "c:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\bin\x86_amd64\ml64.exe" /nologo /Sn /FlTestAsm.lst TestAsm.asm
        // 2. Open TestAsm.lst in Visual Studio.
        // 3. Select .code section (do not include .data section).
        // 4. Copy/paste below.
        // 5. Use ALT-select to add quotation marks un the left and a newline
        //    and quotation mark on the right.
        // 6. Note that leading spaces and tabs must be preserved in order for the
        //    verifier to work correctly.
        // 7. Add a semicolon at the end of the multiline string.
        char const * g_ml64Output =
            " 00000000  48/ 03 C3              add rax, rbx                \n"
            " 00000003  02 C3                  add al, bl                  \n"
            " 00000005  48/ 03 43 12           add rax, [rbx + 12h]        \n"
            " 00000009  48/ 03 83              add rax, [rbx + 12345678h]  \n"
            "           12345678                                           \n"
            " 00000010  04 12                  add al, 12h                 \n"
            " 00000012  48/ 83 C0 12           add rax, 12h                \n"
            " 00000016  48/ 05                 add rax, 12345678h          \n"
            "           12345678                                           \n";

        TestCase(FooBar)
        {
            ExecutionBuffer allocator(1000);
            FunctionBufferBase buffer(allocator, 200, 3, 0, false);

            std::cout << "Add" << std::endl;

            unsigned __int8 const * start =  buffer.BufferStart() + buffer.CurrentPosition();

            buffer.Emit<OpCode::Add>(rax, rbx);
            buffer.Emit<OpCode::Add>(al, bl);
            buffer.Emit<OpCode::Add>(rax, rbx, 0x12);
            buffer.Emit<OpCode::Add>(rax, rbx, 0x12345678);
            buffer.Emit<OpCode::Add>(al, static_cast<__int8>(0x12));
            buffer.Emit<OpCode::Add>(rax, 0x12);
            buffer.Emit<OpCode::Add>(rax, 0x12345678);

            ML64Verifier v(g_ml64Output, start);
        }
    }
}

