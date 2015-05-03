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
    class RIPRelativeImmediate;

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
        void AddRIPRelative(RIPRelativeImmediate& node);
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
        Storage<T> RIPRelative(__int32 offset);

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

        void Pass0();
        void Pass1();
        void Pass2();
        void Pass3();
        void Print() const;


        template <unsigned SIZE>
        unsigned GetAvailableRegisterCountInternal(Register<SIZE, false> ignore) const;

        template <unsigned SIZE>
        unsigned GetAvailableRegisterCountInternal(Register<SIZE, true> ignore) const;

        template <typename T>
        class FreeListHelper;

        std::vector<NodeBase*> m_topologicalSort;
        std::vector<ParameterBase*> m_parameters;
        std::vector<RIPRelativeImmediate*> m_ripRelatives;

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

        // Note: attempt to create an immediate storage will not compile for floats.
        template <typename T>
        Data(ExpressionTree& tree, T value);

        ExpressionTree& GetTree() const;

        StorageClass GetStorageClass() const;
        void SetStorageClass(StorageClass storageClass);

        unsigned GetRegisterId() const;
        void SetRegisterId(unsigned id);

        __int32 GetOffset() const;

        // Note: attempt to GetImmediate() will not compile for floats.
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
        typedef Register<sizeof(T), IsFloatingPointType<T>::value> DirectRegister;
        typedef Register<sizeof(T*), false> BaseRegister;
        typedef Register<8, IsFloatingPointType<T>::value> FullRegister;

        Storage();

        template <typename U>
        explicit Storage(const Storage<U>& other);

        // Takes ownership of a base storage, adds the offset to it and
        // dereferences it to produce a Storage<T>. Equivalent to
        // *static_cast<T*>(base + byteOffset).
        Storage(ExpressionTree& tree, Storage<void*>&& base, __int32 byteOffset);

        Storage(Storage const & other);

        Storage& operator=(Storage const & other);

        ~Storage();

        void Reset();
        bool IsNull() const;

        StorageClass GetStorageClass() const;

        DirectRegister GetDirectRegister() const;

        BaseRegister GetBaseRegister() const;
        __int32 GetOffset() const;

        // Note: attempt to GetImmediate() will not compile for floats.
        T GetImmediate() const;

        DirectRegister ConvertToDirect(bool forModification);

        void Spill(ExpressionTree& tree);

        // Prints the information about the storage's type, value, base register
        // and offset to the output stream.
        void Print(std::ostream& out) const;

    private:
        friend class ExpressionTree;

        // Types used to select the correct flavor of immediate methods
        // depending on whether T is a floating point value.
        struct FloatingPointFlavor {};
        struct NonFloatingPointFlavor {};
        typedef typename std::conditional<std::is_floating_point<T>::value,
                                          FloatingPointFlavor,
                                          NonFloatingPointFlavor>::type
            ImmediateFlavor;

        Storage(ExpressionTree::Data* data);

        void SetData(ExpressionTree::Data* data);
        void SetData(Storage& other);

        void ConvertImmediateToDirect(NonFloatingPointFlavor);
        void ConvertImmediateToDirect(FloatingPointFlavor);

        void PrintImmediate(std::ostream& out, NonFloatingPointFlavor) const;
        void PrintImmediate(std::ostream& out, FloatingPointFlavor) const;

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
    ExpressionTree::Storage<T> ExpressionTree::RIPRelative(__int32 offset)
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
        if (!base.IsRIP() && !tree.IsBasePointer(base))
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
          m_isFloat(false),
          m_registerId(0),
          m_offset(0),
          m_refCount(0)
    {
        static_assert(!std::is_floating_point<T>::value, "Immediate floating point values are not supported");
        static_assert(sizeof(T) <= sizeof(m_immediate), "Unsupported type.");
        *reinterpret_cast<T*>(&m_immediate) = value;
    }


    template <typename T>
    T ExpressionTree::Data::GetImmediate() const
    {
        static_assert(!std::is_floating_point<T>::value, "Immediate floating point values are not supported");
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
    ExpressionTree::Storage<T>::Storage(const Storage<U>& other)
        : m_data(nullptr)
    {
        SetData(other.m_data);
    }


    template <typename T>
    ExpressionTree::Storage<T>::Storage(ExpressionTree& tree,
                                        Storage<void*>&& base,
                                        __int32 byteOffset)
        : m_data(nullptr)
    {
        // Load the base pointer into a register.
        base.ConvertToDirect(true);

        // Dereference it.
        base.m_data->ConvertToIndirect(byteOffset);

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
        static_assert(!std::is_floating_point<T>::value, "Immediate floating point values are not supported");
        Assert(m_data->GetStorageClass() == StorageClass::Immediate, "GetImmediate(): storage class must be immediate.");
        return m_data->GetImmediate<T>();
    }


    template <typename T>
    typename ExpressionTree::Storage<T>::DirectRegister
    ExpressionTree::Storage<T>::ConvertToDirect(bool forModification)
    {
        // TODO: Target the whole target register with MovZX to prevent the partial register stall.

        auto & tree = m_data->GetTree();
        auto & code = tree.GetCodeGenerator();
        const bool isSoleOwner = m_data->GetRefCount() == 1;

        switch (m_data->GetStorageClass())
        {
        case StorageClass::Immediate:
            ConvertImmediateToDirect(ImmediateFlavor());
            break;

        case StorageClass::Direct:
            // If the storage is already direct, action is only necessary if it
            // needs to be modified but we're not the sole owner.
            if (!isSoleOwner && forModification)
            {
                auto dest = tree.Direct<T>();
                code.Emit<OpCode::Mov>(dest.GetDirectRegister(), GetDirectRegister());

                SetData(dest);
            }
            break;
        case StorageClass::Indirect:
            {
                BaseRegister base = GetBaseRegister();

                // If we fully own the storage, the type of the base register is
                // compatible and it's not one of the reserved (shared) registers,
                // then the base register can be reused.
                // TODO: Also check for RSP; add IsAnyReservedBaseRegister()
                // method and use everywhere.
                if (isSoleOwner
                    && BaseRegister::c_isFloat == DirectRegister::c_isFloat
                    && !base.IsRIP()
                    && !tree.IsBasePointer(base))
                {
                    code.Emit<OpCode::Mov>(DirectRegister(base), base, GetOffset());
                    m_data->SetStorageClass(StorageClass::Direct);
                }
                else
                {
                    // Otherwise, move the data to a newly allocated register.
                    auto dest = tree.Direct<T>();
                    code.Emit<OpCode::Mov>(dest.GetDirectRegister(), base, GetOffset());
                    SetData(dest);
                }
            }
            break;

        default:
            Assert(false, "ConvertToDirect: invalid storage class.");
            break;
        }

        return GetDirectRegister();
    }


    template <typename T>
    void ExpressionTree::Storage<T>::ConvertImmediateToDirect(NonFloatingPointFlavor)
    {
        Assert(m_data->GetStorageClass() == StorageClass::Immediate, "Unexpected storage class");

        auto & tree = m_data->GetTree();
        auto & code = tree.GetCodeGenerator();

        // Allocate a register and load the immediate into it.
        auto dest = tree.Direct<T>();
        code.EmitImmediate<OpCode::Mov>(dest.GetDirectRegister(), m_data->GetImmediate<T>());

        SetData(dest);
    }


    template <typename T>
    void ExpressionTree::Storage<T>::ConvertImmediateToDirect(FloatingPointFlavor)
    {
        Assert(m_data->GetStorageClass() == StorageClass::Immediate, "Unexpected storage class");

        // This should never be hit, it's impossible to compile a float with StorageClass::Immediate.
        Assert(false, "Unexpected occurrence of a floating point immediate");
    }


    template <typename T>
    void ExpressionTree::Storage<T>::Spill(ExpressionTree& tree)
    {
        ConvertToDirect(false);
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


    template <typename T>
    void ExpressionTree::Storage<T>::Print(std::ostream& out) const
    {
        switch (GetStorageClass())
        {
        case StorageClass::Direct:
            out << "register " << GetDirectRegister().GetName();
            break;
    
        case StorageClass::Immediate:
            PrintImmediate(out, ImmediateFlavor());
            break;

        case StorageClass::Indirect:
            out << "indirect [" << GetBaseRegister().GetName() << std::hex;

            if (GetOffset() < 0)
            {
                out << " - " << -GetOffset() << "h";
            }
            else
            {
                out << " + " << GetOffset() << "h";
            }
            out << "]" << std::dec;
            break;

        default:
            out << "[unknown storage type]";
            break;
        }
    }


    template <typename T>
    void ExpressionTree::Storage<T>::PrintImmediate(std::ostream& out, NonFloatingPointFlavor) const
    {
        out << "immediate value " << std::hex << GetImmediate() << "h" << std::dec;
    }


    template <typename T>
    void ExpressionTree::Storage<T>::PrintImmediate(std::ostream& out, FloatingPointFlavor) const
    {
        // This should never be hit, it's impossible to compile a float with StorageClass::Immediate.
        Assert(false, "Unexpected occurrence of a floating point immediate");
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
