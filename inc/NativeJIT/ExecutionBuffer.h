#pragma once


#ifdef _MSC_VER
#include "ExecutionBuffer_Windows.h"
namespace NativeJIT
{
    typedef Windows::ExecutionBuffer ExecutionBufferBase;
}
#else
#include "NativeJIT/ExecutionBuffer_POSIX.h"
namespace NativeJIT
{
    typedef POSIX::ExecutionBuffer ExecutionBufferBase;
}
#endif


namespace NativeJIT
{
    class ExecutionBuffer : public Windows::ExecutionBuffer
    {
    public:
        ExecutionBuffer(size_t bufferSize)
            : ExecutionBufferBase(bufferSize)
        {
        }
    };
}
