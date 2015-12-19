#pragma once


#ifdef _MSC_VER
#include "FunctionBuffer_Windows.h"
namespace NativeJIT
{
    typedef Windows::FunctionBuffer FunctionBufferBase;
}
#else
#include "NativeJIT/FunctionBuffer_POSIX.h"
namespace NativeJIT
{
    typedef POSIX::FunctionBuffer FunctionBufferBase;
}
#endif

namespace NativeJIT
{
    class FunctionBuffer : public Windows::FunctionBuffer
    {
    public:
        FunctionBuffer::FunctionBuffer(Allocators::IAllocator& allocator,
                                       unsigned capacity,
                                       unsigned maxLabels,
                                       unsigned maxCallSites,
                                       unsigned slotCount,
                                       unsigned registerSaveMask,
                                       bool isLeaf)
            : FunctionBufferBase(allocator,
                                 capacity,
                                 maxLabels,
                                 maxCallSites,
                                 slotCount,
                                 registerSaveMask,
                                 isLeaf)
        {
        }
    };
}
