// CodeGenUnitTest.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "ExecutionBuffer.h"
#include "FunctionBuffer.h"
#include "SuiteCpp/UnitTest.h"

namespace NativeJIT
{
    namespace CodeGenUnitTest
    {
        TestCase(SampleTest)
        {
            TestAssert(true, "Test passed.");
        }


        TestCase(ExecutionBuffer)
        {
            ExecutionBuffer allocator(1000);
            FunctionBufferBase buffer(allocator, 200, 3, 0, false);
        }
    }
}

