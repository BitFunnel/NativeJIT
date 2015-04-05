#pragma once

#include <vector>

#include "Temporary/StlAllocator.h"


namespace NativeJIT
{
    class CallSite;


    class Label
    {
    public:
        Label();
        explicit Label(unsigned id);

        unsigned GetId() const;

    private:
        unsigned m_id;
    };


    // A JumpTable maintains collections of Labels and CallSites used in jump resolution.
    class JumpTable
    {
    public:
        // Allocator is used to allocate memory for labels and call sites.
        JumpTable(Allocators::IAllocator& allocator);

        void Clear();

        // Allocates a label that can be used as an argument to an X64 JCC/JMP/CALL instruction.
        Label AllocateLabel();

        // Binds a label to a memory address.
        void PlaceLabel(Label label, const unsigned __int8* address);

        // Records a position in memory containing call site (offset set portion of a JCC/JMP/CALL instruction)
        // that will need to be patch once all labels have been placed.
        void AddCallSite(Label label, unsigned __int8* site, unsigned size);

        // Patches each call site with the correct offset derived from its resolved label.
        void PatchCallSites();

        bool LabelIsDefined(Label label) const;
        const unsigned __int8* AddressOfLabel(Label label) const;

    private:
        template <typename T>
        using AllocatorVector = std::vector<T, Allocators::StlAllocator<T>>;

        Allocators::StlAllocator<void*> m_stlAllocator;     // STL adapter for IAllocator.
        AllocatorVector<const unsigned __int8*> m_labels;   // Storage for jump labels.
        AllocatorVector<CallSite> m_callSites;              // Call sites to be patched.
    };


    // Call sites represent positions in memory which will contain jump offsets.
    // These locations need to be patched after all label offsets are resolved.
    class CallSite
    {
    public:
        CallSite();
        CallSite(Label label, unsigned size, unsigned __int8* site);

        Label GetLabel() const;

        unsigned Size() const;
        unsigned __int8* Site() const;

    private:
        Label m_label;              // The jump label used to find the jump target address for this site.
        unsigned m_size;            // The number of bytes to be patched at this site.
        unsigned __int8* m_site;    // The address to be patched.
    };
}
