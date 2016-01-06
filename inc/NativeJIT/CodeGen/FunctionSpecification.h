#pragma once

#include <cstdint>

#include "NativeJIT/AllocatorVector.h"      // Embedded member.

#ifdef _MSC_VER
#include "NativeJIT/CodeGen/Windows/FunctionSpecification.h"
#else
#include "NativeJIT/CodeGen/POSIX/FunctionSpecification.h"
#endif


namespace NativeJIT
{
    class FunctionSpecification : public FunctionSpecificationBase
    {
    public:
        // TODO:
        // http://stackoverflow.com/questions/3119929/forwarding-all-constructors-in-c0x
        // https://alfps.wordpress.com/2010/06/05/cppx-w4-no-warnings-part-ii-disabling-msvc-sillywarnings/
        // Using directive for constructors does not work in VS2013.
        // using FunctionSpecificationBase::FunctionSpecificationBase;

        FunctionSpecification(Allocators::IAllocator& allocator,
                              int maxFunctionCallParameters,
                              unsigned localStackSlotCount,
                              unsigned savedRxxNonvolatilesMask,
                              unsigned savedXmmNonvolatilesMask,
                              BaseRegisterType baseRegisterType,
                              std::ostream* diagnosticStream)
            : FunctionSpecificationBase(allocator,
                                        maxFunctionCallParameters,
                                        localStackSlotCount,
                                        savedRxxNonvolatilesMask,
                                        savedXmmNonvolatilesMask,
                                        baseRegisterType,
                                        diagnosticStream)
        {
        }
    };
}

