#pragma once

#include <iostream>             // TODO: Delete this file after removing all references to std::cout.

#include "ExpressionTree.h"
#include "NativeJIT/Register.h"
#include "NativeJIT/X64CodeGenerator.h"
#include "Temporary/Allocator.h"
#include "Temporary/Assert.h"
#include "Temporary/NonCopyable.h"
#include "TypePredicates.h"


namespace Allocators
{
    class IAllocator;
}


namespace NativeJIT
{
    class ExpressionTree;

    // DESIGN NOTE: Class Data is not embedded inside of Storage because Data
    // objects are shared by Storage object parameterized by different types.
    class Data : public NonCopyable
    {
    public:
        typedef enum {Immediate, Indirect, Direct} Class;

        template <typename T>
        Data(ExpressionTree& tree, T immediate)
            : m_refCount(0),
              m_tree(tree),
              m_storageClass(Immediate)
        {
            static_assert(sizeof(T) <= sizeof(m_immediate2), "Unsupported type.");
            *reinterpret_cast<T*>(&m_immediate2) = immediate;
        }


        template <unsigned SIZE, bool ISFLOAT>
        Data(ExpressionTree& tree, Register<SIZE, ISFLOAT> r)
            : m_refCount(0),
              m_tree(tree),
              m_storageClass(Direct),
              m_registerId(r.GetId())
        {
        }


        Data(ExpressionTree& tree, Register<sizeof(void*), false> base, size_t offset);


        template <typename T>
        T GetImmediate() const
        {
            static_assert(sizeof(T) <= sizeof(m_immediate2), "Unsupported type.");
            return *(reinterpret_cast<T*>(&m_immediate2));
        }


        Data* ConvertToIndirect(unsigned registerId, size_t offset)
        {
            m_storageClass = Indirect;
            m_registerId = registerId;
            m_offset = offset;
            return this;
        }

    private:
        template <typename OTHER> friend class Storage;

        unsigned m_refCount;

        ExpressionTree& m_tree;
        Class m_storageClass;

        unsigned m_registerId;
        size_t m_offset;

        // TODO: Comment why this parameter is mutable.
        mutable size_t m_immediate2;
    };


    template <typename T>
    class Storage
    {
    public:
        typedef Register<sizeof(T), IsFloatingPointType<T>::value/*, IsSigned<T>::value*/> DirectRegister;
        typedef Register<sizeof(T*), false/*, false*/> BaseRegister;

        Storage();
        Storage(ExpressionTree& tree, T immediate);
        Storage(ExpressionTree& tree, DirectRegister r);
        Storage(ExpressionTree& tree, BaseRegister r, size_t offset);

        Storage(ExpressionTree& tree, Storage<T*>& base, size_t offset);

        template <typename U>
        Storage(ExpressionTree& /*tree*/, Storage<U> other)
            : m_data(nullptr)
        {
            SetData(other.m_data);
        }


        Storage(Storage const & other);

        ~Storage();

        Storage& operator=(Storage const & other);

        bool IsNull() const;

        void Reset();


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

        void Spill(ExpressionTree& tree);

        Data::Class GetClass() const;

        T GetImmediate() const;

        BaseRegister GetBaseRegister() const;
        size_t GetOffset() const;

        DirectRegister GetDirectRegister() const;

    private:
        template <typename OTHER> friend class Storage;

        void SetData(Data* data);

        Data* m_data;
    };


    //*************************************************************************
    //
    // Template definitions for Storage.
    //
    //*************************************************************************
    template <typename T>
    Storage<T>::Storage()
        : m_data(nullptr)
    {
    }


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
    Storage<T>::Storage(Storage const & other)
        : m_data(nullptr)
    {
        SetData(other.m_data);
    }


    template <typename T>
    Storage<T>::Storage(ExpressionTree& tree, Storage<T*>& base, size_t offset)
        : m_data(nullptr)
    {
        base.ConvertToValue(tree, true);

        // Transfer ownership of datablock to this Storage.
        SetData(base.m_data->ConvertToIndirect(base.m_data->m_registerId, offset));
        base.Reset();
    }


    template <typename T>
    Storage<T>::~Storage()
    {
        SetData(nullptr);
    }


    template <typename T>
    Storage<T>& Storage<T>::operator=(Storage const & other)
    {
        SetData(other.m_data);
        return *this;
    }


    template <typename T>
    bool Storage<T>::IsNull() const
    {
        return m_data == nullptr;
    }


    template <typename T>
    void Storage<T>::Reset()
    {
        SetData(nullptr);
    }


    template <typename T>
    void Storage<T>::ConvertToValue(ExpressionTree& tree, bool forModification)
    {
        if (m_data->m_refCount == 1)
        {
            // We are the sole owner of this storage and can repurpose it.
            switch (m_data->m_storageClass)
            {
            case Data::Immediate:
                {
                    // Allocate a register and load this value into the register.
                    auto dest = m_data->m_tree.AllocateRegister<DirectRegister>();
                    m_data->m_tree.GetCodeGenerator().Op("mov", dest, m_data->GetImmediate<T>());

                    m_data->m_storageClass = Data::Direct;
                    m_data->m_registerId = dest.GetId();
                }
                break;
            case Data::Direct:
                // This storage is already a direct value. No work required.
                break;
            case Data::Indirect:
                {
                    BaseRegister base = BaseRegister(m_data->m_registerId);
#pragma warning(push)
#pragma warning(disable:4127)
                    if (IsFloatingPointType<T>::value || m_data->m_tree.IsBasePointer(base))
#pragma warning(pop)
                    {
                        // We need to allocate a new register for the value for one of two reasons:
                        //   The value is float and therefore cannot be stored in base.
                        //   Base is the reserved base pointer register in the tree.
                        DirectRegister dest = m_data->m_tree.AllocateRegister<DirectRegister>();
                        m_data->m_tree.GetCodeGenerator().Op("mov", dest, base, m_data->m_offset);

                        if (m_data->m_tree.IsBasePointer(base))
                        {
                            m_data->m_tree.ReleaseTemporary(m_data->m_offset);
                        }
                        else
                        {
                            m_data->m_tree.ReleaseRegister(base);
                        }
                        m_data->m_registerId = dest.GetId();
                        m_data->m_storageClass = Data::Direct;
                    }
                    else
                    {
                        // We can reuse the base register for the value.
                        DirectRegister dest(m_data->m_registerId);
                        BaseRegister base = BaseRegister(m_data->m_registerId);
                        m_data->m_tree.GetCodeGenerator().Op("mov", dest, base, m_data->m_offset);
                        m_data->m_storageClass = Data::Direct;
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
            case Data::Immediate:
                {
                    // Allocate a register and load this value into the register.
                    auto dest = m_data->m_tree.AllocateRegister<DirectRegister>();
                    m_data->m_tree.GetCodeGenerator().Op("mov", dest, m_data->GetImmediate<T>());

                    SetData(new (tree.GetAllocator().Allocate(sizeof(Data))) Data(tree, dest));
                }
                break;
            case Data::Direct:
                if (forModification)
                {
                    // We need to allocate a new register for the value.
                    DirectRegister dest = m_data->m_tree.AllocateRegister<DirectRegister>();
                    DirectRegister src = DirectRegister(m_data->m_registerId);
                    m_data->m_tree.GetCodeGenerator().Op("mov", dest, src);

                    SetData(new (tree.GetAllocator().Allocate(sizeof(Data))) Data(tree, dest));
                }
                break;
            case Data::Indirect:
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

    template <typename T>
    void Storage<T>::Spill(ExpressionTree& tree)
    {
        ConvertToValue(tree, false);
        Storage dest(tree, tree.GetBasePointer(), tree.AllocateTemporary());
        tree.GetCodeGenerator().Mov(dest.GetBaseRegister(), dest.GetOffset(), GetDirectRegister());
        *this = dest;
    }


    template <typename T>
    typename Data::Class Storage<T>::GetClass() const
    {
        return m_data->m_storageClass;
    }


    template <typename T>
    T Storage<T>::GetImmediate() const
    {
        Assert(m_data->m_storageClass == Data::Immediate, "GetImmediate(): storage class must be immediate.");
        return m_data->GetImmediate<T>();
    }


    template <typename T>
    typename Storage<T>::BaseRegister Storage<T>::GetBaseRegister() const
    {
        Assert(m_data->m_storageClass == Data::Indirect, "GetBaseRegister(): storage class must be indirect.");
        return BaseRegister(m_data->m_registerId);
    }


    template <typename T>
    size_t Storage<T>::GetOffset() const
    {
        Assert(m_data->m_storageClass == Data::Indirect, "GetOffset(): storage class must be indirect.");
        return m_data->m_offset;
    }


    template <typename T>
    typename Storage<T>::DirectRegister Storage<T>::GetDirectRegister() const
    {
        Assert(m_data->m_storageClass == Data::Direct, "GetDirectRegister(): storage class must be direct.");
        return DirectRegister(m_data->m_registerId);
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
//                std::cout << "Decrement " << m_data << " to " << m_data->m_refCount - 1 << std::endl;
                if (--m_data->m_refCount == 0)
                {
                    if (m_data->m_storageClass == Data::Direct)
                    {
//                        std::cout << "Release Direct Register " << DirectRegister(m_data->m_registerId).GetName() << std::endl;
                        m_data->m_tree.ReleaseRegister(DirectRegister(m_data->m_registerId));
                    }
                    else if (m_data->m_storageClass == Data::Indirect)
                    {
                        BaseRegister base(m_data->m_registerId);

                        if (m_data->m_tree.IsBasePointer(base))
                        {
//                            std::cout << "Release temporary " << m_data->m_offset << std::endl;
                            m_data->m_tree.ReleaseTemporary(m_data->m_offset);
                        }
                        else
                        {
//                            std::cout << "Release Base Register " << base.GetName() << std::endl;
                            m_data->m_tree.ReleaseRegister(base);
                        }
                    }
                }
            }

            // Set the new data ptr and increment its ref count if applicable.
            m_data = data;
            if (m_data != nullptr)
            {
                ++m_data->m_refCount;
//                std::cout << "Increment " << m_data << " to " << m_data->m_refCount << std::endl;
            }
        }
    }
}
