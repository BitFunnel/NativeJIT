#pragma once

#include <vector>


namespace NativeJIT
{
    class CallSite;


    //// TODO: Signed or unsigned? X64CodeGenerator version uses signed to
    //// distinguish between local and global labels.
    //typedef unsigned Label;
    class Label
    {
    public:
        Label();
        Label(unsigned id);

        unsigned GetId() const;

    private:
        unsigned m_id;
    };


    // A JumpTable maintains collections of Labels and CallSites used in jump resolution.
    class JumpTable
    {
    public:
        JumpTable(int maxLabels, int maxCallSites);

        void Clear();

        // Allocates a label that can be used as an argument to an X64 JCC/JMP/CALL instruction.
        Label AllocateLabel();

        // Binds a label to a memory address.
        void PlaceLabel(Label label, const unsigned char* address);

        // Records a position in memory containing call site (offset set portion of a JCC/JMP/CALL instruction)
        // that will need to be patch once all labels have been placed.
        void AddCallSite(Label label, unsigned char* site, int size);

        // Patches each call site with the correct offset derived from its resolved label.
        void PatchCallSites();

        bool LabelIsDefined(Label label) const;
        const unsigned char* AddressOfLabel(Label label) const;

    private:
        std::vector<const unsigned char*> m_labels;     // Storage for jump labels.
        std::vector<CallSite> m_callSites;              // Call sites to be patched.
    };


    // Call sites represent positions in memory which will contain jump offsets.
    // These locations need to be patched after all label offsets are resolved.
    class CallSite
    {
    public:
        CallSite();
        CallSite(Label label, int size, unsigned char* site);

        Label GetLabel() const;

        // TODO: X64CodeGenerator version had return type as Label. This is a bug and should be fixed in X64CodeGenerator.
        // Also fix and throw new Foo in X64CodeGenerator.
        // Also, some files in X64CodeGenerator missing #includes for <ostream>, possibly other standard libaries.
        unsigned Size() const;
        unsigned char* Site() const;

    private:
        Label m_label;          // The jump label used to find the jump target address for this site.
        unsigned m_size;        // The number of bytes to be patched at this site.
        unsigned char* m_site;  // The address to be patched.
    };
}
