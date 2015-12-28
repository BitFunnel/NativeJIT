#include "stdafx.h"

#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "SuiteCpp/UnitTest.h"
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
        TestCase(StorageDefinition)
        {
            struct SmallerThanQuadword
            {
                unsigned __int8 m_x1;
                unsigned __int8 m_x2;
            };

            struct LargerThanQuadword
            {
                unsigned __int64 m_x1;
                unsigned __int16 m_x2;
            };

            // Direct register for primitive and small types.
            AssertSizeAndType(DirectRegister<char>, 1, false);
            AssertSizeAndType(DirectRegister<const char>, 1, false);
            AssertSizeAndType(DirectRegister<unsigned __int64>, 8, false);

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
            AssertSizeAndType(BaseRegister<unsigned __int64>, 8, false);
            AssertSizeAndType(BaseRegister<float>, 8, false);
            AssertSizeAndType(BaseRegister<float[1024]>, 8, false);
        }

        #undef AssertSizeAndType


        #define AssertImmediateCategory(TYPE, EXPECTED_CATEGORY) \
            static_assert(ImmediateCategoryOf<TYPE>::value == EXPECTED_CATEGORY, \
                          #TYPE " should be in " #EXPECTED_CATEGORY)

        TestCase(ImmediateTypes)
        {
            AssertImmediateCategory(void, ImmediateCategory::NonImmediate);

            AssertImmediateCategory(char, ImmediateCategory::InlineImmediate);
            AssertImmediateCategory(const char, ImmediateCategory::InlineImmediate);
            AssertImmediateCategory(__int32, ImmediateCategory::InlineImmediate);
            AssertImmediateCategory(unsigned __int32, ImmediateCategory::InlineImmediate);

            AssertImmediateCategory(unsigned __int64, ImmediateCategory::RIPRelativeImmediate);
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


        TestClass(ExpressionTreeTest), private TestFixture
        {
        public:
            // Verify that the sole owner of an indirect storage will reuse the
            // base register for the direct register after dereferencing.
            TestCase(BaseRegisterReuse)
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

                TestAssert(storage.ConvertToDirect(false).IsSameHardwareRegister(baseRegister));
            }


            // Verify that the sole owner will not reuse the base register in
            // cases when it's not possible (floating point value, reserved
            // base pointer, RIP relative addressing).

            TestCase(NoBaseRegisterReuseRIPRelative)
            {
                auto setup = GetSetup();
                ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

                auto storage = e.RIPRelative<int>(16);
                TestAssert(storage.GetBaseRegister().IsRIP());

                TestAssert(!storage.ConvertToDirect(false).IsRIP());
            }


            TestCase(NoBaseRegisterReuseBasePointer)
            {
                auto setup = GetSetup();
                ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

                auto storage = e.Temporary<int>();
                auto baseRegister = storage.GetBaseRegister();

                TestAssert(!storage.ConvertToDirect(false).IsSameHardwareRegister(baseRegister));
            }


            TestCase(NoBaseRegisterReuseFloat)
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

                TestAssert(!storage.ConvertToDirect(false).IsSameHardwareRegister(baseRegister));
            }


            // Make sure that expression tree allows multiple references to
            // the reserved base pointers.

            TestCase(MultipleReferencesToBasePointer)
            {
                auto setup = GetSetup();
                ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

                auto temp1 = e.Temporary<int>();
                auto temp2 = e.Temporary<int>();

                TestAssert(temp1.GetBaseRegister() == temp2.GetBaseRegister());
                TestAssert(temp1.GetOffset() != temp2.GetOffset());
            }


            TestCase(MultipleReferencesToRIP)
            {
                auto setup = GetSetup();
                ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

                auto temp1 = e.RIPRelative<int>(0);
                auto temp2 = e.RIPRelative<int>(16);

                TestAssert(temp1.GetBaseRegister().IsRIP());
                TestAssert(temp1.GetBaseRegister() == temp2.GetBaseRegister());
            }


            TestCase(ReferenceCounter)
            {
                unsigned count = 0;

                ReferenceCounter ref1(count);
                TestEqual(1u, count);

                ReferenceCounter ref2(count);
                TestEqual(2u, count);

                ReferenceCounter ref3;
                TestEqual(2u, count);

                ref3 = ref2;
                TestEqual(3u, count);

                ref3.Reset();
                TestEqual(2u, count);

                ref2 = ReferenceCounter();
                TestEqual(1u, count);

                ref1.Reset();
                TestEqual(0u, count);
            }


            TestCase(RegisterSpillingInteger)
            {
                auto setup = GetSetup();
                ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());
                std::vector<Storage<int>> storages;
                const unsigned totalRegisterCount = RegisterBase::c_maxIntegerRegisterID + 1;

                // Try to obtain totalRegisterCount direct registers.
                for (unsigned i = 0; i < totalRegisterCount; ++i)
                {
                    storages.push_back(e.Direct<int>());
                    TestAssert(storages.back().GetStorageClass() == StorageClass::Direct);
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
                TestEqual(totalRegisterCount - reservedRegisterCount, directCount);
                TestEqual(reservedRegisterCount, indirectCount);

                // A new direct storage should cause the oldest reserved register
                // to be spilled (note: this assumes current allocation strategy).
                TestAssert(storages[indexOfFirstDirect].GetStorageClass() == StorageClass::Direct);
                Storage<int> spillTrigger = e.Direct<int>();

                TestAssert(spillTrigger.GetStorageClass() == StorageClass::Direct);
                TestAssert(storages[indexOfFirstDirect].GetStorageClass() == StorageClass::Indirect);
            }


            TestCase(RegisterSpillingFloat)
            {
                auto setup = GetSetup();
                ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());
                std::vector<Storage<float>> storages;
                const unsigned totalRegisterCount = RegisterBase::c_maxFloatRegisterID + 1;

                // Try to obtain totalRegisterCount direct registers.
                for (unsigned i = 0; i < totalRegisterCount; ++i)
                {
                    storages.push_back(e.Direct<float>());
                    TestAssert(storages.back().GetStorageClass() == StorageClass::Direct);
                }

                // There are no reserved registers for floats, so no register
                // should have been bumped.
                for (auto const & s : storages)
                {
                    TestAssert(s.GetStorageClass() == StorageClass::Direct);
                }

                // A new direct storage should cause the oldest reserved register
                // to be spilled (note: this assumes current allocation strategy).
                Storage<float> spillTrigger = e.Direct<float>();

                TestAssert(spillTrigger.GetStorageClass() == StorageClass::Direct);
                TestAssert(storages[0].GetStorageClass() == StorageClass::Indirect);

                Storage<float> spillTrigger2 = e.Direct<float>();

                TestAssert(spillTrigger2.GetStorageClass() == StorageClass::Direct);
                TestAssert(storages[1].GetStorageClass() == StorageClass::Indirect);

                // Make sure that a released register will be used for a new
                // direct rather than spilling storages[2] for it.
                TestAssert(storages[2].GetStorageClass() == StorageClass::Direct);
                spillTrigger2.Reset();

                spillTrigger2 = e.Direct<float>();
                TestAssert(storages[2].GetStorageClass() == StorageClass::Direct);
            }


            template <typename T>
            void TestRegisterPinning()
            {
                auto setup = GetSetup();
                ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

                // Get a direct register and pin it.
                Storage<T> storage = e.Direct<T>();
                ReferenceCounter pin = storage.GetPin();

                auto reg = storage.GetDirectRegister();

                // Attempt to obtain that specific register should now fail.
                try
                {
                    e.Direct<T>(reg);
                    TestFail("It should not have been possible to obtain a pinned register");
                }
                catch (std::exception const & e)
                {
                    std::string msg = e.what();

                    TestAssert(msg.find("Attempted to obtain the pinned register") != std::string::npos,
                               "Unexpected exception received");
                }
                catch (...)
                {
                    TestFail("Unexpected exception type");
                }

                // Unpin the register and try obtaining it again (should succceed now).
                pin.Reset();
                Storage<T> storage2 = e.Direct<T>(reg);

                // The new storage should own the register, the old not should not.
                TestAssert(storage2.GetStorageClass() == StorageClass::Direct
                           && storage2.GetDirectRegister() == reg);
                TestAssert(!(storage.GetStorageClass() == StorageClass::Direct
                             && storage.GetDirectRegister() == reg));
            }


            TestCase(RegisterPinning)
            {
                TestRegisterPinning<int>();
                TestRegisterPinning<float>();
            }


            TestCase(SoleDataOwner)
            {
                auto setup = GetSetup();
                ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

                Storage<int> empty;
                TestAssert(empty.IsSoleDataOwner());

                auto s = e.Direct<int>();
                TestAssert(s.IsSoleDataOwner());

                auto s2 = s;
                TestAssert(!s.IsSoleDataOwner());
                TestAssert(!s2.IsSoleDataOwner());

                s2.Reset();
                TestAssert(s.IsSoleDataOwner());
                TestAssert(s2.IsSoleDataOwner());

                // Different indirects that refer to stack are sole owners of their storage.
                auto stack1 = e.Temporary<int>();
                TestAssert(stack1.IsSoleDataOwner());

                auto stack2 = e.Temporary<int>();
                TestAssert(stack2.IsSoleDataOwner());
            }


            TestCase(TakeSoleOwnershipOfDirect)
            {
                auto setup = GetSetup();
                ExpressionNodeFactory e(setup->GetAllocator(), setup->GetCode());

                auto s = e.Direct<int>(eax);
                TestAssert(s.IsSoleDataOwner());
                TestAssert(s.GetStorageClass() == StorageClass::Direct);
                TestAssert(s.GetDirectRegister() == eax);

                auto s2 = s;
                TestAssert(!s.IsSoleDataOwner());
                TestAssert(!s2.IsSoleDataOwner());
                TestAssert(s2.GetStorageClass() == StorageClass::Direct);
                TestAssert(s2.GetDirectRegister() == eax);

                s.TakeSoleOwnershipOfDirect();

                TestAssert(s.IsSoleDataOwner());
                TestAssert(s.GetStorageClass() == StorageClass::Direct);
                TestAssert(s.GetDirectRegister() == eax);

                TestAssert(s2.IsSoleDataOwner());
                TestAssert(s2.GetStorageClass() == StorageClass::Direct);
                TestAssert(!(s2.GetDirectRegister() == eax));
            }
        };
    }
}
