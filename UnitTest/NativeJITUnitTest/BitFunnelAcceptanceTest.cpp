#include "stdafx.h"


#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "NativeJIT/Packed.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"

#include "NativeJIT/Model.h"


namespace NativeJIT
{
    //*************************************************************************
    //
    // This file contains temporary unit test used during development of
    // partially implemented features. The intention is that this file will
    // eventually be deleted.
    //
    //*************************************************************************
    namespace BitFunnelAcceptance
    {
        TestClass(AcceptanceUnitTest)
        {
        public:
            AcceptanceUnitTest()
                : m_allocator(5000),
                  m_executionBuffer(5000)
            {
                m_code.reset(new FunctionBuffer(m_executionBuffer, 5000, 10, 10, 3, 0, false));
            }


            typedef Packed<2, Packed <3, Packed <4, Packed<5>>>> TermFeatures;
            typedef Model<TermFeatures> TermModel;

            typedef Packed<4> DocFeatures;
            typedef Model<DocFeatures> DocModel;

            typedef unsigned __int64 TermHash;
            typedef unsigned __int64 DocId;
            typedef unsigned __int32 Shard;

            typedef unsigned __int32 DocIndex;

            // TODO: Currently it is not possible to call a function that returns void.
            // The templates won't compile because of a sizeof(void).
            // typedef void (*Callback)(DocId docId, float score);
            //typedef void (*Callback)(DocId docId, float score);
            typedef int (*Callback)(DocId docId, float score);

            class TermScoreTable
            {
            public:
                TermFeatures Lookup(TermHash key);
            };

            // TODO: Will eventually need a way to resolve the address of LookupHelper on remote system.
            // Either ProcessResources contains all function pointers for various feature types or
            // codegen does casts or inline lookup code.
            static TermFeatures LookupHelper(TermScoreTable* table, TermHash key)
            {
                return TermFeatures();
            }

            typedef TermFeatures (*LookupHelperType)(TermScoreTable* table, TermHash key);

#pragma pack(push, 1)
            struct DocInfo
            {
                TermScoreTable* m_termScoreTable;
                unsigned m_staticScore;         // TODO: Need some way to make Packed use less than 64 bits.
                                                // Either Packed automatically sizes m_fields or need constructor
                                                // from smaller sizes.
                DocId m_docId;
            };
#pragma pack(pop)

            class DocTable
            {
            public:
                // TODO: Figure out how to support this form.
                // DocInfo m_entries[100];
                DocInfo* m_entries;
            };

            struct ModelSet
            {
                TermModel* m_termModel;
                DocModel* m_docModel;
            };

            struct ProcessResources
            {
                ModelSet* m_models;
                DocTable *m_docTable;
                Callback m_callback;
            };


            TestCase(BitFunnel)
            {
                AutoResetAllocator reset(m_allocator);

                {
                    typedef Packed<3, Packed<4, Packed<5>>> PackedType;
                    typedef Model<PackedType> ModelType;

                    // TODO: Currently the JIT does not support return type of void.
                    //Function<void, ProcessResources*, Shard, DocIndex> expression(m_allocator, *m_code);
                    Function<int, ProcessResources*, Shard, DocIndex> expression(m_allocator, *m_code);

                    auto & processResources = expression.GetP1();
                    auto & shard = expression.GetP2();
                    auto & docIndex = expression.GetP3();

                    // TODO: Is LookupHelperType necessary?
                    auto & lookupFunction = expression.Immediate<LookupHelperType>(LookupHelper);

                    auto & docTable = expression.Deref(expression.FieldPointer(processResources, &ProcessResources::m_docTable));
                    auto & docTableEntries = expression.Deref(expression.FieldPointer(docTable, &DocTable::m_entries));
                    auto & docInfo = expression.Add(docTableEntries, docIndex);
                    auto & termScoreTable = expression.Deref(expression.FieldPointer<DocInfo, TermScoreTable*>(docInfo, &DocInfo::m_termScoreTable));

                    auto & callback = expression.Deref(expression.FieldPointer(processResources, & ProcessResources::m_callback));

                    auto & modelSet = expression.Deref(expression.FieldPointer(processResources, &ProcessResources::m_models));
                    auto & termModel = expression.Deref(expression.FieldPointer(modelSet, &ModelSet::m_termModel));

                    // Term 1
                    auto & termHash1 = expression.Immediate(1234ull);
                    auto & termFeatures1 = expression.Call(lookupFunction, termScoreTable, termHash1);
                    //    // TODO: Need to implement Packed::Push() to append shard.
                    auto & termScore1 = expression.ApplyModel(termModel, termFeatures1);

                    auto & docId = expression.Deref(expression.FieldPointer(docInfo, &DocInfo::m_docId));

                    auto & result = expression.Call(callback, docId, termScore1);

                    auto function = expression.Compile(result);

                    //ProcessResources p1;
                    Shard p2 = 3;
                    DocIndex p3 = 0x1234;

                    //auto expected = model.Apply(packedParameter);
                    //auto observed = function(&p1, p2, p3);

                    //TestAssert(observed == expected);
                }
            }


/*************************************I*************************
imul r8, 14h                    // r8 = docIndex * sizeof(DocInfo)
mov r12, rcx                    // r12 = processResources
mov r12, [r12 + 8h]             // r12 = processResources->docTable
mov r12, [r12]                  // r12 = &processResources->docTable->m_entries[0]
add r12, r8                     // r12 = &processResources->docTable->m_entries[docIndex]

mov r8, rcx                     // r8 = processResources (save for later)
mov r11, r12                    // r11 = docInfo (save for later)

mov r10, 000007FAA29A30B2h      // r10 = &Lookup

mov r9, rcx                     // r9 = processResources (save for later)
mov rcx, [r12]                  // rcx = docInfo.m_termScoreTable

mov r12, rdx                    // r12 = shard (save for later)
mov rdx, 4D2h                   // rdx = termHash1

push r8
push r9
push r11
call r10                        // rax = Lookup(docInfo.m_termScoreTable, termHash1)
pop r11
pop r9
pop r8

imul rax, 4h                    // rax *= sizeof(float) for index into model float array
mov r9, [r9]                    // r9 = processResources->m_models
mov r9, [r9]                    // r9 = processResources->m_models->m_termModel
add r9, rax                     // r9 = &processResources->m_models->m_termModel[valueIndex]

mov r8, [r8 + 10h]              // r8 = processResources->m_callback
mov rcx, [r11 + Ch]             // rcx = docInfo.m_docId

mov xmm1s, [r9]                 // xmm1s = *modelValuePointer

call r8                         // Callback(docId, modelValue)

*****************************/

        private:
            Allocator m_allocator;
            ExecutionBuffer m_executionBuffer;
            std::unique_ptr<FunctionBuffer> m_code;
        };
    }
}
