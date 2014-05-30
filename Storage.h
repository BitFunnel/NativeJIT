#pragma once

#include <iostream>     // TODO: Delete this file after removing all references to std::cout.

#include "Allocator.h"
#include "Assert.h"
#include "ExpressionTree.h"
#include "Register.h"
#include "X64CodeGenerator.h"


namespace Allocators
{
    class IAllocator;
}


namespace NativeJIT
{
    //// Used to determine if the base register can be reused to hold the value.
    //template <unsigned SIZE1, bool ISFLOAT1, unsigned SIZE2, bool ISFLOAT2>
    //bool CompatibleRegisters(Register<SIZE1, ISFLOAT1> r1, Register<SIZE2, ISFLOAT2> r2)
    //{
    //    return ISFLOAT1 == ISFLOAT2;
    //}

    class ExpressionTree;

    template <typename T>
    class Storage
    {
    public:
        typedef Register<sizeof(T), IsFloatingPointType<T>::value/*, IsSigned<T>::value*/> DirectRegister;
        typedef Register<sizeof(T*), false/*, false*/> BaseRegister;


        Storage(ExpressionTree& tree, T immediate);
        Storage(ExpressionTree& tree, DirectRegister r);
        Storage(ExpressionTree& tree, BaseRegister r, size_t offset);

        Storage(Storage& other);

        ~Storage();

        Storage& operator=(Storage & other);


        // Not sure there is any case where we would take ownership without converting to value.
        //// If reference count greater than one
        ////   Allocate new Storage::Data
        ////   Generate code to copy values from old storage to new storage.
        ////   Release old storage.
        ////   Set m_data to point to new Storage::Data.
        //// Otherwise
        ////   Do nothing - ownership is implicit.
        //void TakeOwnership(ExpressionTree& tree);

        // Case: Immediate
        //   Allocate new Storage::Data Direct.
        //   Generate code to load immediate.
        //   Release old storage.
        //   Set m_data.
        // Case: Direct
        //   TakeOwnership
        // Case: Indirect
        //   If base register can hold direct value
        //     TakeOwnership
        //     Generate code to load indirect from base to direct.
        //     Change type to direct.
        //   Else
        //     Allocate new Storage:Data Direct
        //     Generate code to load indirect from base to new direct.
        //     Release old storage.
        //     Set m_data.
        void ConvertToValue(ExpressionTree& tree, bool forModification);


        // Transfer the contents and reference count to the new storage location. Take ownership
        // of the current storage. Scenario is spilling register or cache register.
        void SwapWith(Storage other);


        typedef enum {Immediate, Indirect, Direct} Class;

        Class GetClass() const;


        T GetImmedidate() const;


        BaseRegister GetBaseRegister() const;
        size_t GetOffset() const;


        DirectRegister GetDirectRegister() const;

    private:
        class Data
        {
        public:
            Data(ExpressionTree& tree, T immediate);
            Data(ExpressionTree& tree, DirectRegister r);
            Data(ExpressionTree& tree, BaseRegister base, size_t offset);

            unsigned m_refCount;

            ExpressionTree& m_tree;
            Class m_storageClass;

            T m_immediate;
            unsigned m_registerId;
            size_t m_offset;
        };

        void SetData(Data* data);

        Data* m_data;
    };


    //*************************************************************************
    //
    // Template definitions for Storage.
    //
    //*************************************************************************
    template <typename T>
    Storage<T>::Storage(ExpressionTree& tree, T immediate)
        : m_data(nullptr)
    {
        SetData(new (tree.GetAllocator().Allocate(sizeof(Data))) Data(tree, immediate));
    }


    template <typename T>
    Storage<T>::Storage(ExpressionTree& tree, DirectRegister r)
        : m_data(nullptr)
    {
        SetData(new (tree.GetAllocator().Allocate(sizeof(Data))) Data(tree, r));
    }


    template <typename T>
    Storage<T>::Storage(ExpressionTree& tree, BaseRegister r, size_t offset)
        : m_data(nullptr)
    {
        SetData(new (tree.GetAllocator().Allocate(sizeof(Data))) Data(tree, r, offset));
    }



    template <typename T>
    Storage<T>::Storage(Storage& other)
        : m_data(nullptr)
    {
        SetData(other.m_data);
    }


    template <typename T>
    Storage<T>::~Storage()
    {
        SetData(nullptr);
    }


    template <typename T>
    Storage<T>& Storage<T>::operator=(Storage & other)
    {
        SetData(other.m_data);
        return *this;
    }


    //void Storage<T>::TakeOwnership(ExpressionTree& tree);


    template <typename T>
    void Storage<T>::ConvertToValue(ExpressionTree& tree, bool forModification)
    {
        if (m_data->m_refCount == 1)
        {
            // We are the sole owner of this storage and can repurpose it.
            switch (m_data->m_storageClass)
            {
            case Immediate:
                {
                    // Allocate a register and load this value into the register.
                    auto dest = m_data->m_tree.AllocateRegister<DirectRegister>();
                    m_data->m_tree.GetCodeGenerator().Op("mov", dest, m_data->m_immediate);

                    m_data->m_storageClass = Direct;
                    m_data->m_registerId = dest.GetId();
                }
                break;
            case Direct:
                // This storage is already a direct value. No work required.
                break;
            case Indirect:
                {
                    BaseRegister base = BaseRegister(m_data->m_registerId);
                    if (IsFloatingPointType<T>::value || m_data->m_tree.IsBasePointer(base))
                    {
                        // We need to allocate a new register for the value for one of two reasons:
                        //   The value is float and therefore cannot be stored in base.
                        //   Base is the reserved base pointer register in the tree.
                        DirectRegister dest = m_data->m_tree.AllocateRegister<DirectRegister>();
                        m_data->m_tree.GetCodeGenerator().Op("mov", dest, base, m_data->m_offset);

                        m_data->m_tree.ReleaseRegister(base);
                        m_data->m_registerId = dest.GetId();
                        m_data->m_storageClass = Direct;
                    }
                    else
                    {
                        // We can reuse the base register for the value.
                        DirectRegister dest(m_data->m_registerId);
                        BaseRegister base = BaseRegister(m_data->m_registerId);
                        m_data->m_tree.GetCodeGenerator().Op("mov", dest, base, m_data->m_offset);
                        m_data->m_storageClass = Direct;
                    }
                }
                break;
            default:
                Assert(false, "ConvertToValue: invalid storage class.");
            }
        }
        else
        {
            // We do not own this storage and therefore must allocate new storage.
            // The main case for not owning the storage is when it represents the cached value
            // of a common subexpression. WARNING: Note that holding an extra local copy of the
            // storage will artificially increase the ref count.
            switch (m_data->m_storageClass)
            {
            case Immediate:
                {
                    // Allocate a register and load this value into the register.
                    auto dest = m_data->m_tree.AllocateRegister<DirectRegister>();
                    m_data->m_tree.GetCodeGenerator().Op("mov", dest, m_data->m_immediate);

                    SetData(new (tree.GetAllocator().Allocate(sizeof(Data))) Data(tree, dest));
                }
                break;
            case Direct:
                if (forModification)
                {
                    // We need to allocate a new register for the value.
                    DirectRegister dest = m_data->m_tree.AllocateRegister<DirectRegister>();
                    DirectRegister src = DirectRegister(m_data->m_registerId);
                    m_data->m_tree.GetCodeGenerator().Op("mov", dest, src);

                    SetData(new (tree.GetAllocator().Allocate(sizeof(Data))) Data(tree, dest));
                }
                break;
            case Indirect:
                {
                    // We need to allocate a new register for the value.
                    DirectRegister dest = m_data->m_tree.AllocateRegister<DirectRegister>();
                    BaseRegister base = BaseRegister(m_data->m_registerId);
                    m_data->m_tree.GetCodeGenerator().Op("mov", dest, base, m_data->m_offset);

                    SetData(new (tree.GetAllocator().Allocate(sizeof(Data))) Data(tree, dest));
                }
                break;
            default:
                Assert(false, "ConvertToValue: invalid storage class.");
            }
        }
    }


    //void Storage<T>::SwapWith(Storage other);



    template <typename T>
    typename Storage<T>::Class Storage<T>::GetClass() const
    {
        return m_data->m_storageClass;
    }


    template <typename T>
    T Storage<T>::GetImmedidate() const
    {
        Assert(m_data->m_storageClass == Immediate, "GetImmediate(): storage class must be immediate.");
        return m_data->m_immediate;
    }


    template <typename T>
    typename Storage<T>::BaseRegister Storage<T>::GetBaseRegister() const
    {
        Assert(m_data->m_storageClass == Indirect, "GetBaseRegister(): storage class must be indirect.");
        return BaseRegister(m_data->m_registerId());
    }


    template <typename T>
    size_t Storage<T>::GetOffset() const
    {
        Assert(m_data->m_storageClass == Indirect, "GetOffset(): storage class must be indirect.");
        return m_data->m_offset;
    }


    template <typename T>
    typename Storage<T>::DirectRegister Storage<T>::GetDirectRegister() const
    {
        Assert(m_data->m_storageClass == Direct, "GetDirectRegister(): storage class must be direct.");
        return DirectRegister(m_data->m_registerId());
    }


    template <typename T>
    void Storage<T>::SetData(Data* data)
    {
        // If the new data is different from the existing data ...
        if (m_data != data)
        {
            // If there is existing data, decrement the ref count and release resources if zero.
            if (m_data != nullptr)
            {
                Assert(m_data->m_refCount > 0, "Attempting to decrement zero ref count.");
                std::cout << "Decrement " << m_data << " to " << m_data->m_refCount - 1 << std::endl;
                if (--m_data->m_refCount == 0)
                {
                    if (m_data->m_storageClass == Class::Direct)
                    {
                        std::cout << "Release Direct Register" << DirectRegister(m_data->m_registerId).GetName() << std::endl;
                        m_data->m_tree.ReleaseRegister(DirectRegister(m_data->m_registerId));
                    }
                    else if (m_data->m_storageClass == Class::Indirect)
                    {
                        std::cout << "Release Base Register " << BaseRegister(m_data->m_registerId).GetName() << std::endl;
                        m_data->m_tree.ReleaseRegister(BaseRegister(m_data->m_registerId));
                    }
                }
            }

            // Set the new data ptr and increment its ref count if applicable.
            m_data = data;
            if (m_data != nullptr)
            {
                ++m_data->m_refCount;
                std::cout << "Increment " << m_data << " to " << m_data->m_refCount << std::endl;
            }
        }
    }

    //*************************************************************************
    //
    // Template definitions for Storage::Data
    //
    //*************************************************************************
    template <typename T>
    Storage<T>::Data::Data(ExpressionTree& tree, T immediate)
        : m_refCount(0),
          m_tree(tree),
          m_storageClass(Immediate),
          m_immediate(immediate)
    {
    }


    template <typename T>
    Storage<T>::Data::Data(ExpressionTree& tree, DirectRegister r)
        : m_refCount(0),
          m_tree(tree),
          m_storageClass(Direct),
          m_registerId(r.GetId())
    {
    }


    template <typename T>
    Storage<T>::Data::Data(ExpressionTree& tree, BaseRegister r, size_t offset)
        : m_refCount(0),
          m_tree(tree),
          m_storageClass(Indirect),
          m_registerId(r.GetId()),
          m_offset(offset)
    {
    }
}
