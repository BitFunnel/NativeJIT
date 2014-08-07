#include "stdafx.h"

#include "NativeJIT/JumpTable.h"


namespace NativeJIT
{
    JumpTable::JumpTable(int maxLabels, int maxCallSites)
    {
        m_labels.reserve(maxLabels);
        m_callSites.reserve(maxCallSites);
    }

    void JumpTable::Clear()
    {
        m_labels.clear();
        m_callSites.clear();
    }

    Label JumpTable::AllocateLabel()
    {
        // TODO: Code to ensure that this cast is always safe.
        Label label = Label(static_cast<unsigned>(m_labels.size()));
        m_labels.push_back(NULL);

        return label;
    }

    void JumpTable::PlaceLabel(Label label, const unsigned char* address)
    {
        // TODO: should we enable this code in RELEASE builds?
#ifdef _DEBUG
        if (LabelIsDefined(label))
        {
            throw std::runtime_error("CodeBuffer: attempting to place label more than once.");
        }
#endif

        m_labels[label.GetId()] = address;
    }


    void JumpTable::AddCallSite(Label label, unsigned char* site, unsigned size)
    {
        m_callSites.push_back(CallSite(label, size, site));
    }


    bool JumpTable::LabelIsDefined(Label label) const
    {
        return m_labels.at(label.GetId()) != NULL;
    }


    const unsigned char* JumpTable::AddressOfLabel(Label label) const
    {
        // TODO: should we enable this code in RELEASE builds?
#ifdef _DEBUG
        if (!LabelIsDefined(label))
        {
            throw std::runtime_error("CodeBuffer: attempting to use a label that hasn't been placed.");
        }
#endif

        return m_labels[label.GetId()];
    }


    // WARNING: Non portable. Assumes little endian machine architecture.
    // WARNING: Non portable. Assumes that fixup value is labelAddress - siteAddress - size.Size().
    void JumpTable::PatchCallSites()
    {
        for (int i=0; i < m_callSites.size(); ++i)
        {
            const CallSite& site = m_callSites[i];
            const unsigned char* labelAddress = AddressOfLabel(site.GetLabel());
            unsigned char* siteAddress = site.Site();
            size_t delta = labelAddress - siteAddress - site.Size();

            // TODO: Evaluate whether special cases for size == 2 and size == 4 actually improve performance.
            int size = site.Size();
            if (size == 2)
            {
                *((signed __int16*)siteAddress) = (signed __int16)delta;
                siteAddress += size;
            }
            else if (size == 4)
            {
                *((signed __int32*)siteAddress) = (signed __int32)delta;
                siteAddress += size;
            }
            else
            {
                while (size > 0)
                {
                    *siteAddress++ = (unsigned char)delta;
                    delta = delta >> 8;
                    size--;
                }
            }
        }
    }

    //*************************************************************************
    //
    // CallSite
    //
    //*************************************************************************
    CallSite::CallSite()
        : m_label(0)
    {
        m_site = NULL;
    }

    CallSite::CallSite(Label label, unsigned size, unsigned char* site)
        : m_label(label)
    {
        m_size = size;
        m_site = site;
    }

    Label CallSite::GetLabel() const
    {
        return m_label;
    }

    unsigned CallSite::Size() const
    {
        return m_size;
    }

    unsigned char* CallSite::Site() const
    {
        return m_site;
    }
}
