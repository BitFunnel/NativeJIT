#include "stdafx.h"

#include "ExpressionNodeFactory2.h"


namespace NativeJIT
{
    ExpressionNodeFactory2::ExpressionNodeFactory2(Allocators::IAllocator& allocator,
                                                   FunctionBuffer& code)
        : ExpressionTree(allocator, code)
    {
    }
}
