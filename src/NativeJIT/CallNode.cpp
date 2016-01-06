#include "stdafx.h"

#include "NativeJIT/BitOperations.h"
#include "NativeJIT/CodeGen/CallingConvention.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/Nodes/CallNode.h"


namespace NativeJIT
{
    //*************************************************************************
    //
    // SaveRestoreVolatilesHelper
    //
    //*************************************************************************
    SaveRestoreVolatilesHelper::SaveRestoreVolatilesHelper(Allocators::IAllocator& allocator)
        : m_rxxCallExclusiveRegisterMask(0),
          m_xmmCallExclusiveRegisterMask(0),
          m_preservationStorage(Allocators::StlAllocator<void*>(allocator))
    {
        m_preservationStorage.reserve(RegisterBase::c_maxIntegerRegisterID + 1
                                      + RegisterBase::c_maxIntegerRegisterID + 1);
    }


    template<>
    unsigned SaveRestoreVolatilesHelper::GetRegistersToPreserve<false>(ExpressionTree& tree) const
    {
        // Save all used volatiles, except those used in and *fully* owned by
        // the call itself.
        return CallingConvention::c_rxxVolatileRegistersMask
               & CallingConvention::c_rxxWritableRegistersMask
               & tree.GetRXXUsedMask()
               & ~m_rxxCallExclusiveRegisterMask;
    }


    template<>
    unsigned SaveRestoreVolatilesHelper::GetRegistersToPreserve<true>(ExpressionTree& tree) const
    {
        // Save all used volatiles, except those used in and *fully* owned by
        // the call itself.
        return CallingConvention::c_xmmVolatileRegistersMask
               & CallingConvention::c_xmmWritableRegistersMask
               & tree.GetXMMUsedMask()
               & ~m_xmmCallExclusiveRegisterMask;
    }


    void SaveRestoreVolatilesHelper::SaveVolatiles(ExpressionTree& tree)
    {
        auto & code = tree.GetCodeGenerator();

        unsigned rxxVolatiles = GetRegistersToPreserve<false>(tree);

        unsigned r = 0;
        while (BitOp::GetLowestBitSet(rxxVolatiles, &r))
        {
            m_preservationStorage.push_back(tree.Temporary<void*>());
            auto const & s = m_preservationStorage.back();

            code.Emit<OpCode::Mov>(s.GetBaseRegister(),
                                   s.GetOffset(),
                                   Register<8, false>(r));

            BitOp::ClearBit(&rxxVolatiles, r);
        }

        unsigned xmmVolatiles = GetRegistersToPreserve<true>(tree);

        while (BitOp::GetLowestBitSet(xmmVolatiles, &r))
        {
            // TODO: This preserves only the lower 64 bits of the XMM register.
            // That is currently fine as NativeJIT only uses floats and doubles
            // which don't overlap with the upper 64-bits. Also, the full 128
            // bits of XMM nonvolatiles are preserved in the function prolog
            // by FunctionBuffer. To be fully correct, this should preserve
            // all 128 bits (f. ex. allow Temporary() to return 16-byte
            // aligned 16-byte space and use movaps to save the whole register).
            // RestoreVolatiles() needs to be modified accordingly as well.
            m_preservationStorage.push_back(tree.Temporary<void*>());
            auto const & s = m_preservationStorage.back();

            code.Emit<OpCode::Mov>(s.GetBaseRegister(),
                                   s.GetOffset(),
                                   Register<8, true>(r));


            BitOp::ClearBit(&xmmVolatiles, r);
        }
    }


    void SaveRestoreVolatilesHelper::RestoreVolatiles(ExpressionTree& tree)
    {
        auto & code = tree.GetCodeGenerator();

        // Do everything in the reverse order, including popping XMM registers first.
        unsigned xmmVolatiles = GetRegistersToPreserve<true>(tree);

        unsigned r = 0;
        while (BitOp::GetHighestBitSet(xmmVolatiles, &r))
        {
            Assert(!m_preservationStorage.empty(), "Logic error");
            auto const & s = m_preservationStorage.back();

            code.Emit<OpCode::Mov>(Register<8, true>(r),
                                   s.GetBaseRegister(),
                                   s.GetOffset());

            m_preservationStorage.pop_back();
            BitOp::ClearBit(&xmmVolatiles, r);
        }

        unsigned rxxVolatiles = GetRegistersToPreserve<false>(tree);

        while (BitOp::GetHighestBitSet(rxxVolatiles, &r))
        {
            Assert(!m_preservationStorage.empty(), "Logic error");
            auto const & s = m_preservationStorage.back();

            code.Emit<OpCode::Mov>(Register<8, false>(r),
                                   s.GetBaseRegister(),
                                   s.GetOffset());

            m_preservationStorage.pop_back();
            BitOp::ClearBit(&rxxVolatiles, r);
        }
    }
}
