#pragma once

#include "Assert.h"
#include "Storage.h"
#include "X64CodeGenerator.h"


namespace NativeJIT
{
    namespace CodeGenHelpers
    {
        template <unsigned SIZE, bool ISFLOAT, typename R>
        void Emit(X64CodeGenerator& code, char const * operation, Register<SIZE, ISFLOAT> left, Storage<R> right)
        {
            switch (right.GetClass())
            {
            case Data::Immediate:
                code.Op(operation, left, right.GetImmediate());
                break;
            case Data::Direct:
                code.Op(operation, left, right.GetDirectRegister());
                break;
            case Data::Indirect:
                code.Op(operation, left, right.GetBaseRegister(), right.GetOffset());
                break;
            default:
                Assert(false, "BinaryNode<L, R>::CodeGenOp: invalid storage class.");
            }
        }
    }
}
