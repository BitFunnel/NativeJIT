#pragma once

#include "IAllocator.h"


namespace Allocators
{
    // Allocates memory from the allocator to store an object of type T and
    // performs in-place construction of T.
    template <typename T, typename... ConstructorArgs>
    T&
    PlacementConstruct(IAllocator& allocator, ConstructorArgs&&... constructorArgs)
    {
        // Obtain the memory needed to store the object itself.
        // Note: Assumes that IAllocator's Allocate() method returns memory
        // aligned to satisfy all object types.
        void* memory = allocator.Allocate(sizeof(T));

        try
        {
            // In-place construct the object, perfectly forwarding the
            // constructor arguments.
            new (memory) T(std::forward<ConstructorArgs>(constructorArgs)...);
        }
        catch(...)
        {
            allocator.Deallocate(memory);
            throw;
        }

        return *reinterpret_cast<T*>(memory);
    }
}
