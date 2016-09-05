// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include <algorithm>
#include <iostream>
#include <functional>
#include <random>

#include "NativeJIT/CodeGen/CallingConvention.h"
#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/CodeGen/FunctionSpecification.h"
#include "Temporary/Allocator.h"
#include "TestSetup.h"
#include "UnwindCode.h"

// TODO: Use alignas with VC14.
#ifdef _MSC_VER
#define ALIGNAS(x) __declspec(align(x))
#else
#define ALIGNAS(x) alignas(x)
#endif

// Using a macro to keep original source line information in failure text.
// Comparing m_frameOffset as it's a single variable which includes all of
// UnwindCode's bits.
#define ASSERT_EQ_UNWIND_CODE(expected, actual) \
    ASSERT_EQ(expected.m_frameOffset, \
              actual.m_frameOffset) \
    << "UnwindCode difference: (" \
    << expected.m_operation.m_codeOffset \
    << ", " \
    << expected.m_operation.m_unwindOp \
    << ", " \
    << expected.m_operation.m_opInfo \
    << ") vs (" \
    << actual.m_operation.m_codeOffset \
    << ", " \
    << actual.m_operation.m_unwindOp \
    << ", " \
    << actual.m_operation.m_opInfo \
    << ")";

// The two-code version.
#define ASSERT_EQ_UNWIND_CODE2(expected1, expected2, actual1, actual2) \
    ASSERT_TRUE(expected1.m_frameOffset == actual1.m_frameOffset \
                && expected2.m_frameOffset == actual2.m_frameOffset) \
    << "UnwindCode difference: (" \
    << expected1.m_operation.m_codeOffset \
    << ", " \
    << expected1.m_operation.m_unwindOp \
    << ", " \
    << expected1.m_operation.m_opInfo \
    << ", " \
    << expected2.m_frameOffset \
    << ") vs (" \
    << actual1.m_operation.m_codeOffset \
    << ", " \
    << actual1.m_operation.m_unwindOp \
    << ", " \
    << actual1.m_operation.m_opInfo \
    << ", " \
    << actual2.m_frameOffset \
    << ")";


namespace NativeJIT
{
    namespace CodeGenUnitTest
    {
        TEST_FIXTURE_START(FunctionBufferTest)

        protected:
            void ValidateUnwindInfo(FunctionSpecification const & spec)
            {
                auto & unwindInfo = *reinterpret_cast<UnwindInfo const *>(spec.GetUnwindInfoBuffer());
                const unsigned unwindByteLen = spec.GetUnwindInfoByteLength();

                ASSERT_TRUE(unwindByteLen >= sizeof(UnwindInfo)) << "Invalid UnwindInfo length " << unwindByteLen;

                ASSERT_EQ(1, unwindInfo.m_version);
                ASSERT_EQ(0, unwindInfo.m_flags);
                ASSERT_EQ(0, unwindInfo.m_frameRegister);
                ASSERT_EQ(0, unwindInfo.m_frameOffset);

                // Verify consistency of unwind codes count. One UnwindCode is
                // already included inside UnwindInfo structure.
                const unsigned baseUnwindInfoSize = sizeof(UnwindInfo) - sizeof(UnwindCode);
                const unsigned occupiedUnwindInfoBytes = baseUnwindInfoSize
                    + unwindInfo.m_countOfCodes * sizeof(UnwindCode);

                // Compare the expected and actual size of unwind data, accounting
                // for potential additional UnwindCode for alignment.
                ASSERT_TRUE(occupiedUnwindInfoBytes == unwindByteLen
                           || occupiedUnwindInfoBytes + sizeof(UnwindCode) == unwindByteLen);
            }


            void FillAllWritableRegistersWithGarbage(FunctionBuffer& code)
            {
                // Erase all writable registers (except RSP).
                unsigned regMask = CallingConvention::c_rxxWritableRegistersMask
                                   & ~rsp.GetMask();
                unsigned regId;

                // Using RAX explicitly in a few places below.
                ASSERT_TRUE((regMask & rax.GetMask()) != 0)
                    << "This test assumes RAX is writable";

                while (BitOp::GetLowestBitSet(regMask, &regId))
                {
                    code.EmitImmediate<OpCode::Mov>(Register<8, false>(regId),
                                                    m_uniformUInt64(m_rng));
                    BitOp::ClearBit(&regMask, regId);
                }

                // Do the same for XMM registers.
                regMask = CallingConvention::c_xmmWritableRegistersMask;

                while (BitOp::GetLowestBitSet(regMask, &regId))
                {
                    auto someRxxRegister = Register<8, false>(regId % (RegisterBase::c_maxIntegerRegisterID + 1));

                    code.Emit<OpCode::Mov>(Register<8, true>(regId), someRxxRegister);
                    BitOp::ClearBit(&regMask, regId);
                }
            }


            // Important: ihis structure must be 128-bit aligned to be able to use its
            // m_xmm members as targets for movaps.
            struct RegInfo
            {
                // Need to save all 128 bits.
                ALIGNAS(16) uint64_t m_xmm[2 * (RegisterBase::c_maxFloatRegisterID + 1)];

                // 64 bits for RXX registers.
                uint64_t m_rxx[RegisterBase::c_maxIntegerRegisterID + 1];
            };


            // Emits the code to save all nonvolatiles into the regInfo structure
            // in memory and returns the pointer to the start of that code.
            // No unwind information is necessary since the function doesn't
            // touch the stack/call any other functions.
            void (*EmitSaveNonVolatilesCode(FunctionBuffer & code,
                                            RegInfo const & regInfo))()
            {
                auto functionStart = reinterpret_cast<void (*)()>(code.BufferStart()
                                                                  + code.CurrentPosition());

                // Save all nonvolatiles.
                unsigned regMask = c_rxxWritableNonVolatilesMask;
                unsigned regId;

                // Using RAX as a scratch register.
                LogThrowAssert((regMask & rax.GetMask()) == 0, "This test assumes RAX is volatile");

                while (BitOp::GetLowestBitSet(regMask, &regId))
                {
                    // Load the target address into RAX and store the data.
                    code.EmitImmediate<OpCode::Mov>(rax, &regInfo.m_rxx[regId]);
                    code.Emit<OpCode::Mov>(rax, 0, Register<8, false>(regId));
                    BitOp::ClearBit(&regMask, regId);
                }

                // Do the same for XMM registers.
                regMask = c_xmmWritableNonVolatilesMask;

                while (BitOp::GetLowestBitSet(regMask, &regId))
                {
                    // Load the target address into RAX and store the data.
                    code.EmitImmediate<OpCode::Mov>(rax, &regInfo.m_xmm[2 * regId]);
                    code.Emit<OpCode::MovAP>(rax, 0, Register<4, true>(regId));
                    BitOp::ClearBit(&regMask, regId);
                }

                code.Emit<OpCode::Ret>();

                return functionStart;
            }


            // Emits the code into the buffer and records the buffer offset after
            // the emit.
            void EmitAndRecordOffset(FunctionBuffer& code,
                                     std::function<void(FunctionBuffer&)> const & emitter,
                                     std::vector<uint8_t>& offsets)
            {
                emitter(code);

                const unsigned offset = code.CurrentPosition();
                ASSERT_TRUE(offset < (std::numeric_limits<uint8_t>::max)());

                offsets.push_back(static_cast<uint8_t>(offset));
            }


            // Compares the prolog in FunctionSpecification against the one
            // in the function buffer. The function buffer is expected to contain
            // the code from offset 0.
            void VerifyProlog(FunctionSpecification const & spec,
                              FunctionBuffer const & expectedProlog)
            {
                ASSERT_EQ(expectedProlog.CurrentPosition(), spec.GetPrologLength());
                ASSERT_EQ(0,
                          memcmp(expectedProlog.BufferStart(),
                                 spec.GetProlog(),
                                 spec.GetPrologLength()));
            }


            // Same as above, for epilog.
            void VerifyEpilog(FunctionSpecification const & spec,
                              FunctionBuffer const & expectedEpilog)
            {
                ASSERT_EQ(expectedEpilog.CurrentPosition(), spec.GetEpilogLength());
                ASSERT_EQ(0,
                          memcmp(expectedEpilog.BufferStart(),
                                 spec.GetEpilog(),
                                 spec.GetEpilogLength()));
            }


            static const unsigned c_rxxWritableNonVolatilesMask
                = CallingConvention::c_rxxNonVolatileRegistersMask
                  & CallingConvention::c_rxxWritableRegistersMask;
            static const unsigned c_xmmWritableNonVolatilesMask
                = CallingConvention::c_xmmNonVolatileRegistersMask
                  & CallingConvention::c_xmmWritableRegistersMask;

            // Random number generator. Note: any default seed is acceptable, even if constant.
            std::default_random_engine m_rng;
            std::uniform_int_distribution<uint64_t> m_uniformUInt64;

        TEST_FIXTURE_END_TEST_CASES_BEGIN


        TEST_F(FunctionBufferTest, Trivial)
        {
            auto setup = GetSetup();
            auto & code = setup->GetCode();

            // A function with no stack requirements, which would not even
            // need unwind information.
            FunctionSpecification spec(setup->GetAllocator(), -1, 0, 0, 0, FunctionSpecification::BaseRegisterType::Unused, GetDiagnosticsStream());
            ASSERT_NO_FATAL_FAILURE(ValidateUnwindInfo(spec));

            // We impose a stricter requirement that stack must be aligned
            // for all functions, so there's at least one code to perform
            // the alignment even if the function makes no calls or uses
            // no stack. So, 1 quadword slot allocated for the alignment:
            ASSERT_EQ(8, spec.GetOffsetToOriginalRsp());

            // Verify prolog.
            std::vector<uint8_t> offsets;

            code.Reset();
            EmitAndRecordOffset(code,
                                [](FunctionBuffer& f) { f.EmitImmediate<OpCode::Sub>(rsp, 8); },
                                offsets);

            ASSERT_NO_FATAL_FAILURE(VerifyProlog(spec, code));

            // Verify unwind info.
            auto & unwindInfo = *reinterpret_cast<UnwindInfo const *>(spec.GetUnwindInfoBuffer());
            auto unwindCodes = &unwindInfo.m_firstUnwindCode;

            ASSERT_EQ(1, unwindInfo.m_countOfCodes);
            ASSERT_EQ_UNWIND_CODE(UnwindCode(offsets.at(0), UnwindCodeOp::UWOP_ALLOC_SMALL, 1 - 1),
                                unwindCodes[0]);

            // Verify epilog.
            code.Reset();

            code.EmitImmediate<OpCode::Add>(rsp, 8);
            code.Emit<OpCode::Ret>();

            VerifyEpilog(spec, code);
        }


        TEST_F(FunctionBufferTest, FunctionWithCalls)
        {
            auto setup = GetSetup();
            auto & code = setup->GetCode();

            // A function that calls functions with at most 1 argument.
            FunctionSpecification spec(setup->GetAllocator(), 1, 0, 0, 0, FunctionSpecification::BaseRegisterType::Unused, GetDiagnosticsStream());
            ASSERT_NO_FATAL_FAILURE(ValidateUnwindInfo(spec));

            // 4 slots for parameter homes, 1 slot to align stack.
            ASSERT_EQ(40, spec.GetOffsetToOriginalRsp());

            // Verify prolog.
            std::vector<uint8_t> offsets;

            code.Reset();
            EmitAndRecordOffset(code,
                                [](FunctionBuffer& f) { f.EmitImmediate<OpCode::Sub>(rsp, 40); },
                                offsets);

            ASSERT_NO_FATAL_FAILURE(VerifyProlog(spec, code));

            // Verify unwind info.
            auto & unwindInfo = *reinterpret_cast<UnwindInfo const *>(spec.GetUnwindInfoBuffer());
            auto unwindCodes = &unwindInfo.m_firstUnwindCode;

            ASSERT_EQ(1, unwindInfo.m_countOfCodes);
            ASSERT_EQ_UNWIND_CODE(UnwindCode(offsets.at(0), UnwindCodeOp::UWOP_ALLOC_SMALL, 5 - 1),
                                unwindCodes[0]);

            // Verify epilog.
            code.Reset();

            code.EmitImmediate<OpCode::Add>(rsp, 40);
            code.Emit<OpCode::Ret>();

            VerifyEpilog(spec, code);
        }


        TEST_F(FunctionBufferTest, LargeStackAlloc)
        {
            auto setup = GetSetup();
            auto & code = setup->GetCode();

            // A function that allocates 17 stack slots.
            FunctionSpecification spec(setup->GetAllocator(), -1, 17, 0, 0, FunctionSpecification::BaseRegisterType::Unused, GetDiagnosticsStream());
            ASSERT_NO_FATAL_FAILURE(ValidateUnwindInfo(spec));

            // 17 quadword slots exactly (already aligned).
            ASSERT_EQ(136, spec.GetOffsetToOriginalRsp());

            // Verify prolog.
            std::vector<uint8_t> offsets;

            code.Reset();
            EmitAndRecordOffset(code,
                                [](FunctionBuffer& f) { f.EmitImmediate<OpCode::Sub>(rsp, 136); },
                                offsets);

            ASSERT_NO_FATAL_FAILURE(VerifyProlog(spec, code));

            // Verify unwind info.
            auto & unwindInfo = *reinterpret_cast<UnwindInfo const *>(spec.GetUnwindInfoBuffer());
            auto unwindCodes = &unwindInfo.m_firstUnwindCode;

            ASSERT_EQ(2, unwindInfo.m_countOfCodes);
            ASSERT_EQ_UNWIND_CODE2(UnwindCode(offsets.at(0), UnwindCodeOp::UWOP_ALLOC_LARGE, 0),
                                 UnwindCode(17),
                                 unwindCodes[0],
                                 unwindCodes[1]);

            // Verify epilog.
            code.Reset();

            code.EmitImmediate<OpCode::Add>(rsp, 136);
            code.Emit<OpCode::Ret>();

            VerifyEpilog(spec, code);
        }


        TEST_F(FunctionBufferTest, RbpSetToOldRsp)
        {
            auto setup = GetSetup();
            auto & code = setup->GetCode();

            // Max 6 arguments for a call, no explicit register saves, but RBP saved implicitly.
            FunctionSpecification spec(setup->GetAllocator(), 6, 0, 0, 0, FunctionSpecification::BaseRegisterType::SetRbpToOriginalRsp, GetDiagnosticsStream());
            ASSERT_NO_FATAL_FAILURE(ValidateUnwindInfo(spec));

            // 6 slots for parameters, one for RBP, which also aligns the stack.
            ASSERT_EQ(56, spec.GetOffsetToOriginalRsp());

            // Verify prolog.
            std::vector<uint8_t> offsets;

            code.Reset();

            EmitAndRecordOffset(code,
                                [](FunctionBuffer& f) { f.EmitImmediate<OpCode::Sub>(rsp, 56); },
                                offsets);

            EmitAndRecordOffset(code,
                                [](FunctionBuffer& f)
                                {
                                    // Parameters must be right after rsp,
                                    // storage for saving rbp follows.
                                    f.Emit<OpCode::Mov>(rsp, 48, rbp);
                                    f.Emit<OpCode::Lea>(rbp, rsp, 56);
                                },
                                offsets);

            ASSERT_NO_FATAL_FAILURE(VerifyProlog(spec, code));

            // Verify unwind info.
            auto & unwindInfo = *reinterpret_cast<UnwindInfo const *>(spec.GetUnwindInfoBuffer());
            auto unwindCodes = &unwindInfo.m_firstUnwindCode;

            // Reverse the offsets to match the epilog order.
            std::reverse(offsets.begin(), offsets.end());

            ASSERT_EQ(3, unwindInfo.m_countOfCodes);
            ASSERT_EQ_UNWIND_CODE(UnwindCode(offsets.at(1), UnwindCodeOp::UWOP_ALLOC_SMALL, 7 - 1),
                                unwindCodes[2]);
            ASSERT_EQ_UNWIND_CODE2(UnwindCode(offsets.at(0), UnwindCodeOp::UWOP_SAVE_NONVOL, static_cast<uint8_t>(rbp.GetId())),
                                 UnwindCode(6), // Quardword offset off rsp.
                                 unwindCodes[0],
                                 unwindCodes[1]);

            // Verify epilog.
            code.Reset();

            code.Emit<OpCode::Mov>(rbp, rsp, 48);
            code.EmitImmediate<OpCode::Add>(rsp, 56);
            code.Emit<OpCode::Ret>();

            VerifyEpilog(spec, code);
        }


        TEST_F(FunctionBufferTest, Complex)
        {
            auto setup = GetSetup();
            auto & code = setup->GetCode();

            // Calls functions with max 1 argument, 2 local slots for variables,
            // RBP (implicitly) saved, XMM10/11 explicitly saved.
            // Total: 4 slots for calls, 1 for RXX saves, 1 to make the next
            // 4 slots for XMM10/11 16-byte aligned, 2 for variables, 1 empty
            // for ensuring the whole stack is 16-byte aligned. Sum: 13.
            FunctionSpecification spec(setup->GetAllocator(),
                                        1,
                                        2,
                                        0, // RBP implicit due to SetRbpToOriginalRsp.
                                        xmm10.GetMask() | xmm11.GetMask(),
                                        FunctionSpecification::BaseRegisterType::SetRbpToOriginalRsp,
                                        GetDiagnosticsStream());
            ASSERT_NO_FATAL_FAILURE(ValidateUnwindInfo(spec));

            ASSERT_EQ(104, spec.GetOffsetToOriginalRsp());

            // Verify prolog.
            std::vector<uint8_t> offsets;

            code.Reset();

            EmitAndRecordOffset(code,
                                [](FunctionBuffer& f) { f.EmitImmediate<OpCode::Sub>(rsp, 104); },
                                offsets);
            EmitAndRecordOffset(code,
                                // 4 slots skipped for parameters, fifth used to save RBP.
                                [](FunctionBuffer& f) { f.Emit<OpCode::Mov>(rsp, 32, rbp); },
                                offsets);
            EmitAndRecordOffset(code,
                                // Skip offset 40 as it's not 16-byte aligned.
                                [](FunctionBuffer& f) { f.Emit<OpCode::MovAP>(rsp, 48, xmm10s); },
                                offsets);
            EmitAndRecordOffset(code,
                                // 16 bytes needed for xmm10, advance to offset 64 for xmm11.
                                [](FunctionBuffer& f)
                                {
                                    f.Emit<OpCode::MovAP>(rsp, 64, xmm11s);
                                    // Note: offsets [80, 96) are used for the
                                    // 2 variable slots, [96, 104) to align
                                    // the beginning of the stack.
                                    f.Emit<OpCode::Lea>(rbp, rsp, 104);
                                },
                                offsets);

            ASSERT_NO_FATAL_FAILURE(VerifyProlog(spec, code));

            // Verify unwind info.
            auto & unwindInfo = *reinterpret_cast<UnwindInfo const *>(spec.GetUnwindInfoBuffer());
            auto unwindCodes = &unwindInfo.m_firstUnwindCode;

            // Reverse the offsets to match the epilog order.
            std::reverse(offsets.begin(), offsets.end());

            ASSERT_EQ(7, unwindInfo.m_countOfCodes);
            ASSERT_EQ_UNWIND_CODE(UnwindCode(offsets.at(3), UnwindCodeOp::UWOP_ALLOC_SMALL, 13 - 1),
                                unwindCodes[6]);
            ASSERT_EQ_UNWIND_CODE2(UnwindCode(offsets.at(2), UnwindCodeOp::UWOP_SAVE_NONVOL, static_cast<uint8_t>(rbp.GetId())),
                                 UnwindCode(4), // Quadword offset off rsp.
                                 unwindCodes[4],
                                 unwindCodes[5]);
            ASSERT_EQ_UNWIND_CODE2(UnwindCode(offsets.at(1), UnwindCodeOp::UWOP_SAVE_XMM128, static_cast<uint8_t>(xmm10.GetId())),
                                 UnwindCode(3), // 16-byte offset off rsp.
                                 unwindCodes[2],
                                 unwindCodes[3]);
            ASSERT_EQ_UNWIND_CODE2(UnwindCode(offsets.at(0), UnwindCodeOp::UWOP_SAVE_XMM128, static_cast<uint8_t>(xmm11.GetId())),
                                 UnwindCode(4), // 16-byte offset off rsp.
                                 unwindCodes[0],
                                 unwindCodes[1]);

            // Verify epilog.
            code.Reset();

            code.Emit<OpCode::MovAP>(xmm11s, rsp, 64);
            code.Emit<OpCode::MovAP>(xmm10s, rsp, 48);
            code.Emit<OpCode::Mov>(rbp, rsp, 32);
            code.EmitImmediate<OpCode::Add>(rsp, 104);
            code.Emit<OpCode::Ret>();

            VerifyEpilog(spec, code);
        }


#ifdef NATIVEJIT_PLATFORM_WINDOWS
        // NativeJIT only implements stack unwinding on Windows.
        // Therefore, the exception propagation unit test must be
        // disabled for other operating systems.

        static void ThrowTestException()
        {
            throw std::runtime_error("Test");
        }


        TEST_F(FunctionBufferTest, Exception)
        {
            auto setup = GetSetup();

            // A function that preserves all non-volatiles.
            FunctionSpecification spec(setup->GetAllocator(),
                                        -1,
                                        12, // Stack slots
                                        c_rxxWritableNonVolatilesMask,
                                        c_xmmWritableNonVolatilesMask,
                                        FunctionSpecification::BaseRegisterType::Unused,
                                        GetDiagnosticsStream());
            ASSERT_NO_FATAL_FAILURE(ValidateUnwindInfo(spec));

            auto & code = setup->GetCode();

            code.BeginFunctionBodyGeneration(spec);

            // Erase all writable registers. An exception will be thrown
            // later on and the code would crash due to garbage in registers
            // if unwind information wasn't correct.
            ASSERT_NO_FATAL_FAILURE(FillAllWritableRegistersWithGarbage(code));

            // Call a function that will trigger an exception.
            code.EmitImmediate<OpCode::Mov>(rax, &ThrowTestException);
            code.Emit<OpCode::Call>(rax);

            code.EndFunctionBodyGeneration(spec);

            auto func = reinterpret_cast<void (*)()>(const_cast<void*>(code.GetEntryPoint()));
            bool exceptionCaught = false;

            try
            {
                ASSERT_FALSE(exceptionCaught);
                func();
                FAIL() << "Should not have reached here";
            }
            catch (std::exception const &e)
            {
                ASSERT_FALSE(exceptionCaught);
                ASSERT_EQ(std::string("Test"), std::string(e.what()));

                exceptionCaught = true;
            }
            catch (...)
            {
                FAIL() << "Unexpected exception caught";
            }

            ASSERT_TRUE(exceptionCaught);
        }
#endif


        // This tests that, FunctionSpecification correctly drives non-volatile
        // save and restore. This does not test that the register allocator
        // correctly indicates which volatiles were clobbered.
        TEST_F(FunctionBufferTest, RegisterPreservation)
        {
            ALIGNAS(16) RegInfo before;
            ALIGNAS(16) RegInfo after;

            // Make sure that the test fails if these don't get filled in.
            memset(&before, 1, sizeof(before));
            memset(&after, 2, sizeof(after));

            auto setup = GetSetup();
            auto & code = setup->GetCode();

            void (*saveBeforeFunc)() = EmitSaveNonVolatilesCode(code, before);
            void (*saveAfterFunc)() = EmitSaveNonVolatilesCode(code, after);

            // A function that perserves all non-volatiles.
            FunctionSpecification spec(setup->GetAllocator(),
                                        -1,
                                        12, // Allocate some slots to increase entropy.
                                        c_rxxWritableNonVolatilesMask,
                                        c_xmmWritableNonVolatilesMask,
                                        FunctionSpecification::BaseRegisterType::Unused,
                                        GetDiagnosticsStream());
            ASSERT_NO_FATAL_FAILURE(ValidateUnwindInfo(spec));

            // Erase all writable registers to show that epilogue indeed restores
            // nonvolatiles.
            code.BeginFunctionBodyGeneration(spec);
            ASSERT_NO_FATAL_FAILURE(FillAllWritableRegistersWithGarbage(code));
            code.EndFunctionBodyGeneration(spec);

            auto mainFunc = reinterpret_cast<void (*)()>(const_cast<void*>(code.GetEntryPoint()));

            // Note: there's an assumption that no nonvolatiles will be
            // modified after saveBeforeFunc() completes and before mainFunc()
            // starts. There's a similar assumption between mainFunc() and
            // saveAfterFunc().
            saveBeforeFunc();
            mainFunc();
            saveAfterFunc();

            // Verify whether nonvolatiles in before and after storages are the same.
            unsigned regMask = c_rxxWritableNonVolatilesMask;
            unsigned regId;

            while (BitOp::GetLowestBitSet(regMask, &regId))
            {
                ASSERT_EQ(before.m_rxx[regId], after.m_rxx[regId])
                    << "Mismatch for register "
                    << Register<8, false>(regId).GetName();
                BitOp::ClearBit(&regMask, regId);
            }

            regMask = c_xmmWritableNonVolatilesMask;

            while (BitOp::GetLowestBitSet(regMask, &regId))
            {
              ASSERT_EQ(before.m_xmm[2 * regId], after.m_xmm[2 * regId])
                  << "Mismatch for register "
                  << Register<8, true>(regId).GetName();
              ASSERT_EQ(before.m_xmm[2 * regId] + 1,
                        after.m_xmm[2 * regId] + 1)
                  << "Mismatch for register "
                  << Register<8, true>(regId).GetName();
              BitOp::ClearBit(&regMask, regId);
            }
        }

        TEST_CASES_END
    }
}

#undef ASSERT_EQ_UNWIND_CODE
#undef ASSERT_EQ_UNWIND_CODE2
#undef ALIGNAS
