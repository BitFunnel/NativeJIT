#include "stdafx.h"

#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "SuiteCpp/UnitTest.h"
#include "Temporary/Allocator.h"


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


        TestClass(ExpressionTreeTest)
        {
        public:
            ExpressionTreeTest()
                : m_allocator(5000),
                  m_executionBuffer(5000)
            {
                m_code.reset(new FunctionBuffer(m_executionBuffer, 5000, 10, 10, 3, 0, false));
            }


            // Verify that the sole owner of an indirect storage will reuse the
            // base register for the direct register after dereferencing.
            TestCase(BaseRegisterReuse)
            {
                AutoResetAllocator reset(m_allocator);
                ExpressionNodeFactory e(m_allocator, *m_code);

                // Create indirect storage to reference an integer.
                struct Test { int m_dummy; };
                Test testStruct;

                auto & structNode = e.Immediate(&testStruct);
                auto & indirectNode = e.Deref(e.FieldPointer(structNode, &Test::m_dummy));

                auto storage = indirectNode.CodeGenValue(e);
                auto baseRegister = storage.GetBaseRegister();

                TestAssert(storage.ConvertToDirect(false).IsSameHardwareRegister(baseRegister));
            }


            // Verify that the sole owner will not reuse the base register in
            // cases when it's not possible (floating point value, reserved
            // base pointer, RIP relative addressing).

            TestCase(NoBaseRegisterReuseRIPRelative)
            {
                AutoResetAllocator reset(m_allocator);
                ExpressionNodeFactory e(m_allocator, *m_code);

                auto storage = e.RIPRelative<int>(16);
                TestAssert(storage.GetBaseRegister().IsRIP());

                TestAssert(!storage.ConvertToDirect(false).IsRIP());
            }


            TestCase(NoBaseRegisterReuseBasePointer)
            {
                AutoResetAllocator reset(m_allocator);
                ExpressionNodeFactory e(m_allocator, *m_code);

                auto storage = e.Temporary<int>();
                auto baseRegister = storage.GetBaseRegister();

                TestAssert(!storage.ConvertToDirect(false).IsSameHardwareRegister(baseRegister));
            }


            TestCase(NoBaseRegisterReuseFloat)
            {
                AutoResetAllocator reset(m_allocator);
                ExpressionNodeFactory e(m_allocator, *m_code);

                // Create indirect storage to reference a float.
                struct Test { float m_dummy; };
                Test testStruct;

                auto & structNode = e.Immediate(&testStruct);
                auto & indirectNode = e.Deref(e.FieldPointer(structNode, &Test::m_dummy));

                auto storage = indirectNode.CodeGenValue(e);
                auto baseRegister = storage.GetBaseRegister();

                TestAssert(!storage.ConvertToDirect(false).IsSameHardwareRegister(baseRegister));
            }


            // Make sure that expression tree allows multiple references to
            // the reserved base pointers.

            TestCase(MultipleReferencesToBasePointer)
            {
                AutoResetAllocator reset(m_allocator);
                ExpressionNodeFactory e(m_allocator, *m_code);

                auto temp1 = e.Temporary<int>();
                auto temp2 = e.Temporary<int>();

                TestAssert(temp1.GetBaseRegister() == temp2.GetBaseRegister());
                TestAssert(temp1.GetOffset() != temp2.GetOffset());
            }


            TestCase(MultipleReferencesToRIP)
            {
                AutoResetAllocator reset(m_allocator);
                ExpressionNodeFactory e(m_allocator, *m_code);

                auto temp1 = e.RIPRelative<int>(0);
                auto temp2 = e.RIPRelative<int>(16);

                TestAssert(temp1.GetBaseRegister().IsRIP());
                TestAssert(temp1.GetBaseRegister() == temp2.GetBaseRegister());
            }


        private:
            Allocator m_allocator;
            ExecutionBuffer m_executionBuffer;
            std::unique_ptr<FunctionBuffer> m_code;
        };
    }
}
