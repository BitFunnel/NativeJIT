#include "stdafx.h"

#include "NativeJIT/ExpressionNodeFactory.h"


namespace NativeJIT
{
    ExpressionNodeFactory::ExpressionNodeFactory(Allocators::IAllocator& allocator,
                                                 FunctionBuffer& code)
        : ExpressionTree(allocator, code)
    {
    }
}
