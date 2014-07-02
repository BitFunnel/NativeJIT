// CodeGenUnitTest.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "SuiteCpp/UnitTest.h"

namespace NativeJIT
{
    namespace CodeGenUnitTest
    {
        TestCase(SampleTest)
        {
            TestAssert(true, "Test passed.");
        }
    }
}

