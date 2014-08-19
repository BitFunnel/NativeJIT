#pragma once

#include "ExpressionTree.h"                 // ExpressionTree::Storage<T> parameter.
#include "NativeJIT/X64CodeGenerator.h"
#include "Storage.h"
#include "Temporary/Assert.h"


namespace NativeJIT
{
    namespace CodeGenHelpers
    {
        template <OpCode OP, unsigned SIZE, bool ISFLOAT, typename R>
        void Emit(X64CodeGenerator& code, Register<SIZE, ISFLOAT> left, ExpressionTree::Storage<R> right)
        {
            switch (right.GetStorageClass())
            {
            case StorageClass::Immediate:
                code.Emit<OP>(left, right.GetImmediate());
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
    }
}
