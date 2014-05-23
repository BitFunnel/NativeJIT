#pragma once

namespace Allocators
{
    //*************************************************************************
    //
    // IAllocator is an abstract base class or interface for classes that are
    // memory allocators. Classes that implement IAllocator may use any
    // allocation strategy and may choose whether or not to be threadsafe.
    //
    //*************************************************************************
    class IAllocator
    {
    public:
        virtual ~IAllocator() {};

        // Allocates a block of a specified byte size.
        virtual void* Allocate(size_t size) = 0;

        // Frees a block.
        virtual void Deallocate(void* block) = 0;

        // Returns the maximum legal allocation size in bytes.
        virtual size_t MaxSize() const = 0;

        // Frees all blocks that have been allocated since construction or the
        // last call to Reset().
        virtual void Reset() = 0;
    };
}