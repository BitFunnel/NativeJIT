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

            unsigned __int8 const * start =  buffer.BufferStart() + buffer.CurrentPosition();

            // Mod/RM special cases for r12.
            std::cout << "Mod/RM special cases for r12." << std::endl;
            buffer.Emit<OpCode::Sub>(rbx, r12, 0);
            buffer.Emit<OpCode::Sub>(rdi, r12, 0x12);
            buffer.Emit<OpCode::Sub>(rbp, r12, 0x1234);
            buffer.Emit<OpCode::Sub>(r10, r12, 0x12345678);

            // Mod/RM special cases for rsp.
            // TODO

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

            // call

            // lea
            std::cout << "lea" << std::endl;
            buffer.Emit<OpCode::Lea>(rax, rsi, 0);
            buffer.Emit<OpCode::Lea>(rax, rsi, 0x12);
            buffer.Emit<OpCode::Lea>(rax, rsi, 0x1234);
            buffer.Emit<OpCode::Lea>(rax, rsi, 0x12345678);
            buffer.Emit<OpCode::Lea>(rbp, r12, 0);
            buffer.Emit<OpCode::Lea>(rbp, r12, 0x87);
            buffer.Emit<OpCode::Lea>(rbp, r12, 0x87654321);
            buffer.Emit<OpCode::Lea>(rsp, rbp, 0xFFFFFFE0);     // From function epilogue.
            buffer.Emit<OpCode::Lea>(rbp, rsp, 0x20);           // From function prologue.

            // mov
            std::cout << "mov" << std::endl;
            buffer.Emit<OpCode::Mov>(al, cl);
            buffer.Emit<OpCode::Mov>(bx, dx);
            buffer.Emit<OpCode::Mov>(esi, eax);
            buffer.Emit<OpCode::Mov>(rax, rbx);
            buffer.Emit<OpCode::Mov>(r8, r9);
            buffer.Emit<OpCode::Mov>(rsp, r12);
            // TODO: mov immdiate
            // TODO: mov r/m reg

            // direct-indirect with zero, byte, word, and double word offsets
            std::cout << "direct-indirect with zero, byte, word, and double word offsets" << std::endl;
            buffer.Emit<OpCode::Mov>(cl, rax, 0);
            buffer.Emit<OpCode::Mov>(bl, rcx, 0x12);
            buffer.Emit<OpCode::Mov>(r9b, rsi, 0x100);
            buffer.Emit<OpCode::Mov>(r15b, rdi, 0x12345678);

            buffer.Emit<OpCode::Mov>(dl, rdx, 0);
            buffer.Emit<OpCode::Mov>(cx, rcx, 0x12);
            buffer.Emit<OpCode::Mov>(r9w, rsi, 0x1234);
            buffer.Emit<OpCode::Mov>(r11w, rdi, 0x12345678);

            buffer.Emit<OpCode::Mov>(esp, r9, 0);
            buffer.Emit<OpCode::Mov>(edx, rcx, 0x12);
            buffer.Emit<OpCode::Mov>(esi, rsi, 0x1234);
            buffer.Emit<OpCode::Mov>(r11d, rdi, 0x12345678);

            buffer.Emit<OpCode::Mov>(rbx, r12, 0);
            buffer.Emit<OpCode::Mov>(rdi, rcx, 0x12);
            buffer.Emit<OpCode::Mov>(rbp, rsi, 0x1234);
            buffer.Emit<OpCode::Mov>(r10, rdi, 0x12345678);

            // pop/push
            std::cout << "pop/push" << std::endl;
            buffer.Emit<OpCode::Pop>(rax);
            buffer.Emit<OpCode::Pop>(rbp);
            buffer.Emit<OpCode::Pop>(r12);
            buffer.Emit<OpCode::Push>(rbx);
            buffer.Emit<OpCode::Push>(rbp);
            buffer.Emit<OpCode::Push>(r12);

            // ret
            buffer.Emit<OpCode::Ret>();

            // nop
            // jcc
            //   each opcode
            //   correct offsets
            // mov
            // imul

            // shift

            // floating point

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
            //
            // Better approach:
            // 1. In cmd window, "type TestAsm.lst"
            // 2. Copy/paste from cmd window. This converts tabs to spaces correctly.
            //
            //*********************************************************************
            char const * ml64Output = 
                " 00000000  49/ 2B 1C 24         sub rbx, [r12]                                                     \n"
                " 00000004  49/ 2B 7C 24         sub rdi, [r12 + 12h]                                               \n"
                "           12                                                                                      \n"
                " 00000009  49/ 2B AC 24         sub rbp, [r12 + 1234h]                                             \n"
                "           00001234                                                                                \n"
                " 00000011  4D/ 2B 94 24         sub r10, [r12 + 12345678h]                                         \n"
                "           12345678                                                                                \n"
                " 00000019  02 C1                add al, cl                                                         \n"
                " 0000001B  66| 03 DA            add bx, dx                                                         \n"
                " 0000001E  03 F0                add esi, eax                                                       \n"
                " 00000020  48/ 03 C3            add rax, rbx                                                       \n"
                " 00000023  4D/ 03 C1            add r8, r9                                                         \n"
                " 00000026  49/ 03 E4            add rsp, r12                                                       \n"
                "                                                                                                   \n"
                "                                ; direct-indirect with zero, byte, word, and double word offsets   \n"
                " 00000029  02 08                add cl, [rax]                                                      \n"
                " 0000002B  02 59 12             add bl, [rcx + 12h]                                                \n"
                " 0000002E  44/ 02 8E            add r9b, [rsi + 100h]                                              \n"
                "           00000100                                                                                \n"
                " 00000035  44/ 02 BF            add r15b, [rdi + 12345678h]                                        \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                " 0000003C  3A 12                cmp dl, [rdx]                                                      \n"
                " 0000003E  66| 3B 49 12         cmp cx, [rcx + 12h]                                                \n"
                " 00000042  66| 44/ 3B 8E        cmp r9w, [rsi + 1234h]                                             \n"
                "           00001234                                                                                \n"
                " 0000004A  66| 44/ 3B 9F        cmp r11w, [rdi + 12345678h]                                        \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                " 00000052  41/ 0B 21            or esp, [r9]                                                       \n"
                " 00000055  0B 51 12             or edx, [rcx + 12h]                                                \n"
                " 00000058  0B B6 00001234       or esi, [rsi + 1234h]                                              \n"
                " 0000005E  44/ 0B 9F            or r11d, [rdi + 12345678h]                                         \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                " 00000065  49/ 2B 1C 24         sub rbx, [r12]                                                     \n"
                " 00000069  48/ 2B 79 12         sub rdi, [rcx + 12h]                                               \n"
                " 0000006D  48/ 2B AE            sub rbp, [rsi + 1234h]                                             \n"
                "           00001234                                                                                \n"
                " 00000074  4C/ 2B 97            sub r10, [rdi + 12345678h]                                         \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                "                                ; direct-immediate register 0 case                                 \n"
                " 0000007B  0C 34                or al, 34h                                                         \n"
                " 0000007D  66| 83 C8 56         or ax, 56h                                                         \n"
                " 00000081  66| 0D 5678          or ax, 5678h                                                       \n"
                " 00000085  83 C8 12             or eax, 12h                                                        \n"
                " 00000088  0D 00001234          or eax, 1234h                                                      \n"
                " 0000008D  0D 12345678          or eax, 12345678h                                                  \n"
                " 00000092  48/ 83 C8 12         or rax, 12h                                                        \n"
                " 00000096  48/ 0D               or rax, 1234h                                                      \n"
                "           00001234                                                                                \n"
                " 0000009C  48/ 0D               or rax, 12345678h                                                  \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                "                                ; direct-immediate general purpose register case                   \n"
                " 000000A2  80 E3 34             and bl, 34h                                                        \n"
                " 000000A5  66| 83 E1 56         and cx, 56h                                                        \n"
                " 000000A9  66| 81 E2 5678       and dx, 5678h                                                      \n"
                " 000000AE  83 E5 12             and ebp, 12h                                                       \n"
                " 000000B1  81 E5 00001234       and ebp, 1234h                                                     \n"
                " 000000B7  81 E5 12345678       and ebp, 12345678h                                                 \n"
                " 000000BD  49/ 83 E4 12         and r12, 12h                                                       \n"
                " 000000C1  49/ 81 E4            and r12, 1234h                                                     \n"
                "           00001234                                                                                \n"
                " 000000C8  49/ 81 E4            and r12, 12345678h                                                 \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                "                                ; lea                                                              \n"
                " 000000DF  48/ 8D 06            lea rax, [rsi]                                                     \n"
                " 000000E2  48/ 8D 46 12         lea rax, [rsi + 12h]                                               \n"
                " 000000E6  48/ 8D 86            lea rax, [rsi + 1234h]                                             \n"
                "           00001234                                                                                \n"
                " 000000ED  48/ 8D 86            lea rax, [rsi + 12345678h]                                         \n"
                "           12345678                                                                                \n"
                " 000000F4  49/ 8D 2C 24         lea rbp, [r12]                                                     \n"
                " 000000F8  49/ 8D AC 24         lea rbp, [r12 + 87h]                                               \n"
                "           00000087                                                                                \n"
                " 00000100  49/ 8D AC 24         lea rbp, [r12 + 87654321h]                                         \n"
                "           87654321                                                                                \n"
                " 00000108  48/ 8D 65 E0         lea rsp, [rbp + 0FFFFFFE0h]                                        \n"
                " 0000010C  48/ 8D 6C 24         lea rbp, [rsp + 20h]                                               \n"
                "           20                                                                                      \n"
                "                                                                                                   \n"
                "                                                                                                   \n"
                "                                ; Mov                                                              \n"
                " 00000111  8A C1                mov al, cl                                                         \n"
                " 00000113  66| 8B DA            mov bx, dx                                                         \n"
                " 00000116  8B F0                mov esi, eax                                                       \n"
                " 00000118  48/ 8B C3            mov rax, rbx                                                       \n"
                " 0000011B  4D/ 8B C1            mov r8, r9                                                         \n"
                " 0000011E  49/ 8B E4            mov rsp, r12                                                       \n"
                "                                                                                                   \n"
                " 00000121  8A 08                mov cl, [rax]                                                      \n"
                " 00000123  8A 59 12             mov bl, [rcx + 12h]                                                \n"
                " 00000126  44/ 8A 8E            mov r9b, [rsi + 100h]                                              \n"
                "           00000100                                                                                \n"
                " 0000012D  44/ 8A BF            mov r15b, [rdi + 12345678h]                                        \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                " 00000134  8A 12                mov dl, [rdx]                                                      \n"
                " 00000136  66| 8B 49 12         mov cx, [rcx + 12h]                                                \n"
                " 0000013A  66| 44/ 8B 8E        mov r9w, [rsi + 1234h]                                             \n"
                "           00001234                                                                                \n"
                " 00000142  66| 44/ 8B 9F        mov r11w, [rdi + 12345678h]                                        \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                " 0000014A  41/ 8B 21            mov esp, [r9]                                                      \n"
                " 0000014D  8B 51 12             mov edx, [rcx + 12h]                                               \n"
                " 00000150  8B B6 00001234       mov esi, [rsi + 1234h]                                             \n"
                " 00000156  44/ 8B 9F            mov r11d, [rdi + 12345678h]                                        \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                " 0000015D  49/ 8B 1C 24         mov rbx, [r12]                                                     \n"
                " 00000161  48/ 8B 79 12         mov rdi, [rcx + 12h]                                               \n"
                " 00000165  48/ 8B AE            mov rbp, [rsi + 1234h]                                             \n"
                "           00001234                                                                                \n"
                " 0000016C  4C/ 8B 97            mov r10, [rdi + 12345678h]                                         \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                "                                ; pop/push                                                         \n"
                " 00000108  58                   pop rax                                                            \n"
                " 00000109  5D                   pop rbp                                                            \n"
                " 0000010A  41/ 5C               pop r12                                                            \n"
                " 0000010C  53                   push rbx                                                           \n"
                " 0000010D  55                   push rbp                                                           \n"
                " 0000010E  41/ 54               push r12                                                           \n"
                "                                                                                                   \n"
                "                                ; ret                                                              \n"
                " 00000110  C3                   ret                                                                \n";

            ML64Verifier v(ml64Output, start);
        }
    }
}

