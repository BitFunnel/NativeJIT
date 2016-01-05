#pragma once

#include <cstdint>

#include "NativeJIT/AllocatorVector.h" // Embedded member.


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
        void PlaceLabel(Label label, const uint8_t* address);

        // Records a position in memory containing call site (offset set portion of a JCC/JMP/CALL instruction)
        // that will need to be patch once all labels have been placed.
        void AddCallSite(Label label, uint8_t* site, unsigned size);

        // Patches each call site with the correct offset derived from its resolved label.
        void PatchCallSites();

        bool LabelIsDefined(Label label) const;
        const uint8_t* AddressOfLabel(Label label) const;

    private:
        Allocators::StlAllocator<void*> m_stlAllocator; // STL adapter for IAllocator.
        AllocatorVector<const uint8_t*> m_labels;       // Storage for jump labels.
        AllocatorVector<CallSite> m_callSites;          // Call sites to be patched.
    };


    // Call sites represent positions in memory which will contain jump offsets.
    // These locations need to be patched after all label offsets are resolved.
    class CallSite
    {
    public:
        CallSite();
        CallSite(Label label, unsigned size, uint8_t* site);

        Label GetLabel() const;

        unsigned Size() const;
        uint8_t* Site() const;

    private:
        Label m_label;              // The jump label used to find the jump target address for this site.
        unsigned m_size;            // The number of bytes to be patched at this site.
        uint8_t* m_site;    // The address to be patched.
    };
}
