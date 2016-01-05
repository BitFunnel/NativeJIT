#pragma once

#include <vector>

#include "Temporary/StlAllocator.h"


namespace NativeJIT
{
    // An alias for a vector which is using StlAllocator to allocate memory.
    template <typename T>
    using AllocatorVector = std::vector<T, Allocators::StlAllocator<T>>;
}
