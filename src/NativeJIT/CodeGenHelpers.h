#pragma once

#include "ExpressionTree.h"                 // ExpressionTree::Storage<T> parameter.
#include "NativeJIT/X64CodeGenerator.h"     // Emit<OP> referenced by template definition.
#include "Temporary/Assert.h"               // Assert() referenced by template definition.


namespace NativeJIT
{
    namespace CodeGenHelpers
    {
        template <OpCode OP, unsigned SIZE, typename R>
        void Emit(X64CodeGenerator& code, Register<SIZE, false> left, ExpressionTree::Storage<R> right)
        {
            switch (right.GetStorageClass())
            {
            case StorageClass::Immediate:
                code.EmitImmediate<OP>(left, right.GetImmediate());
                break;
            case StorageClass::Direct:
                code.Emit<OP>(left, right.GetDirectRegister());
                break;
            case StorageClass::Indirect:
                code.Emit<OP>(left, right.GetBaseRegister(), right.GetOffset());
                break;
            default:
                Assert(false, "BinaryNode<L, R>::CodeGenOp: invalid storage class.");
            }
        }


        template <OpCode OP, unsigned SIZE, typename R>
        void Emit(X64CodeGenerator& code, Register<SIZE, true> left, ExpressionTree::Storage<R> right)
        {
            switch (right.GetStorageClass())
            {
            // NOTE: X64 floating point instructions do no support StorageClass::Immediate.
            case StorageClass::Direct:
                code.Emit<OP>(left, right.GetDirectRegister());
                break;
            case StorageClass::Indirect:
                code.Emit<OP>(left, right.GetBaseRegister(), right.GetOffset());
                break;
            default:
                Assert(false, "BinaryNode<L, R>::CodeGenOp: invalid storage class.");
            }
        }
    }
}
