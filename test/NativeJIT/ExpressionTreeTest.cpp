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


#include <iostream>

#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/Function.h"
#include "Temporary/Allocator.h"
#include "TestSetup.h"


namespace NativeJIT
{
    namespace ExpressionTreeUnitTest
    {
        template <typename T>
        using DirectRegister = typename Storage<T>::DirectRegister;

        template <typename T>
        using BaseRegister = typename Storage<T>::BaseRegister;

        #define AssertSizeAndType(REG_TYPE, EXPECTED_SIZE, EXPECTED_IS_FLOAT) \
            static_assert(REG_TYPE::c_size == EXPECTED_SIZE, #REG_TYPE " should require " #EXPECTED_SIZE " bytes"); \
            static_assert(REG_TYPE::c_isFloat == EXPECTED_IS_FLOAT, "IsFloat property for " #REG_TYPE " should be " #EXPECTED_IS_FLOAT)

        // These tests can only fail during compilation, never during runtime.
        TEST(ExpressionTreeBasic, StorageDefinition)
        {
            struct SmallerThanQuadword
            {
                uint8_t m_x1;
                uint8_t m_x2;
            };

            struct LargerThanQuadword
            {
                uint64_t m_x1;
                uint16_t m_x2;
            };

            // Direct register for primitive and small types.
            AssertSizeAndType(DirectRegister<char>, 1, false);
            AssertSizeAndType(DirectRegister<const char>, 1, false);
            AssertSizeAndType(DirectRegister<uint64_t>, 8, false);

            AssertSizeAndType(DirectRegister<float>, 4, true);
            AssertSizeAndType(DirectRegister<double>, 8, true);
            AssertSizeAndType(DirectRegister<const float>, 4, true);

            AssertSizeAndType(DirectRegister<SmallerThanQuadword>, 2, false);

            // Direct register for pointers.
            AssertSizeAndType(DirectRegister<char*>, 8, false);
            AssertSizeAndType(DirectRegister<char const *>, 8, false);
            AssertSizeAndType(DirectRegister<char const * const>, 8, false);

            AssertSizeAndType(DirectRegister<void*>, 8, false);
            AssertSizeAndType(DirectRegister<float*>, 8, false);
            AssertSizeAndType(DirectRegister<double*>, 8, false);

            // Direct register for references.
            AssertSizeAndType(DirectRegister<char&>, 8, false);
            AssertSizeAndType(DirectRegister<const char&>, 8, false);
            AssertSizeAndType(DirectRegister<float &>, 8, false);

            // Direct register for arrays.
            AssertSizeAndType(DirectRegister<int[1024]>, 8, false);
            AssertSizeAndType(DirectRegister<float[1024]>, 8, false);

            // Direct register for structures larger than quadword.
            AssertSizeAndType(DirectRegister<LargerThanQuadword>, 8, false);

            // Base register for various types.
            AssertSizeAndType(BaseRegister<char>, 8, false);
            AssertSizeAndType(BaseRegister<uint64_t>, 8, false);
            AssertSizeAndType(BaseRegister<float>, 8, false);
            AssertSizeAndType(BaseRegister<float[1024]>, 8, false);
        }

        #undef AssertSizeAndType


        #define AssertImmediateCategory(TYPE, EXPECTED_CATEGORY) \
            static_assert(ImmediateCategoryOf<TYPE>::value == EXPECTED_CATEGORY, \
                          #TYPE " should be in " #EXPECTED_CATEGORY)

        TEST(ExpressionTreeBasic, ImmediateTypes)
        {
            AssertImmediateCategory(void, ImmediateCategory::NonImmediate);

            AssertImmediateCategory(char, ImmediateCategory::InlineImmediate);
            AssertImmediateCategory(const char, ImmediateCategory::InlineImmediate);
            AssertImmediateCategory(int32_t, ImmediateCategory::InlineImmediate);
            AssertImmediateCategory(uint32_t, ImmediateCategory::InlineImmediate);

            AssertImmediateCategory(uint64_t, ImmediateCategory::RIPRelativeImmediate);
            AssertImmediateCategory(float, ImmediateCategory::RIPRelativeImmediate);
            AssertImmediateCategory(double, ImmediateCategory::RIPRelativeImmediate);
            AssertImmediateCategory(void*, ImmediateCategory::RIPRelativeImmediate);
            AssertImmediateCategory(void(*)(int), ImmediateCategory::RIPRelativeImmediate);

            // These decay to pointers when passed as arguments.
            typedef char ArrayLargerThanQuadword[123];
            struct StructLargerThanQuadword { ArrayLargerThanQuadword m_x;};

            AssertImmediateCategory(ArrayLargerThanQuadword, ImmediateCategory::RIPRelativeImmediate);
            AssertImmediateCategory(StructLargerThanQuadword, ImmediateCategory::RIPRelativeImmediate);
        }

        #undef AssertImmediateCategory


        TEST_FIXTURE_START(ExpressionTree)
        TEST_FIXTURE_END_TEST_CASES_BEGIN

        // Verify that the sole owner of an indirect storage will reuse the
        // base register for the direct register after dereferencing.
        TEST_F(ExpressionTree, BaseRegisterReuse)
        {
            auto setup = GetSetup();
            ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

            // Create indirect storage to reference an integer.
            struct Test { int m_dummy; };
            Test testStruct;

            auto & structNode = e.Immediate(&testStruct);
            auto & indirectNode = e.Deref(e.FieldPointer(structNode, &Test::m_dummy));
            indirectNode.IncrementParentCount();

            structNode.CodeGenCache(e);
            indirectNode.CodeGenCache(e);

            auto storage = indirectNode.CodeGen(e);
            auto baseRegister = storage.GetBaseRegister();

            ASSERT_TRUE(storage.ConvertToDirect(false).IsSameHardwareRegister(baseRegister));
        }


        // Verify that the sole owner will not reuse the base register in
        // cases when it's not possible (floating point value, reserved
        // base pointer, RIP relative addressing).

        TEST_F(ExpressionTree, NoBaseRegisterReuseRIPRelative)
        {
            auto setup = GetSetup();
            ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

            auto storage = e.RIPRelative<int>(16);
            ASSERT_TRUE(storage.GetBaseRegister().IsRIP());

            ASSERT_FALSE(storage.ConvertToDirect(false).IsRIP());
        }


        TEST_F(ExpressionTree, NoBaseRegisterReuseBasePointer)
        {
            auto setup = GetSetup();
            ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

            auto storage = e.Temporary<int>();
            auto baseRegister = storage.GetBaseRegister();

            ASSERT_FALSE(storage.ConvertToDirect(false).IsSameHardwareRegister(baseRegister));
        }


        TEST_F(ExpressionTree, NoBaseRegisterReuseFloat)
        {
            auto setup = GetSetup();
            ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

            // Create indirect storage to reference a float.
            struct Test { float m_dummy; };
            Test testStruct;

            auto & structNode = e.Immediate(&testStruct);
            auto & indirectNode = e.Deref(e.FieldPointer(structNode, &Test::m_dummy));
            indirectNode.IncrementParentCount();

            structNode.CodeGenCache(e);
            indirectNode.CodeGenCache(e);

            auto storage = indirectNode.CodeGen(e);
            auto baseRegister = storage.GetBaseRegister();

            ASSERT_FALSE(storage.ConvertToDirect(false).IsSameHardwareRegister(baseRegister));
        }


        // Make sure that expression tree allows multiple references to
        // the reserved base pointers.

        TEST_F(ExpressionTree, MultipleReferencesToBasePointer)
        {
            auto setup = GetSetup();
            ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

            auto temp1 = e.Temporary<int>();
            auto temp2 = e.Temporary<int>();

            ASSERT_EQ(temp1.GetBaseRegister(), temp2.GetBaseRegister());
            ASSERT_NE(temp1.GetOffset(), temp2.GetOffset());
        }


        TEST_F(ExpressionTree, MultipleReferencesToRIP)
        {
            auto setup = GetSetup();
            ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

            auto temp1 = e.RIPRelative<int>(0);
            auto temp2 = e.RIPRelative<int>(16);

            ASSERT_TRUE(temp1.GetBaseRegister().IsRIP());
            ASSERT_EQ(temp1.GetBaseRegister(), temp2.GetBaseRegister());
        }


        TEST_F(ExpressionTree, ReferenceCounter)
        {
            unsigned count = 0;

            ReferenceCounter ref1(count);
            ASSERT_EQ(1u, count);

            ReferenceCounter ref2(count);
            ASSERT_EQ(2u, count);

            ReferenceCounter ref3;
            ASSERT_EQ(2u, count);

            ref3 = ref2;
            ASSERT_EQ(3u, count);

            ref3.Reset();
            ASSERT_EQ(2u, count);

            ref2 = ReferenceCounter();
            ASSERT_EQ(1u, count);

            ref1.Reset();
            ASSERT_EQ(0u, count);
        }


        TEST_F(ExpressionTree, RegisterSpillingInteger)
        {
            auto setup = GetSetup();
            ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());
            std::vector<Storage<int>> storages;
            const unsigned totalRegisterCount = RegisterBase::c_maxIntegerRegisterID + 1;

            // Try to obtain totalRegisterCount direct registers.
            for (unsigned i = 0; i < totalRegisterCount; ++i)
            {
                storages.push_back(e.Direct<int>());
                ASSERT_EQ(storages.back().GetStorageClass(), StorageClass::Direct);
            }

            // There won't be enough registers for all storages to stay
            // direct since reserved base pointers can't be obtained. Thus,
            // some of the storages should have their registers spilled.

            // Three reserved registers are RBP, RSP and RIP. Note: this
            // assumes that RBP is a base register - if RSP is used instead
            // of RBP, the test will need to be adjusted.
            // Alternatively, ExpressionTree::IsAnyBaseRegister could be
            // made accessible and used to count the reserved registers.
            const unsigned reservedRegisterCount = 3;
            unsigned directCount = 0;
            unsigned indirectCount = 0;
            unsigned indexOfFirstDirect = 0;

            for (unsigned i = 0; i < storages.size(); ++i)
            {
                if (storages[i].GetStorageClass() == StorageClass::Direct)
                {
                    directCount++;

                    if (directCount == 1)
                    {
                        indexOfFirstDirect = i;
                    }
                }
                else
                {
                    indirectCount++;
                }
            }

            // Most storages should be direct, some corresponding to the
            // reserved registers should not.
            ASSERT_EQ(totalRegisterCount - reservedRegisterCount, directCount);
            ASSERT_EQ(reservedRegisterCount, indirectCount);

            // A new direct storage should cause the oldest reserved register
            // to be spilled (note: this assumes current allocation strategy).
            ASSERT_EQ(storages[indexOfFirstDirect].GetStorageClass(), StorageClass::Direct);
            Storage<int> spillTrigger = e.Direct<int>();

            ASSERT_EQ(spillTrigger.GetStorageClass(),  StorageClass::Direct);
            ASSERT_EQ(storages[indexOfFirstDirect].GetStorageClass(), StorageClass::Indirect);
        }


        TEST_F(ExpressionTree, RegisterSpillingFloat)
        {
            auto setup = GetSetup();
            ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());
            std::vector<Storage<float>> storages;
            const unsigned totalRegisterCount = RegisterBase::c_maxFloatRegisterID + 1;

            // Try to obtain totalRegisterCount direct registers.
            for (unsigned i = 0; i < totalRegisterCount; ++i)
            {
                storages.push_back(e.Direct<float>());
                ASSERT_EQ(storages.back().GetStorageClass(),  StorageClass::Direct);
            }

            // There are no reserved registers for floats, so no register
            // should have been bumped.
            for (auto const & s : storages)
            {
                ASSERT_EQ(s.GetStorageClass(), StorageClass::Direct);
            }

            // A new direct storage should cause the oldest reserved register
            // to be spilled (note: this assumes current allocation strategy).
            Storage<float> spillTrigger = e.Direct<float>();

            ASSERT_EQ(spillTrigger.GetStorageClass(), StorageClass::Direct);
            ASSERT_EQ(storages[0].GetStorageClass(), StorageClass::Indirect);

            Storage<float> spillTrigger2 = e.Direct<float>();

            ASSERT_EQ(spillTrigger2.GetStorageClass(), StorageClass::Direct);
            ASSERT_EQ(storages[1].GetStorageClass(), StorageClass::Indirect);

            // Make sure that a released register will be used for a new
            // direct rather than spilling storages[2] for it.
            ASSERT_EQ(storages[2].GetStorageClass(), StorageClass::Direct);
            spillTrigger2.Reset();

            spillTrigger2 = e.Direct<float>();
            ASSERT_EQ(storages[2].GetStorageClass(), StorageClass::Direct);
        }


        template <typename T>
        void TestRegisterPinning(TestCaseSetup& setup)
        {
            ExpressionNodeFactory e(setup.GetAllocator(), setup.GetCode());

            // Get a direct register and pin it.
            Storage<T> storage = e.Direct<T>();
            ReferenceCounter pin = storage.GetPin();

            auto reg = storage.GetDirectRegister();

            // Attempt to obtain that specific register should now fail.
            try
            {
                e.Direct<T>(reg);
                FAIL() << "It should not have been possible to obtain a pinned register";
            }
            catch (std::exception const & e)
            {
                std::string msg = e.what();

                ASSERT_TRUE(msg.find("Attempted to obtain the pinned register") !=
                            std::string::npos) <<
                  "Unexpected exception received";
            }
            catch (...)
            {
                FAIL() << "Unexpected exception type";
            }

            // Unpin the register and try obtaining it again (should succceed now).
            pin.Reset();
            Storage<T> storage2 = e.Direct<T>(reg);

            // The new storage should own the register, the old not should not.
            ASSERT_EQ(storage2.GetStorageClass(), StorageClass::Direct);
            ASSERT_EQ(storage2.GetDirectRegister(), reg);
            ASSERT_FALSE((storage.GetStorageClass() == StorageClass::Direct
                         && storage.GetDirectRegister() == reg));
        }


        TEST_F(ExpressionTree, RegisterPinning)
        {
            auto setup = GetSetup();

            TestRegisterPinning<int>(*setup);
            TestRegisterPinning<float>(*setup);
        }


        TEST_F(ExpressionTree, SoleDataOwner)
        {
            auto setup = GetSetup();
            ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

            Storage<int> empty;
            ASSERT_TRUE(empty.IsSoleDataOwner());

            auto s = e.Direct<int>();
            ASSERT_TRUE(s.IsSoleDataOwner());

            auto s2 = s;
            ASSERT_FALSE(s.IsSoleDataOwner());
            ASSERT_FALSE(s2.IsSoleDataOwner());

            s2.Reset();
            ASSERT_TRUE(s.IsSoleDataOwner());
            ASSERT_TRUE(s2.IsSoleDataOwner());

            // Different indirects that refer to stack are sole owners of their storage.
            auto stack1 = e.Temporary<int>();
            ASSERT_TRUE(stack1.IsSoleDataOwner());

            auto stack2 = e.Temporary<int>();
            ASSERT_TRUE(stack2.IsSoleDataOwner());
        }


        TEST_F(ExpressionTree, TakeSoleOwnershipOfDirect)
        {
            auto setup = GetSetup();
            ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

            auto s = e.Direct<int>(eax);
            ASSERT_TRUE(s.IsSoleDataOwner());
            ASSERT_EQ(s.GetStorageClass(), StorageClass::Direct);
            ASSERT_EQ(s.GetDirectRegister(), eax);

            auto s2 = s;
            ASSERT_FALSE(s.IsSoleDataOwner());
            ASSERT_FALSE(s2.IsSoleDataOwner());
            ASSERT_EQ(s2.GetStorageClass(), StorageClass::Direct);
            ASSERT_EQ(s2.GetDirectRegister(), eax);

            s.TakeSoleOwnershipOfDirect();

            ASSERT_TRUE(s.IsSoleDataOwner());
            ASSERT_EQ(s.GetStorageClass(), StorageClass::Direct);
            ASSERT_EQ(s.GetDirectRegister(), eax);

            ASSERT_TRUE(s2.IsSoleDataOwner());
            ASSERT_EQ(s2.GetStorageClass(), StorageClass::Direct);
            // ASSERT_NE(s2.GetDirectRegister(), eax); Doesn't compile!
            ASSERT_FALSE((s2.GetDirectRegister() == eax));
        }


        TEST_CASES_END
    }
}
