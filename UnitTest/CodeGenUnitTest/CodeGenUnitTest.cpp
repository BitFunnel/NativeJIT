#include "stdafx.h"

#include <cctype>               // isxdigit().
#include <iostream>             // TODO: Temporary - for debugging.
#include <vector>

#include "ML64Verifier.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "SuiteCpp/UnitTest.h"


namespace NativeJIT
{
    namespace CodeGenUnitTest
    {
        // TODO: JMP
        // TODO: JCC - all cases.

        TestCase(JCC)
        {
            ExecutionBuffer allocator(1000);
            FunctionBuffer buffer(allocator, 200, 10, 10, 3, 0, false);

            Label l1 = buffer.AllocateLabel();
            buffer.PlaceLabel(l1);
            buffer.Emit<JccType::JG>(l1);
        }


        TestCase(OpCodes)
        {
            ExecutionBuffer allocator(5000);
            FunctionBuffer buffer(allocator, 2000, 10, 10, 3, 0, false);

            unsigned __int8 const * start =  buffer.BufferStart() + buffer.CurrentPosition();

            // Another special case
            std::cout << "Another special case." << std::endl;
            buffer.Emit<OpCode::Add>(r13, r13, 0);
            buffer.Emit<OpCode::Mov>(r13, r13, 0);


            // Mod/RM special cases for RSP and R12 and [RBP] ==> [RBP + disp8]
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

            // mov r, r
            std::cout << "mov r, r" << std::endl;
            buffer.Emit<OpCode::Mov>(al, cl);
            buffer.Emit<OpCode::Mov>(bx, dx);
            buffer.Emit<OpCode::Mov>(esi, eax);
            buffer.Emit<OpCode::Mov>(rax, rbx);
            buffer.Emit<OpCode::Mov>(r8, r9);
            buffer.Emit<OpCode::Mov>(rsp, r12);


            // mov r, [r + offset]
            std::cout << "mov direct-indirect with zero, byte, word, and double word offsets" << std::endl;
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


            // mov r, imm
            std::cout << "mov r, imm" << std::endl;
            buffer.Emit<OpCode::Mov>(al, 0x34);
            buffer.Emit<OpCode::Mov>(ax, 0x56);
            buffer.Emit<OpCode::Mov>(ax, 0x5678);
            buffer.Emit<OpCode::Mov>(eax, 0x12);
            buffer.Emit<OpCode::Mov>(eax, 0x1234);
            buffer.Emit<OpCode::Mov>(eax, 0x12345678);
            buffer.Emit<OpCode::Mov>(rax, 0x12);
            buffer.Emit<OpCode::Mov>(rax, 0x1234);
            buffer.Emit<OpCode::Mov>(rax, 0x12345678);


            std::cout << "direct-immediate general purpose register case" << std::endl;
            buffer.Emit<OpCode::Mov>(bl, 0x34);
            buffer.Emit<OpCode::Mov>(cx, 0x56);
            buffer.Emit<OpCode::Mov>(dx, 0x5678);
            buffer.Emit<OpCode::Mov>(ebp, 0x12);
            buffer.Emit<OpCode::Mov>(ebp, 0x1234);
            buffer.Emit<OpCode::Mov>(ebp, 0x12345678);
            buffer.Emit<OpCode::Mov>(r12, 0x12);
            buffer.Emit<OpCode::Mov>(r12, 0x1234);
            buffer.Emit<OpCode::Mov>(r12, 0x12345678);
            buffer.Emit<OpCode::Mov>(rbx, 0x1234567812345678);
            buffer.Emit<OpCode::Mov>(rsp, 0x1234567812345678);
            buffer.Emit<OpCode::Mov>(r12, 0x1234567812345678);

            // mov [r + offset], r with zero, byte, word, and double word offsets
            std::cout << "mov [r + offset], r with zero, byte, word, and double word offsets" << std::endl;
            buffer.Emit<OpCode::Mov>(rax, 0, cl);
            buffer.Emit<OpCode::Mov>(rcx, 0x12, bl);
            buffer.Emit<OpCode::Mov>(rsi, 0x100, r9b);
            buffer.Emit<OpCode::Mov>(rdi, 0x12345678, r15b);

            buffer.Emit<OpCode::Mov>(rdx, 0, dl);
            buffer.Emit<OpCode::Mov>(rcx, 0x12, cx);
            buffer.Emit<OpCode::Mov>(rsi, 0x1234, r9w);
            buffer.Emit<OpCode::Mov>(rdi, 0x12345678, r11w);

            buffer.Emit<OpCode::Mov>(r9, 0, esp);
            buffer.Emit<OpCode::Mov>(rcx, 0x12, edx);
            buffer.Emit<OpCode::Mov>(rsi, 0x1234, esi);
            buffer.Emit<OpCode::Mov>(rdi, 0x12345678, r11d);

            buffer.Emit<OpCode::Mov>(r12, 0, rbx);
            buffer.Emit<OpCode::Mov>(rcx, 0x12, rdi);
            buffer.Emit<OpCode::Mov>(rsi, 0x1234, rbp);
            buffer.Emit<OpCode::Mov>(rdi, 0x12345678, r10);


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
            // jmp
            // call
            // imul
            std::cout << "imul" << std::endl;
            buffer.Emit<OpCode::IMul>(bx, cx);
            buffer.Emit<OpCode::IMul>(ebx, ecx);
            buffer.Emit<OpCode::IMul>(rbx, rcx);

            buffer.Emit<OpCode::IMul>(cx, rcx, 0x12);
            buffer.Emit<OpCode::IMul>(r9w, rsi, 0x1234);
            buffer.Emit<OpCode::IMul>(r11w, rdi, 0x12345678);

            buffer.Emit<OpCode::IMul>(esp, r9, 0);
            buffer.Emit<OpCode::IMul>(edx, rcx, 0x12);
            buffer.Emit<OpCode::IMul>(esi, rsi, 0x1234);
            buffer.Emit<OpCode::IMul>(r11d, rdi, 0x12345678);

            buffer.Emit<OpCode::IMul>(rbx, r12, 0);
            buffer.Emit<OpCode::IMul>(rdi, rcx, 0x12);
            buffer.Emit<OpCode::IMul>(rbp, rsi, 0x1234);
            buffer.Emit<OpCode::IMul>(r10, rdi, 0x12345678);

            buffer.Emit<OpCode::IMul>(cx, 0x56);
            buffer.Emit<OpCode::IMul>(dx, 0x5678);
            buffer.Emit<OpCode::IMul>(ebp, 0x12);
            buffer.Emit<OpCode::IMul>(ebp, 0x1234);
            buffer.Emit<OpCode::IMul>(ebp, 0x12345678);
            buffer.Emit<OpCode::IMul>(r12, 0x12);
            buffer.Emit<OpCode::IMul>(r12, 0x1234);
            buffer.Emit<OpCode::IMul>(r12, 0x12345678);

            // Call
            buffer.Emit<OpCode::Call>(rax);
            buffer.Emit<OpCode::Call>(rsp);
            buffer.Emit<OpCode::Call>(rbp);
            buffer.Emit<OpCode::Call>(r12);
            buffer.Emit<OpCode::Call>(r13);

            // MovD
            buffer.Emit<OpCode::Mov>(xmm1, rax);
            buffer.Emit<OpCode::Mov>(xmm1, rcx);
            buffer.Emit<OpCode::Mov>(xmm1, r8);
            buffer.Emit<OpCode::Mov>(xmm1, rbp);
            buffer.Emit<OpCode::Mov>(xmm1, r12);

            buffer.Emit<OpCode::Mov>(xmm0, rcx);
            buffer.Emit<OpCode::Mov>(xmm1, rcx);
            buffer.Emit<OpCode::Mov>(xmm2, rcx);
            buffer.Emit<OpCode::Mov>(xmm5, rcx);
            buffer.Emit<OpCode::Mov>(xmm12, rcx);

            // MovSD
            buffer.Emit<OpCode::Mov>(xmm1, xmm2);
            buffer.Emit<OpCode::Mov>(xmm0, xmm12);
            buffer.Emit<OpCode::Mov>(xmm5, xmm12);
            buffer.Emit<OpCode::Mov>(xmm5, xmm3);
            buffer.Emit<OpCode::Mov>(xmm13, xmm5);
            buffer.Emit<OpCode::Mov>(xmm0, xmm15);


            buffer.Emit<OpCode::Mov>(xmm0, r12, 0);
            buffer.Emit<OpCode::Mov>(xmm4, rcx, 0x12);
            buffer.Emit<OpCode::Mov>(xmm5, rsi, 0x1234);
            buffer.Emit<OpCode::Mov>(xmm12, rdi, 0x12345678);


            buffer.Emit<OpCode::Mov>(r12, 0, xmm0);
            buffer.Emit<OpCode::Mov>(rcx, 0x12, xmm4);
            buffer.Emit<OpCode::Mov>(rsi, 0x1234, xmm5);
            buffer.Emit<OpCode::Mov>(rdi, 0x12345678, xmm12);


            buffer.Emit<OpCode::Add>(xmm1, xmm2);
            buffer.Emit<OpCode::Add>(xmm0, xmm12);
            buffer.Emit<OpCode::IMul>(xmm5, xmm12);
            buffer.Emit<OpCode::IMul>(xmm5, xmm3);
            buffer.Emit<OpCode::Sub>(xmm13, xmm5);
            buffer.Emit<OpCode::Sub>(xmm0, xmm15);

            buffer.Emit<OpCode::Add>(xmm0, r12, 0);
            buffer.Emit<OpCode::Add>(xmm4, rcx, 0x12);
            buffer.Emit<OpCode::IMul>(xmm5, rsi, 0x1234);
            buffer.Emit<OpCode::Sub>(xmm12, rdi, 0x12345678);

            // shift

            // floating point
            // signed

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
                "                                ; Another special case                                             \n"
                " 00000000  4D/ 03 6D 00         add r13, [r13]                                                     \n"
                " 00000000  4D/ 8B 6D 00         mov r13, [r13]                                                     \n"
                "                                                                                                   \n"
                "                                ; Mod/RM special cases for RSP and R12 and [RBP] ==> [RBP + disp8] \n"
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
                // TODO: need to look at r8b, r9w, r10d
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
                "                               ; Mov r, imm - register 0 case                                      \n"
                " 00000173  B0 34                mov al, 34h                                                        \n"
                " 00000175  66| B8 0056          mov ax, 56h                                                        \n"
                " 00000179  66| B8 5678          mov ax, 5678h                                                      \n"
                " 0000017D  B8 00000012          mov eax, 12h                                                       \n"
                " 00000182  B8 00001234          mov eax, 1234h                                                     \n"
                " 00000187  B8 12345678          mov eax, 12345678h                                                 \n"
                " 0000018C  48/ C7 C0            mov rax, 12h                                                       \n"
                "           00000012                                                                                \n"
                " 00000193  48/ C7 C0            mov rax, 1234h                                                     \n"
                "           00001234                                                                                \n"
                " 0000019A  48/ C7 C0            mov rax, 12345678h                                                 \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                "                                ; Mov r, imm - general purpose register case                       \n"
                " 000001A1  B3 34                mov bl, 34h                                                        \n"
                " 000001A3  66| B9 0056          mov cx, 56h                                                        \n"
                " 000001A7  66| BA 5678          mov dx, 5678h                                                      \n"
                " 000001AB  BD 00000012          mov ebp, 12h                                                       \n"
                " 000001B0  BD 00001234          mov ebp, 1234h                                                     \n"
                " 000001B5  BD 12345678          mov ebp, 12345678h                                                 \n"
                " 000001BA  49/ C7 C4            mov r12, 12h                                                       \n"
                "           00000012                                                                                \n"
                " 000001C1  49/ C7 C4            mov r12, 1234h                                                     \n"
                "           00001234                                                                                \n"
                " 000001C8  49/ C7 C4            mov r12, 12345678h                                                 \n"
                "           12345678                                                                                \n"
                " 000001CF  48/ BB               mov rbx, 1234567812345678h                                         \n"
                "           1234567812345678                                                                        \n"
                " 000001D9  48/ BC               mov rsp, 1234567812345678h                                         \n"
                "           1234567812345678                                                                        \n"
                " 000001CF  49/ BC               mov r12, 1234567812345678h                                         \n"
                "           1234567812345678                                                                        \n"
                "                                                                                                   \n"
                "                                                                                                   \n"
                "                                ; mov [r + offset], r with zero, byte, word, and dword offsets     \n"
                " 000001ED  88 08                mov [rax], cl                                                      \n"
                " 000001EF  88 59 12             mov [rcx + 12h], bl                                                \n"
                " 000001F2  44/ 88 8E            mov [rsi + 100h], r9b                                              \n"
                "           00000100                                                                                \n"
                " 000001F9  44/ 88 BF            mov [rdi + 12345678h], r15b                                        \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                " 00000200  88 12                mov [rdx], dl                                                      \n"
                " 00000202  66| 89 49 12         mov [rcx + 12h], cx                                                \n"
                " 00000206  66| 44/ 89 8E        mov [rsi + 1234h], r9w                                             \n"
                "           00001234                                                                                \n"
                " 0000020E  66| 44/ 89 9F        mov [rdi + 12345678h], r11w                                        \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                " 00000216  41/ 89 21            mov [r9], esp                                                      \n"
                " 00000219  89 51 12             mov [rcx + 12h], edx                                               \n"
                " 0000021C  89 B6 00001234       mov [rsi + 1234h], esi                                             \n"
                " 00000222  44/ 89 9F            mov [rdi + 12345678h], r11d                                        \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                " 00000229  49/ 89 1C 24         mov [r12], rbx                                                     \n"
                " 0000022D  48/ 89 79 12         mov [rcx + 12h], rdi                                               \n"
                " 00000231  48/ 89 AE            mov [rsi + 1234h], rbp                                             \n"
                "           00001234                                                                                \n"
                " 00000238  4C/ 89 97            mov [rdi + 12345678h], r10                                         \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                "                                                                                                   \n"
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
              
                " 00000110  C3                   ret                                                                \n"
                "                                                                                                   \n"
                "                                ; IMul                                                             \n"
                " 00000258  66| 0F AF D9           imul bx, cx                                                      \n"
                " 0000025C  0F AF D9               imul ebx, ecx                                                    \n"
                " 0000025F  48/ 0F AF D9           imul rbx, rcx                                                    \n"
                "                                                                                                   \n"
                "                                                                                                   \n"
                " 0000026F  66| 0F AF 49         imul cx, [rcx + 12h]                                               \n"
                "           12                                                                                      \n"
                " 00000274  66| 44/ 0F AF 8E     imul r9w, [rsi + 1234h]                                            \n"
                "           00001234                                                                                \n"
                " 0000027D  66| 44/ 0F AF 9F     imul r11w, [rdi + 12345678h]                                       \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                " 00000286  41/ 0F AF 21         imul esp, [r9]                                                     \n"
                " 0000028A  0F AF 51 12          imul edx, [rcx + 12h]                                              \n"
                " 0000028E  0F AF B6             imul esi, [rsi + 1234h]                                            \n"
                "           00001234                                                                                \n"
                " 00000295  44/ 0F AF 9F         imul r11d, [rdi + 12345678h]                                       \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                " 0000029D  49/ 0F AF 1C 24      imul rbx, [r12]                                                    \n"
                " 000002A2  48/ 0F AF 79         imul rdi, [rcx + 12h]                                              \n"
                "           12                                                                                      \n"
                " 000002A7  48/ 0F AF AE         imul rbp, [rsi + 1234h]                                            \n"
                "           00001234                                                                                \n"
                " 000002AF  4C/ 0F AF 97         imul r10, [rdi + 12345678h]                                        \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                "                                                                                                   \n"
                " 000002B7  66| 6B C9 56         imul cx, 56h                                                       \n"
                " 000002BB  66| 69 D2 5678       imul dx, 5678h                                                     \n"
                " 000002C0  6B ED 12             imul ebp, 12h                                                      \n"
                " 000002C3  69 ED 00001234       imul ebp, 1234h                                                    \n"
                " 000002C9  69 ED 12345678       imul ebp, 12345678h                                                \n"
                " 000002CF  4D/ 6B E4 12         imul r12, 12h                                                      \n"
                " 000002D3  4D/ 69 E4            imul r12, 1234h                                                    \n"
                "           00001234                                                                                \n"
                " 000002DA  4D/ 69 E4            imul r12, 12345678h                                                \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                "                                ; Call                                                             \n"
                " 000002E9  FF D0                call rax                                                           \n"
                " 000002EB  FF D4                call rsp                                                           \n"
                " 000002ED  FF D5                call rbp                                                           \n"
                " 000002EF  41/ FF D4            call r12                                                           \n"
                " 000002F2  41/ FF D5            call r13                                                           \n"
                "                                                                                                   \n"
                "                                ; MovD                                                             \n"
                " 0000030D  66| 48/ 0F 6E C8     movd xmm1, rax                                                     \n"
                " 00000312  66| 48/ 0F 6E C9     movd xmm1, rcx                                                     \n"
                " 00000317  66| 49/ 0F 6E C8     movd xmm1, r8                                                      \n"
                " 0000031C  66| 48/ 0F 6E CD     movd xmm1, rbp                                                     \n"
                " 00000321  66| 49/ 0F 6E CC     movd xmm1, r12                                                     \n"
                "                                                                                                   \n"
                " 00000326  66| 48/ 0F 6E C1     movd xmm0, rcx                                                     \n"
                " 0000032B  66| 48/ 0F 6E C9     movd xmm1, rcx                                                     \n"
                " 00000330  66| 48/ 0F 6E D1     movd xmm2, rcx                                                     \n"
                " 00000335  66| 48/ 0F 6E E9     movd xmm5, rcx                                                     \n"
                " 0000033A  66| 4C/ 0F 6E E1     movd xmm12, rcx                                                    \n"
                "                                                                                                   \n"
                "                                ; MovSd                                                            \n"
                " 00000343  F2/ 0F 10 CA         movsd xmm1, xmm2                                                   \n"
                " 00000347  F2/ 41/ 0F 10 C4     movsd xmm0, xmm12                                                  \n"
                " 0000034C  F2/ 41/ 0F 10 EC     movsd xmm5, xmm12                                                  \n"
                " 00000351  F2/ 0F 10 EB         movsd xmm5, xmm3                                                   \n"
                " 00000355  F2/ 44/ 0F 10 ED     movsd xmm13, xmm5                                                  \n"
                " 0000035A  F2/ 41/ 0F 10 C7     movsd xmm0, xmm15                                                  \n"
                "                                                                                                   \n"
                " 0000035F  F2/ 41/ 0F 10 04     movsd xmm0, mmword ptr [r12]                                       \n"
                "           24                                                                                      \n"
                " 00000365  F2/ 0F 10 61         movsd xmm4, mmword ptr [rcx + 12h]                                 \n"
                "           12                                                                                      \n"
                " 0000036A  F2/ 0F 10 AE         movsd xmm5, mmword ptr [rsi + 1234h]                               \n"
                "           00001234                                                                                \n"
                " 00000372  F2/ 44/ 0F 10 A7     movsd xmm12, mmword ptr [rdi + 12345678h]                          \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                " 0000037B  F2/ 41/ 0F 11 04     movsd mmword ptr [r12], xmm0                                       \n"
                "           24                                                                                      \n"
                " 00000381  F2/ 0F 11 61         movsd mmword ptr [rcx + 12h], xmm4                                 \n"
                "           12                                                                                      \n"
                " 00000386  F2/ 0F 11 AE         movsd mmword ptr [rsi + 1234h], xmm5                               \n"
                "           00001234                                                                                \n"
                " 0000038E  F2/ 44/ 0F 11 A7     movsd mmword ptr [rdi + 12345678h], xmm12                          \n"
                "           12345678                                                                                \n"
                "                                                                                                   \n"
                " 00000397  F2/ 0F 58 CA         addsd xmm1, xmm2                                                   \n"
                " 0000039B  F2/ 41/ 0F 58 C4     addsd xmm0, xmm12                                                  \n"
                " 000003A0  F2/ 41/ 0F 59 EC     mulsd xmm5, xmm12                                                  \n"
                " 000003A5  F2/ 0F 59 EB         mulsd xmm5, xmm3                                                   \n"
                " 000003A9  F2/ 44/ 0F 5C ED     subsd xmm13, xmm5                                                  \n"
                " 000003AE  F2/ 41/ 0F 5C C7     subsd xmm0, xmm15                                                  \n"
                "                                                                                                   \n"
                " 000003B3  F2/ 41/ 0F 58 04     addsd xmm0, mmword ptr [r12]                                       \n"
                "           24                                                                                      \n"
                " 000003B9  F2/ 0F 58 61         addsd xmm4, mmword ptr [rcx + 12h]                                 \n"
                "           12                                                                                      \n"
                " 000003BE  F2/ 0F 59 AE         mulsd xmm5, mmword ptr [rsi + 1234h]                               \n"
                "           00001234                                                                                \n"
                " 000003C6  F2/ 44/ 0F 5C A7     subsd xmm12, mmword ptr [rdi + 12345678h]                          \n"
                "           12345678                                                                                \n";

            ML64Verifier v(ml64Output, start);
        }
    }
}

