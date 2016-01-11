#include "stdafx.h"

#include <stdexcept>

#include "NativeJIT/CodeGen/JumpTable.h"


namespace NativeJIT
{
    const unsigned c_labelsReserveCount = 64;
    const unsigned c_callSitesReserveCount = 64;

    JumpTable::JumpTable(Allocators::IAllocator& allocator)
        : m_stlAllocator(allocator),
          m_labels(m_stlAllocator),
          m_callSites(m_stlAllocator)
    {
        m_labels.reserve(c_labelsReserveCount);
        m_callSites.reserve(c_callSitesReserveCount);
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


    void JumpTable::PlaceLabel(Label label, const uint8_t* address)
    {
        if (LabelIsDefined(label))
        {
            throw std::runtime_error("CodeBuffer: attempting to place label more than once.");
        }

        m_labels[label.GetId()] = address;
    }


    void JumpTable::AddCallSite(Label label, uint8_t* site, unsigned size)
    {
        m_callSites.push_back(CallSite(label, size, site));
    }


    bool JumpTable::LabelIsDefined(Label label) const
    {
        return m_labels.at(label.GetId()) != NULL;
    }


    const uint8_t* JumpTable::AddressOfLabel(Label label) const
    {
        if (!LabelIsDefined(label))
        {
            throw std::runtime_error("CodeBuffer: attempting to use a label that hasn't been placed.");
        }

        return m_labels[label.GetId()];
    }


    // WARNING: Non portable. Assumes little endian machine architecture.
    // WARNING: Non portable. Assumes that fixup value is labelAddress - siteAddress - size.Size().
    void JumpTable::PatchCallSites()
    {
        for (int i=0; i < m_callSites.size(); ++i)
        {
            const CallSite& site = m_callSites[i];
            const uint8_t* labelAddress = AddressOfLabel(site.GetLabel());
            uint8_t* siteAddress = site.Site();
            size_t delta = labelAddress - siteAddress - site.Size();

            // TODO: Evaluate whether special cases for size == 2 and size == 4 actually improve performance.
            int size = site.Size();
            if (size == 2)
            {
                *((int16_t*)siteAddress) = (int16_t)delta;
                siteAddress += size;
            }
            else if (size == 4)
            {
                *((int32_t*)siteAddress) = (int32_t)delta;
                siteAddress += size;
            }
            else
            {
                while (size > 0)
                {
                    *siteAddress++ = (uint8_t)delta;
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

    CallSite::CallSite(Label label, unsigned size, uint8_t* site)
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

    uint8_t* CallSite::Site() const
    {
        return m_site;
    }
}
