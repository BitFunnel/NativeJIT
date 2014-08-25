#include "stdafx.h"

#include "CallNode2.h"
#include "NativeJIT/FunctionBuffer.h"


namespace NativeJIT
{
    //*************************************************************************
    //
    // SaveRestoreVolatilesHelper
    //
    //*************************************************************************
    SaveRestoreVolatilesHelper::SaveRestoreVolatilesHelper()
        : m_rxxParameters(0)
    {
    }


    void SaveRestoreVolatilesHelper::SaveVolatiles(ExpressionTree& tree)
    {
        // Save those volatiles that are not staged for parameters, not the
        // return value, and are actually in use in the tree.
        unsigned rxxVolatiles = c_rxxVolatiles & ~m_rxxParameters & ~1ul & tree.GetRXXUsageMask();
        unsigned r = 0;
        while (rxxVolatiles != 0)
        {
            if ((rxxVolatiles & 1) != 0)
            {
                tree.GetCodeGenerator().Emit<OpCode::Push>(Register<8, false>(r));
            }
            ++r;
            rxxVolatiles >>= 1;
        }
    }


    void SaveRestoreVolatilesHelper::RestoreVolatiles(ExpressionTree& tree)
    {
        // Save those volatiles that are not staged for parameters, not the
        // return value, and are actually in use in the tree.
        unsigned rxxVolatiles = c_rxxVolatiles & ~m_rxxParameters & ~1ul & tree.GetRXXUsageMask();
        rxxVolatiles <<= 16;
        unsigned r = 15;
        while (rxxVolatiles != 0)
        {
            if ((rxxVolatiles & 0x80000000) != 0)
            {
                tree.GetCodeGenerator().Emit<OpCode::Pop>(Register<8, false>(r));
            }
            --r;
            rxxVolatiles <<= 1;
        }
    }
}
