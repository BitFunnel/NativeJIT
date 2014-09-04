#pragma once

#include <iostream>             // TODO: Remove this temp debug include.
#include <vector>

#include "NativeJIT/Register.h"
#include "Temporary/NonCopyable.h"
#include "Temporary/Assert.h"         // TODO: Delete this.
#include "Temporary/NonCopyable.h"
#include "TypePredicates.h"

/*
Attempting to spill a temporary
*/

namespace Allocators
{
    class IAllocator;
}


namespace NativeJIT
{
    class FunctionBuffer;
    class NodeBase;
    class ParameterBase;

    enum class StorageClass {Direct, Indirect, Immediate};

    class ExpressionTree : public NonCopyable
    {
    private:
        class Data;

    public:
        ExpressionTree(Allocators::IAllocator& allocator, FunctionBuffer& code);

        Allocators::IAllocator& GetAllocator() const;
        FunctionBuffer& GetCodeGenerator() const;


        //
        // Tree construction
        //
        unsigned AddNode(NodeBase& node);
        unsigned AddParameter(ParameterBase& parameter);
        void Compile();

        //
        // Storage allocation.
        //

        template <typename T> class Storage;

        template <typename T>
        Storage<T> Direct();

        template <typename T>
        Storage<T> Direct(Register<sizeof(T), IsFloatingPointType<T>::value> r);

        template <typename T>
        Storage<T> Indirect(__int32 offset);

        template <typename T>
        Storage<T> Indirect(Register<sizeof(T), false> base, __int32 offset);

        template <typename T>
        Storage<T> RIPRelative(unsigned __int32 offset);

        template <typename T>
        Storage<T> Temporary();

        template <typename T>
        Storage<T> Immediate(T value);

        template <unsigned SIZE>
        void ReleaseRegister(Register<SIZE, false> r);

        template <unsigned SIZE>
        void ReleaseRegister(Register<SIZE, true> r);

        void ReleaseTemporary(__int32 offset);

        //
        //
        //
        template <typename REGISTERTYPE>
        unsigned GetAvailableRegisterCount() const;

        unsigned GetRXXUsageMask() const;

    protected:
        void  const * GetUntypedEntryPoint() const;

        Allocators::IAllocator& m_allocator;
        FunctionBuffer & m_code;

    private:
        template <typename T> friend class Storage;

        template <unsigned SIZE>
        class FreeList
        {
        public:
            FreeList();

            unsigned GetCount() const;

            bool IsAvailable(unsigned id) const;

            unsigned Allocate();
            void Allocate(unsigned id);

            void Release(unsigned id);

            void SetData(unsigned id, Data* data);
            Data* GetData(unsigned id) const;
            void MoveData(unsigned dest, unsigned src);

            unsigned GetUsageMask() const;

        private:
            unsigned m_positions[SIZE];
            unsigned m_free[SIZE];
            Data* m_data[SIZE];
            unsigned m_count;
            unsigned m_usageMask;
        };


        bool IsBasePointer(Register<8, false> r) const;
        Register<sizeof(void*), false> GetBasePointer() const;

        void Pass1();
        void Pass2();
        void Pass3();
        void Print() const;


        template <unsigned SIZE>
        unsigned GetAvailableRegisterCountInternal(Register<SIZE, false> ignore) const;

        template <unsigned SIZE>
        unsigned GetAvailableRegisterCountInternal(Register<SIZE, true> ignore) const;


        //
        // Mov() helper methods to get around limitation that X64 does not support op xmm, imm.
        // RIP-relative addressing for floating point immediates should simplify these methods.
        // May still need some sort of indirection because mov rxx, imm is still supported and is
        // called by code templated by ISFLOAT.
        //
        // TODO: Consider renaming these to something like MovHelper. Consider moving into
        // CodeGenHelpers.
        template <unsigned SIZE, typename T>
        void Mov(Register<SIZE, false> dest, T value);

        template <typename T>
        void Mov(Register<8, true> dest, T value);

        template <typename T>
        void Mov(Register<4, true> dest, T value);


        template <typename T>
        class FreeListHelper;


        std::vector<NodeBase*> m_topologicalSort;
        std::vector<ParameterBase*> m_parameters;

        static const unsigned c_rxxCount = 16;
        FreeList<c_rxxCount> m_rxxFreeList;

        static const unsigned c_xmmCount = 16;
        FreeList<c_xmmCount> m_xmmFreeList;

        // TODO: Do we want to use an std::vector here?
        unsigned m_temporaryCount;
        std::vector<__int32> m_temporaries;

        Register<sizeof(void*), false> m_basePointer;
    };


    class ExpressionTree::Data : public NonCopyable
    {
    public:
        template <unsigned SIZE, bool ISFLOAT>
        Data(ExpressionTree& tree, Register<SIZE, ISFLOAT> r);

        template <unsigned SIZE, bool ISFLOAT>
        Data(ExpressionTree& tree, Register<SIZE, ISFLOAT> r, __int32 offset);

        template <typename T>
        Data(ExpressionTree& tree, T value);

        ExpressionTree& GetTree() const;

        StorageClass GetStorageClass() const;
        void SetStorageClass(StorageClass storageClass);

        unsigned GetRegisterId() const;
        void SetRegisterId(unsigned id);

        __int32 GetOffset() const;

        template <typename T>
        T GetImmediate() const;

        void ConvertToIndirect(__int32 offset);

        unsigned GetRefCount() const;
        unsigned Decrement();
        void Increment();

    private:
        ExpressionTree& m_tree;

        // How the data is stored.
        StorageClass m_storageClass;

        // Which register.
        bool m_isFloat;
        unsigned m_registerId;
        __int32 m_offset;

        // m_immediate is mutable to avoid const-cast in ExpressionTree::Data::GetImmediate().
        mutable size_t m_immediate;

        // Who is using it.
        unsigned m_refCount;
    };


    // Storage should be public because it is a return type for Node.
    // No sense in making every instance be ExpressionTree::Storage
    // when we're already in NativeJIT namespace.
    // Storage should be private so that its constructor can take an m_data.
    template <typename T>
    class ExpressionTree::Storage
    {
    public:
        typedef Register<sizeof(T), IsFloatingPointType<T>::value/*, IsSigned<T>::value*/> DirectRegister;
        typedef Register<sizeof(T*), false/*, false*/> BaseRegister;
        typedef Register<8, IsFloatingPointType<T>::value> FullRegister;

        Storage();

        template <typename U>
        Storage(Storage<U> other);

        Storage(ExpressionTree& tree, Storage<T*>& base, __int32 offset);

        Storage(Storage const & other);

        Storage& operator=(Storage const & other);

        ~Storage();

        void Reset();
        bool IsNull() const;

        StorageClass GetStorageClass() const;

        DirectRegister GetDirectRegister() const;

        BaseRegister GetBaseRegister() const;
        __int32 GetOffset() const;

        T GetImmediate() const;


        void ConvertToValue(ExpressionTree& tree, bool forModification);

        void Spill(ExpressionTree& tree);

    private:
        friend class ExpressionTree;

        Storage(ExpressionTree::Data* data);

        void SetData(ExpressionTree::Data* data);
        void SetData(Storage& other);

        ExpressionTree::Data* m_data;
    };


    //*************************************************************************
    //
    // Template definitions for ExpressionTree
    //
    //*************************************************************************
    template <typename T>
    class ExpressionTree::FreeListHelper
    {
    public:
        static auto GetFreeList(ExpressionTree& tree) -> decltype( tree.m_rxxFreeList ) &
        {
            return tree.m_rxxFreeList;
        }
    };


    template <>
    class ExpressionTree::FreeListHelper<float>
    {
    public:
        static auto GetFreeList(ExpressionTree& tree) -> decltype( tree.m_xmmFreeList ) &
        {
            return tree.m_xmmFreeList;
        }
    };


    template <>
    class ExpressionTree::FreeListHelper<double>
    {
    public:
        static auto GetFreeList(ExpressionTree& tree) -> decltype( tree.m_xmmFreeList ) &
        {
            return tree.m_xmmFreeList;
        }
    };


    template <unsigned SIZE>
    class ExpressionTree::FreeListHelper<Register<SIZE, false>>
    {
    public:
        static auto GetFreeList(ExpressionTree& tree) -> decltype( tree.m_rxxFreeList ) &
        {
            return tree.m_rxxFreeList;
        }
    };


    template <unsigned SIZE>
    class ExpressionTree::FreeListHelper<Register<SIZE, true>>
    {
    public:
        static auto GetFreeList(ExpressionTree& tree) -> decltype( tree.m_rxxFreeList ) &
        {
            return tree.m_xmmFreeList;
        }
    };


    template <typename T>
    ExpressionTree::Storage<T> ExpressionTree::Direct()
    {
        auto & freeList = FreeListHelper<T>::GetFreeList(*this);

        // TODO: What if Allocate() fails?
        unsigned id = freeList.Allocate();

        Storage<T>::DirectRegister r(id);

        // TODO: Use allocator.
        Data* data = new Data(*this, r);

        return Storage<T>(data);
    }


    template <typename T>
    ExpressionTree::Storage<T>
    ExpressionTree::Direct(Register<sizeof(T), IsFloatingPointType<T>::value> r)
    {
        auto & freeList = FreeListHelper<T>::GetFreeList(*this);

        unsigned src = r.GetId();
        if (!freeList.IsAvailable(src))
        {
            // Register is not available - bump it.
            // TODO: Should be able to allocate a temporary if no registers are available.
            // This requires some knowledge of how to dereference indirects since an
            // indirect may have to be spilled to a temporary. At the very least, need to
            // know how to move the data (i.e. its size).
            // TODO: What if Allocate() fails?
            unsigned dest = freeList.Allocate();

            // Important to preserve all bits of register. Reason is that we don't know the
            // size required by the previous user.
            GetCodeGenerator().Emit<OpCode::Mov>(Storage<T>::FullRegister(dest), Storage<T>::FullRegister(src));
//            GetCodeGenerator().Emit<OpCode::Mov>(Register<8, false>(dest), Register<8, false>(src));

            freeList.MoveData(dest, src);
        }

        freeList.Allocate(src);

        // TODO: Use allocator.
        Data* data = new Data(*this, r);

        return Storage<T>(data);
    }


    template <typename T>
    ExpressionTree::Storage<T> ExpressionTree::Indirect(__int32 offset)
    {
        auto & freeList = FreeListHelper<T>::GetFreeList(*this);

        unsigned id = freeList.Allocate();

        Storage<T>::IndirectRegister base(id);

        // TODO: Use allocator.
        Data* data = new Data(*this, base, offset);

        return Storage<T>(data);
    }


    template <typename T>
    ExpressionTree::Storage<T>
    ExpressionTree::Indirect(Register<sizeof(T), false> base, __int32 offset) // TODO: This should be Register<8, false> or Register<sizeof(T*), false>
    {
        auto & freeList = FreeListHelper<T>::GetFreeList(*this);

        unsigned src = base.GetId();
        if (!freeList.IsAvailable(src))
        {
            // Register is not available - bump it.
            // TODO: Should be able to allocate a temporary if no registers are available.
            unsigned dest = freeList.Allocate();

            // Important to preserve all bits of register. Reason is that we don't know the
            // size required by the previous user.
            GetCodeGenerator().Emit<OpCode::Mov>(Storage<T>::FullRegister(dest), Storage<T>::FullRegister(src));

            freeList.MoveData(dest, src);
        }

        freeList.Allocate(src);

        // TODO: Use allocator.
        Data* data = new Data(*this, base, offset);

        return Storage<T>(data);
    }


    template <typename T>
    ExpressionTree::Storage<T> ExpressionTree::RIPRelative(unsigned __int32 offset)
    {
        // TODO: Use allocator.
        Data* data = new Data(*this, rip, offset);

        return Storage<T>(data);
    }


    template <typename T>
    ExpressionTree::Storage<T> ExpressionTree::Temporary()
    {
        __int32 temp;

        if (m_temporaries.size() > 0)
        {
            temp = m_temporaries.back();
            m_temporaries.pop_back();
        }
        else
        {
            temp = m_temporaryCount++;
        }

        __int32 offset = temp * sizeof(void*);

        // TODO: Use allocator.
        // TODO: BUGBUG: data needs to be put in m_rxxFreeList.m_data[].
        Data* data = new Data(*this, GetBasePointer(), offset);

        return Storage<T>(data);
    }


    template <typename T>
    ExpressionTree::Storage<T> ExpressionTree::Immediate(T value)
    {
        // TODO: Use allocator.
        return Storage<T>(new Data(*this, value));
    }


    template <unsigned SIZE>
    void ExpressionTree::ReleaseRegister(Register<SIZE, false> r)
    {
        m_rxxFreeList.Release(r.GetId());
    }


    template <unsigned SIZE>
    void ExpressionTree::ReleaseRegister(Register<SIZE, true> r)
    {
        m_xmmFreeList.Release(r.GetId());
    }


    template <typename REGISTERTYPE>
    unsigned ExpressionTree::GetAvailableRegisterCount() const
    {
        return GetAvailableRegisterCountInternal(REGISTERTYPE());
    }


    template <unsigned SIZE>
    unsigned ExpressionTree::GetAvailableRegisterCountInternal(Register<SIZE, false> /*ignore*/) const
    {
        return m_rxxFreeList.GetCount();
    }


    template <unsigned SIZE>
    unsigned ExpressionTree::GetAvailableRegisterCountInternal(Register<SIZE, true> /*ignore*/) const
    {
        return m_xmmFreeList.GetCount();
    }


    template <unsigned SIZE, typename T>
    void ExpressionTree::Mov(Register<SIZE, false> dest, T value)
    {
        m_code.Emit<OpCode::Mov>(dest, value);
    }


    template <typename T>
    void ExpressionTree::Mov(Register<8, true> dest, T value)
    {
        // TODO: Consider RIP-relative addressing to eliminate a register allocation.
        // TODO: BUGBUG: This code is incorrect for float. In the case of float, temp
        // must be 4 byte.
        // TODO: This code will invalidate the Sethi-Ullman register counts in the Label pass.
        auto temp = Direct<unsigned __int64>();
        auto r = temp.GetDirectRegister();
        m_code.Emit<OpCode::Mov>(r, *(reinterpret_cast<unsigned __int64*>(&value)));
        m_code.Emit<OpCode::Mov>(dest, r);
    }


    // TODO: Coalesc with previous.
    template <typename T>
    void ExpressionTree::Mov(Register<4, true> dest, T value)
    {
        // TODO: Consider RIP-relative addressing to eliminate a register allocation.
        // TODO: BUGBUG: This code is incorrect for float. In the case of float, temp
        // must be 4 byte.
        // TODO: This code will invalidate the Sethi-Ullman register counts in the Label pass.
        auto temp = Direct<unsigned __int32>();
        auto r = temp.GetDirectRegister();
        m_code.Emit<OpCode::Mov>(r, *(reinterpret_cast<unsigned __int32*>(&value)));
        m_code.Emit<OpCode::Mov>(dest, r);
    }


    //*************************************************************************
    //
    // Template definitions for ExpressionTree::Data
    //
    //*************************************************************************
    template <unsigned SIZE, bool ISFLOAT>
    ExpressionTree::Data::Data(ExpressionTree& tree,
                               Register<SIZE, ISFLOAT> r)
        : m_tree(tree),
          m_storageClass(StorageClass::Direct),
          m_isFloat(ISFLOAT),
          m_registerId(r.GetId()),
          m_offset(0),
          m_refCount(0)
    {
        auto & freeList = FreeListHelper<Register<SIZE, ISFLOAT>>::GetFreeList(tree);
        freeList.SetData(m_registerId, this);
    }


    // TODO: This should never allow ISFLOAT == true.
    template <unsigned SIZE, bool ISFLOAT>
    ExpressionTree::Data::Data(ExpressionTree& tree,
                               Register<SIZE, ISFLOAT> base,
                               __int32 offset)
        : m_tree(tree),
          m_storageClass(StorageClass::Indirect),
          m_isFloat(ISFLOAT),
          m_registerId(base.GetId()),
          m_offset(offset),
          m_refCount(0)
    {
        if (!base.IsRIP())
        {
            auto & freeList = FreeListHelper<Register<SIZE, ISFLOAT>>::GetFreeList(tree);
            freeList.SetData(m_registerId, this);
        }
    }


    template <typename T>
    ExpressionTree::Data::Data(ExpressionTree& tree,
                               T value)
        : m_tree(tree),
          m_storageClass(StorageClass::Immediate),
          m_isFloat(IsFloatingPointType<T>::value),
          m_registerId(0),
          m_offset(0),
          m_refCount(0)
    {
        // TODO: replace static_assert with enable_if
        static_assert(sizeof(T) <= sizeof(m_immediate), "Unsupported type.");
        *reinterpret_cast<T*>(&m_immediate) = value;
    }


    template <typename T>
    T ExpressionTree::Data::GetImmediate() const
    {
        static_assert(sizeof(T) <= sizeof(m_immediate), "Unsupported type.");
        Assert(m_storageClass == StorageClass::Immediate, "error");
        return *(reinterpret_cast<T*>(&m_immediate));
    }


    //*************************************************************************
    //
    // Template definitions for Storage<T>
    //
    //*************************************************************************
    template <typename T>
    ExpressionTree::Storage<T>::Storage()
        : m_data(nullptr)
    {
    }


    template <typename T>
    template <typename U>
    ExpressionTree::Storage<T>::Storage(Storage<U> other)
        : m_data(nullptr)
    {
        SetData(other.m_data);
    }


    template <typename T>
    ExpressionTree::Storage<T>::Storage(ExpressionTree& tree, Storage<T*>& base, __int32 offset)
        : m_data(nullptr)
    {
        // Load the base pointer into a register.
        base.ConvertToValue(tree, true);

        // Dereference it.
        base.m_data->ConvertToIndirect(offset);

        // Transfer ownership of datablock to this Storage.
        SetData(base.m_data);
        base.Reset();
    }


    template <typename T>
    ExpressionTree::Storage<T>::Storage(Storage const & other)
        : m_data(nullptr)
    {
        SetData(other.m_data);
    }


    template <typename T>
    ExpressionTree::Storage<T>& ExpressionTree::Storage<T>::operator=(Storage const & other)
    {
        SetData(other.m_data);
        return *this;
    }


    template <typename T>
    ExpressionTree::Storage<T>::Storage(ExpressionTree::Data* data)
        : m_data(data)
    {
        m_data->Increment();
    }


    template <typename T>
    ExpressionTree::Storage<T>::~Storage()
    {
        SetData(nullptr);
    }


    template <typename T>
    void ExpressionTree::Storage<T>::Reset()
    {
        SetData(nullptr);
    }


    template <typename T>
    bool ExpressionTree::Storage<T>::IsNull() const
    {
        return m_data == nullptr;
    }


    template <typename T>
    StorageClass ExpressionTree::Storage<T>::GetStorageClass() const
    {
        return m_data->GetStorageClass();
    }


    template <typename T>
    typename ExpressionTree::Storage<T>::DirectRegister ExpressionTree::Storage<T>::GetDirectRegister() const
    {
        Assert(m_data->GetStorageClass() == StorageClass::Direct, "GetDirectRegister(): storage class must be direct.");
        return DirectRegister(m_data->GetRegisterId());
    }


    template <typename T>
    typename ExpressionTree::Storage<T>::BaseRegister ExpressionTree::Storage<T>::GetBaseRegister() const
    {
        Assert(m_data->GetStorageClass() == StorageClass::Indirect, "GetBaseRegister(): storage class must be indirect.");
        return BaseRegister(m_data->GetRegisterId());
    }


    template <typename T>
    __int32 ExpressionTree::Storage<T>::GetOffset() const
    {
        Assert(m_data->GetStorageClass() == StorageClass::Indirect, "GetOffset(): storage class must be indirect.");
        return m_data->GetOffset();
    }


    template <typename T>
    T ExpressionTree::Storage<T>::GetImmediate() const
    {
        Assert(m_data->GetStorageClass() == StorageClass::Immediate, "GetImmediate(): storage class must be immediate.");
        return m_data->GetImmediate<T>();
    }


    template <typename T>
    void ExpressionTree::Storage<T>::ConvertToValue(ExpressionTree& tree, bool forModification)
    {
        // TODO: Why do we store tree in m_data if it is passed as a parameter here?

        if (m_data->GetRefCount() == 1)
        {
            // We are the sole owner of this storage and can repurpose it.
            switch (m_data->GetStorageClass())
            {
            case StorageClass::Immediate:
                {
                    // Allocate a register and load this value into the register.
                    auto dest = tree.Direct<T>();
                    tree.Mov(dest.GetDirectRegister(), m_data->GetImmediate<T>());

                    // Problem here is that SetData occurs after Mov which allocates another register???

                    SetData(dest);
                }
                break;
            case StorageClass::Direct:
                // This storage is already a direct value. No work required.
                break;
            case StorageClass::Indirect:
                {
                    BaseRegister base = BaseRegister(m_data->GetRegisterId());
#pragma warning(push)
#pragma warning(disable:4127)
                    if (IsFloatingPointType<T>::value || tree.IsBasePointer(base))
#pragma warning(pop)
                    {
                        // We need to allocate a new register for the value for one of two reasons:
                        //   The value is float and therefore cannot be stored in base.
                        //   Base is the reserved base pointer register in the tree.
                        auto dest = tree.Direct<T>();
                        tree.GetCodeGenerator().Emit<OpCode::Mov>(dest.GetDirectRegister(),
                                                                  base,
                                                                  m_data->GetOffset());
                        SetData(dest);
                    }
                    else
                    {
                        // We can reuse the base register for the value.
                        DirectRegister dest(m_data->GetRegisterId());

                        BaseRegister base = BaseRegister(m_data->GetRegisterId());
                        __int32 offset = m_data->GetOffset();

                        tree.GetCodeGenerator().Emit<OpCode::Mov>(dest, base, offset);
                        m_data->SetStorageClass(StorageClass::Direct);
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
            switch (m_data->GetStorageClass())
            {
            case StorageClass::Immediate:
                {
                    // Allocate a register and load this value into the register.
                    auto dest = tree.Direct<T>();
                    tree.Mov(dest.GetDirectRegister(), m_data->GetImmediate<T>());
                    SetData(dest);
                }
                break;
            case StorageClass::Direct:
                if (forModification)
                {
                    // We need to allocate a new register for the value.
                    auto dest = tree.Direct<T>();
                    auto src = DirectRegister(m_data->GetRegisterId());
                    tree.GetCodeGenerator().Emit<OpCode::Mov>(dest.GetDirectRegister(), src);
                    SetData(dest);
                }
                break;
            case StorageClass::Indirect:
                {
                    // We need to allocate a new register for the value.
                    auto dest = tree.Direct<T>();

                    BaseRegister base = BaseRegister(m_data->GetRegisterId());
                    __int32 offset = m_data->GetOffset();
                    tree.GetCodeGenerator().Emit<OpCode::Mov>(dest.GetDirectRegister(),
                                                              base,
                                                              offset);
                    SetData(dest);
                }
                break;
            default:
                Assert(false, "ConvertToValue: invalid storage class.");
            }
        }
    }


    template <typename T>
    void ExpressionTree::Storage<T>::Spill(ExpressionTree& tree)
    {
        ConvertToValue(tree, false);
        auto dest = tree.Temporary<T>();
        tree.GetCodeGenerator().Emit<OpCode::Mov>(dest.GetBaseRegister(), dest.GetOffset(), GetDirectRegister());
        *this = dest;
    }


    template <typename T>
    void ExpressionTree::Storage<T>::SetData(ExpressionTree::Data* data)
    {
        // If the new data is different from the existing data ...
        if (m_data != data)
        {
            // If there is existing data, decrement the ref count and release resources if zero.
            if (!IsNull())
            {
                if (m_data->Decrement() == 0)
                {
                    if (m_data->GetStorageClass() == StorageClass::Direct)
                    {
                        std::cout << "Release Direct Register " << DirectRegister(m_data->GetRegisterId()).GetName() << std::endl;
                        m_data->GetTree().ReleaseRegister(DirectRegister(m_data->GetRegisterId()));
                    }
                    else if (m_data->GetStorageClass() == StorageClass::Indirect)
                    {
                        BaseRegister base(m_data->GetRegisterId());

                        if (base.IsRIP())
                        {
                            std::cout << "Don't release RIP-relative constant at " << m_data->GetOffset() << std::endl;
                        }
                        else if (m_data->GetTree().IsBasePointer(base))
                        {
                            std::cout << "Release temporary " << m_data->GetOffset() << std::endl;
                            m_data->GetTree().ReleaseTemporary(m_data->GetOffset());
                        }
                        else
                        {
                            std::cout << "Release Base Register " << base.GetName() << std::endl;
                            m_data->GetTree().ReleaseRegister(base);
                        }
                    }

                    // TODO: Release m_data if not using arena allocator.
                }
            }

            // Set the new data ptr and increment its ref count if applicable.
            m_data = data;
            if (m_data != nullptr)
            {
                m_data->Increment();
            }
        }
    }


    template <typename T>
    void ExpressionTree::Storage<T>::SetData(Storage& other)
    {
        SetData(other.m_data);
    }

    //*************************************************************************
    //
    // template definitions for FreeList
    //
    //*************************************************************************
    template <unsigned SIZE>
    ExpressionTree::FreeList<SIZE>::FreeList()
        : m_count(0),
          m_usageMask(0)
    {
        // Mark all positions as unavailable.
        for (unsigned i = 0 ; i < SIZE; ++i)
        {
            m_positions[i] = SIZE;
            m_data[i] = nullptr;
        }
    }


    template <unsigned SIZE>
    unsigned ExpressionTree::FreeList<SIZE>::GetCount() const
    {
        return m_count;
    }


    template <unsigned SIZE>
    bool ExpressionTree::FreeList<SIZE>::IsAvailable(unsigned id) const
    {
        Assert(id < SIZE, "id out of range.");
        return m_positions[id] != SIZE;
    }


    template <unsigned SIZE>
    unsigned ExpressionTree::FreeList<SIZE>::Allocate()
    {
        Assert(m_count > 0, "no ids available.");

        unsigned id = m_free[m_count - 1];
        m_positions[id] = SIZE;
        m_usageMask |= (1ul << id);
        m_count--;

        return id;
    }


    // TODO: Perhaps this should return a Data*.
    // If nothing is displaced, returns nullptr.
    // Otherwise returns Data* for displaced.
    template <unsigned SIZE>
    void ExpressionTree::FreeList<SIZE>::Allocate(unsigned id)
    {
        Assert(id < SIZE, "id out of range.");

        unsigned position = m_positions[id];
        Assert(position < SIZE, "id not available.");

        if (position < m_count - 1)
        {
            // Move the last id in to the position of the requested id.
            unsigned lastId = m_free[m_count - 1];
            m_free[position] = lastId;
            m_positions[lastId] = position;
            m_count--;

            m_positions[id] = SIZE;
        }
        else
        {
            // Requested if is in the last position. Just trim the array.
            m_positions[id] = SIZE;
            m_count--;
        }

        m_usageMask |= (1ul << id);
    }


    template <unsigned SIZE>
    void ExpressionTree::FreeList<SIZE>::Release(unsigned id)
    {
        Assert(id < SIZE, "id out of range.");
        Assert(m_positions[id] == SIZE, "attempting to return unallocated id.");

        m_free[m_count] = id;
        m_positions[id] = m_count;
        m_data[id] = nullptr; 
        m_usageMask &= ~(1ul << id);
        m_count++;
    }


    template <unsigned SIZE>
    void ExpressionTree::FreeList<SIZE>::SetData(unsigned id, Data* data)
    {
        Assert(id < SIZE, "id out of range.");
        Assert(m_data[id] == nullptr, "error");
        m_data[id] = data;
    }


    template <unsigned SIZE>
    ExpressionTree::Data* ExpressionTree::FreeList<SIZE>::GetData(unsigned id) const
    {
        Assert(id < SIZE, "id out of range.");
        return m_data[id];
    }


    template <unsigned SIZE>
    void ExpressionTree::FreeList<SIZE>::MoveData(unsigned dest, unsigned src)
    {
        Assert(dest < SIZE, "dest id out of range.");
        Assert(src < SIZE, "src id out of range.");
        Assert(m_data[dest] == nullptr, "error");
        Assert(m_data[src] != nullptr, "error");

        StorageClass storageClass = m_data[src]->GetStorageClass();
        Assert(storageClass == StorageClass::Direct ||
               storageClass == StorageClass::Indirect, "error");

        m_data[dest] = m_data[src];
        m_data[src] = nullptr;

        m_data[dest]->SetRegisterId(dest);

        Release(src);
    }


    template <unsigned SIZE>
    unsigned ExpressionTree::FreeList<SIZE>::GetUsageMask() const
    {
        return m_usageMask;
    }
}
