#include "stdafx.h"

#include "CallNode.h"
#include "NativeJIT/BitOperations.h"
#include "NativeJIT/FunctionBuffer.h"


namespace NativeJIT
{
    //*************************************************************************
    //
    // SaveRestoreVolatilesHelper
    //
    //*************************************************************************
    SaveRestoreVolatilesHelper::SaveRestoreVolatilesHelper()
        : m_rxxCallExclusiveRegisterMask(0),
          m_xmmCallExclusiveRegisterMask(0)
    {
    }


    template<>
    unsigned SaveRestoreVolatilesHelper::GetRegistersToPreserve<false>(ExpressionTree& tree) const
    {
        // Save all used volatiles, except those used in and *fully* owned by
        // the call itself.
        return c_rxxVolatiles
               & tree.GetRXXUsageMask()
               & ~m_rxxCallExclusiveRegisterMask;
    }


    template<>
    unsigned SaveRestoreVolatilesHelper::GetRegistersToPreserve<true>(ExpressionTree& tree) const
    {
        // Save all used volatiles, except those used in and *fully* owned by
        // the call itself.
        return c_xmmVolatiles
               & tree.GetXMMUsageMask()
               & ~m_xmmCallExclusiveRegisterMask;
    }


    void SaveRestoreVolatilesHelper::SaveVolatiles(ExpressionTree& tree,
                                                   unsigned parameterCount)
    {
        auto & code = tree.GetCodeGenerator();

        unsigned rxxVolatiles = GetRegistersToPreserve<false>(tree);

        unsigned r = 0;
        while (BitOp::GetLowestBitSet(rxxVolatiles, &r))
        {
            code.Emit<OpCode::Push>(Register<8, false>(r));
            BitOp::ClearBit(&rxxVolatiles, r);
        }

        unsigned xmmVolatiles = GetRegistersToPreserve<true>(tree);

        while (BitOp::GetLowestBitSet(xmmVolatiles, &r))
        {
            // Simulate a push for FP register until the TODO below is resolved.
            code.EmitImmediate<OpCode::Sub>(rsp, 8);
            code.Emit<OpCode::Mov>(rsp, 0, Register<8, true>(r));
            BitOp::ClearBit(&xmmVolatiles, r);
        }

        // Allocate backing storage for parameter homes.
        //
        // TODO: This should not be done here, X64 calling convention assumes
        // that RSP is changed only in the prolog of a function to allocate
        // space for maximum parameters used in any CALLs it makes and for
        // temporaries. Besides that, it should not be touched (except by the
        // implicit PUSH/POP of the return address by CALL).
        // The code above may leave the stack non 16-bytes aligned.
        // Also review/adjust the related code in RestoreVolatiles().
        if (parameterCount > 0)
        {
            code.EmitImmediate<OpCode::Sub>(
                rsp,
                static_cast<unsigned __int8>(sizeof(void*) * parameterCount));
        }
    }


    void SaveRestoreVolatilesHelper::RestoreVolatiles(ExpressionTree& tree,
                                                      unsigned parameterCount)
    {
        auto & code = tree.GetCodeGenerator();

        // Do everything in the reverse order, including popping XMM registers first.
        if (parameterCount > 0)
        {
            code.EmitImmediate<OpCode::Add>(
                rsp,
                static_cast<unsigned __int8>(8 * parameterCount));
        }

        unsigned xmmVolatiles = GetRegistersToPreserve<true>(tree);

        unsigned r = 0;
        while (BitOp::GetHighestBitSet(xmmVolatiles, &r))
        {
            // Simulate a pop for FP register.
            code.Emit<OpCode::Mov>(Register<8, true>(r), rsp, 0);
            code.EmitImmediate<OpCode::Add>(rsp, 8);
            BitOp::ClearBit(&xmmVolatiles, r);
        }

        unsigned rxxVolatiles = GetRegistersToPreserve<false>(tree);

        while (BitOp::GetHighestBitSet(rxxVolatiles, &r))
        {
            code.Emit<OpCode::Pop>(Register<8, false>(r));
            BitOp::ClearBit(&rxxVolatiles, r);
        }
    }
}
