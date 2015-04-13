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

                storage.ConvertToValue(e, false);
                TestAssert(storage.GetDirectRegister().IsSameHardwareRegister(baseRegister));
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

                storage.ConvertToValue(e, false);
                TestAssert(!storage.GetDirectRegister().IsRIP());
            }


            TestCase(NoBaseRegisterReuseBasePointer)
            {
                AutoResetAllocator reset(m_allocator);
                ExpressionNodeFactory e(m_allocator, *m_code);

                auto storage = e.Temporary<int>();
                auto baseRegister = storage.GetBaseRegister();

                storage.ConvertToValue(e, false);
                TestAssert(!storage.GetDirectRegister().IsSameHardwareRegister(baseRegister));
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

                storage.ConvertToValue(e, false);
                TestAssert(!storage.GetDirectRegister().IsSameHardwareRegister(baseRegister));
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
