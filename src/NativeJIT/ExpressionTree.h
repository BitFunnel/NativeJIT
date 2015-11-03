#pragma once

#include <array>                // For arrays in FreeList.
#include <iostream>             // TODO: Remove this temp debug include.
#include <vector>

#include "NativeJIT/BitOperations.h"
#include "NativeJIT/Register.h"
#include "Temporary/NonCopyable.h"
#include "Temporary/Assert.h"         // TODO: Delete this.
#include "Temporary/NonCopyable.h"
#include "TypePredicates.h"

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


    // A class which increases reference counter on construction and decreases
    // it on destruction.
    // This class is not thread safe.
    class ReferenceCounter final
    {
    public:
        // Default constructor performs no observable reference counting. Used
        // in cases where an object of this class needs to be constructed before
        // the counter object is known (f. ex. in constructors of classes which
        // contain the ReferenceCounter as a member).
        ReferenceCounter();

        // Increases the reference count in the specified counter.
        ReferenceCounter(unsigned& counter);

        // Uses the counter in the other object as its counter and increases it.
        ReferenceCounter(ReferenceCounter const & other);

        // Decreases the current counter.
        ~ReferenceCounter();

        // Decreases the current counter, replaces it with the one in the other
        // object and increases that counter.
        ReferenceCounter& operator=(ReferenceCounter const & other);

        // Decreases the current counter and disassociates itself from it.
        void Reset();

    private:
        // Pointer to the current counter.
        unsigned* m_counter;

        // Methods to add and remove a reference to the counter.
        void AddReference();
        void RemoveReference();
    };


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
        Storage<T> Direct(typename Storage<T>::DirectRegister r);

        template <typename T>
        Storage<T> RIPRelative(__int32 offset);

        // Returns indirect storage relative to the base pointer for a variable
        // of type T. It is guaranteed that it's legal to access the whole quadword
        // at the target address.
        template <typename T>
        Storage<T> Temporary();

        template <typename T>
        Storage<T> Immediate(T value);

        template <unsigned SIZE>
        void ReleaseRegister(Register<SIZE, false> r);

        template <unsigned SIZE>
        void ReleaseRegister(Register<SIZE, true> r);

        void ReleaseTemporary(__int32 offset);

        // Returns whether a register is pinned.
        template <unsigned SIZE, bool ISFLOAT>
        bool IsPinned(Register<SIZE, ISFLOAT> reg);

        //
        //
        //
        unsigned GetRXXUsageMask() const;
        unsigned GetXMMUsageMask() const;

    protected:
        void  const * GetUntypedEntryPoint() const;

        Allocators::IAllocator& m_allocator;
        FunctionBuffer & m_code;

    private:
        // Keeps track of used and free registers and the Data* associated with
        // each register.
        //
        // The Data* is kept so that it's easily possible to create another
        // Storage around an allocated register when necessary (f. ex. for
        // spilling. Thus, there's a required 1-1 mapping between a general
        // purpose register and the Data* that refers to it (either as Direct
        // or Indirect).
        //
        // Note that it's neither necessary nor possible to hold information
        // about a singular Data* for *Indirect* references to shared base
        // registers such as RSP and RIP since multiple Data* refer to them by
        // definition and since they don't need to be spilled. The Data* for
        // Direct reference to shared base registers is kept, though.
        template <unsigned SIZE>
        class FreeList
        {
        public:
            // The bit-mask signifying that all valid registers have been allocated.
            static const unsigned c_fullUsageMask = (1ul << SIZE) - 1;

            FreeList();

            // Returns the number of unallocated registers.
            unsigned GetFreeCount() const;

            bool IsAvailable(unsigned id) const;

            unsigned Allocate();

            void Allocate(unsigned id);

            // Returns a pin for a register. Pinned register cannot be spilled.
            // IMPORTANT: Register pinning should be done in a very limited
            // scope. Otherwise, in a larger scope (f. ex. before a CodeGen()
            // call) there is a risk of pinning some of the registers used for
            // parameter passing, which would cause the compilation of a function
            // call to fail.
            ReferenceCounter GetPin(unsigned id);

            // Returns whether a register is pinned.
            bool IsPinned(unsigned id) const;

            void Release(unsigned id);

            // The methods to set and retrieve the Data* owned by the respective
            // register. The InitializeData() and UpdateData() methods differ
            // only in sanity checks: the former requires that no Data* was
            // previously set for the register whereas the latter requires that
            // a Data* already existed.
            void InitializeData(unsigned id, Data* data);
            void UpdateData(unsigned id, Data* data);
            Data* GetData(unsigned id) const;

            // Returns the bit-mask for used and allocated registers.
            unsigned GetUsedMask() const;
            unsigned GetFreeMask() const;

            // Returns the ID of an allocated register that is not pinned and
            // can be spilled. Throws if there are no such registers available.
            unsigned GetAllocatedSpillable() const;

        private:
            // Helper methods to perform sanity check on arguments and data contents.
            void AssertValidID(unsigned id) const;
            void AssertValidData(unsigned id) const;
            void AssertValidData(unsigned id, Data* data) const;

            unsigned m_usageMask;

            // See the class description for more details.
            std::array<Data*, SIZE> m_data;

            // An array of currently allocated IDs, oldest at the beginning.
            // DESIGN NOTE: Deque and list better satisfy this variable's usage
            // pattern in general (elements always added at the back, mostly pulled
            // out from the front). However, given the small number of elements
            // and the simplicitly of the vector, it is likely to perform better.
            std::vector<unsigned __int8> m_allocatedRegisters;

            // Number of active references to a pinned register. The register
            // cannot be spilled while it's pinned.
            std::array<unsigned, SIZE> m_pinCount;
        };


        bool IsBasePointer(PointerRegister r) const;
        PointerRegister GetBasePointer() const;

        // Returns whether the register is one of the reserved/shared base
        // registers (instruction, stack or base pointer).
        template <unsigned SIZE>
        bool IsAnySharedBaseRegister(Register<SIZE, false> r) const;

        template <unsigned SIZE>
        bool IsAnySharedBaseRegister(Register<SIZE, true> r) const;

        void Pass0();
        void Pass1();
        void Pass2();
        void Pass3();
        void Print() const;

        // The following template and the alias template provide a way to access
        // the free list for a register and a C++ type respectively.

        template <bool ISFLOAT>
        class FreeListForRegister;

        template <typename T>
        using FreeListForType = typename FreeListForRegister<RegisterStorage<T>::c_isFloat>;

        std::vector<NodeBase*> m_topologicalSort;
        std::vector<ParameterBase*> m_parameters;
        std::vector<RIPRelativeImmediate*> m_ripRelatives;

        FreeList<RegisterBase::c_maxIntegerRegisterID + 1> m_rxxFreeList;
        FreeList<RegisterBase::c_maxFloatRegisterID + 1> m_xmmFreeList;

        // Note: using void* since there are no floating point reserved registers.
        std::vector<Storage<void*>> m_reservedRegistersStorages;
        std::vector<ReferenceCounter> m_reservedRegistersPins;

        // TODO: Use StlAllocator for the vector.
        unsigned m_temporaryCount;
        std::vector<__int32> m_temporaries;

        PointerRegister m_basePointer;
    };


    class ExpressionTree::Data : public NonCopyable
    {
    public:
        template <unsigned SIZE, bool ISFLOAT>
        Data(ExpressionTree& tree, Register<SIZE, ISFLOAT> r);

        // TODO: Review. Indirect data must know the size of the memory it
        // points to, otherwise access violation can happen if larger, non-owned
        // memory area is dereferenced.
        // Also applies to ConvertDirectToIndirect and Storage::Direct(Register).
        Data(ExpressionTree& tree, PointerRegister r, __int32 offset);

        // Note: attempt to create an immediate storage will static_assert for
        // invalid immediates.
        template <typename T>
        Data(ExpressionTree& tree, T value);

        ExpressionTree& GetTree() const;

        StorageClass GetStorageClass() const;

        unsigned GetRegisterId() const;
        __int32 GetOffset() const;

        // Note: attempt to GetImmediate() will static_assert for invalid immediates.
        template <typename T>
        T GetImmediate() const;

        void ConvertDirectToIndirect(__int32 offset);
        void ConvertIndirectToDirect();

        unsigned GetRefCount() const;
        unsigned Decrement();
        void Increment();

        // Swaps the targets between two Data objects keeping the reference
        // count unchanged and notifies the free list of the register change.
        // Used when all clients of both data objects need to have the contents
        // exchanged.
        void SwapContents(Data* other);

    private:
        // The type of the register change that the free list gets notified about.
        enum class RegisterChangeType { Initialize, Update };

        // Returns true if data's storage class is direct/immediate and if the
        // register it refers to is one of the shared registers.
        bool IsSharedBaseRegister() const;

        // Notifies the free list that register ID of this data object has
        // been modified. Calls the templated version of the same method to
        // peform the actual work.
        void NotifyDataRegisterChange(RegisterChangeType type);

        // See above.
        template <bool ISFLOAT>
        void NotifyDataRegisterChange(RegisterChangeType type);

        ExpressionTree& m_tree;

        // How the data is stored.
        StorageClass m_storageClass;

        // Which register.
        bool m_isFloat;
        unsigned m_registerId;
        __int32 m_offset;

        // Holds the immediate value for Data whose storage class is Immediate.
        size_t m_immediate;

        // Who is using it.
        unsigned m_refCount;
    };


    // Storage should be public because it is a return type for Node.
    // Storage should be private so that its constructor can take an m_data.
    template <typename T>
    class ExpressionTree::Storage
    {
    public:
        // All Storages need to be treated as the same class, regardless of
        // the template parameter.
        template <typename T> friend class Storage;

        typedef typename RegisterStorage<T>::RegisterType DirectRegister;
        typedef PointerRegister BaseRegister;
        typedef typename DirectRegister::FullRegister FullRegister;

        // Types of swaps. The Single swap affects only the two storages directly
        // involved by swapping their Data*. The AllReferences swap modifies the
        // underlying Data objects thus swapping the data for the first storage
        // as well as the storages that share the same Data* with the second
        // storage as well as the storages that share its Data*.
        enum class SwapType { Single, AllReferences };

        Storage();

        template <typename U>
        explicit Storage(const Storage<U>& other);

        Storage(Storage const & other);

        // Takes ownership of a base storage, adds the offset to it and
        // dereferences it to produce a Storage<T>. Equivalent to
        // *static_cast<T*>(base + byteOffset).
        Storage(Storage<void*>&& base, __int32 byteOffset);

        // Creates another storage referencing a register. The register must
        // already be allocated. For shared base registers, the Storage will
        // reference the direct Data version which cannot be converted to
        // indirect nor spilled since it's pinned.
        static Storage<T> ForAdditionalReferenceToRegister(
            ExpressionTree& tree,
            DirectRegister reg);

        // Allocates an empty register and creates a direct storage referencing it.
        // Can only be called when there are available registers.
        static Storage<T> ForAnyFreeRegister(ExpressionTree& tree);

        // Allocates a specific empty register and creates a direct storage
        // referencing it. Can only be called when the specified register is free.
        static Storage<T> ForFreeRegister(ExpressionTree& tree,
                                          DirectRegister reg);

        // Creates an indirect storage against a shared base register.
        static Storage<T> ForSharedBaseRegister(ExpressionTree& tree,
                                                BaseRegister base,
                                                __int32 offset);

        // Creates an immediate storage with the specified value.
        static Storage<T> ForImmediate(ExpressionTree& tree, T value);

        // Loads the address of an immediate storage into a newly created direct
        // storage and resets the original storage. If possible, reuses the
        // register from the source storage.
        // Note: do not call this method with a last reference to a Temporary as
        // the Temporary may be released after the source storage is reset and
        // the created storage would point to invalid the memory.
        template <typename U>
        static Storage<T> AddressOfIndirect(Storage<U>&& indirect);

        Storage& operator=(Storage const & other);

        ~Storage();

        void Reset();
        bool IsNull() const;

        // Returns whether this Storage shares ownership over its Data*.
        // Note that for direct storages, IsSoleDataOwner() result of true
        // implies that the storage also exclusively owns the register. For
        // indirect storages, this implication stands only for non-shared base
        // registers.
        bool IsSoleDataOwner() const;

        StorageClass GetStorageClass() const;

        DirectRegister GetDirectRegister() const;

        BaseRegister GetBaseRegister() const;
        __int32 GetOffset() const;

        // Note: The method must be removed for invalid immediates rather than
        // static_assert triggered because for some of them (arrays) the method
        // has invalid declaration otherwise.
        template <typename U = T,
                  typename ENABLED = typename std::enable_if<ImmediateCategoryOf<U>::value
                                                             == ImmediateCategory::CoreImmediate>::type>
        U GetImmediate() const;

        DirectRegister ConvertToDirect(bool forModification);

        // Swaps the contents of the two storages. See SwapType definition for
        // more details.
        template <typename U>
        void Swap(Storage<U>& other, SwapType type);

        // If the storage is not the sole data owner, copies the data to stack
        // and switches the ownership of other owners to it.
        // Does nothing if the storage was already the sole data owner.
        // Requires that the storage is direct and that it does not refer to
        // one of the shared base registers.
        void TakeSoleOwnershipOfDirect();

        // Returns a pin for the storage's register. While the pin is held,
        // the register cannot be spilled. Can only be called if Storage is
        // either direct or if it's indirect and refers to non-shared base
        // registers.
        ReferenceCounter GetPin();

        // Prints the information about the storage's type, value, base register
        // and offset to the output stream.
        void Print(std::ostream& out) const;

    private:
        // Types used to select the correct flavor of immediate methods. This is
        // necessary because GetStorageClass() is a runtime rather than a compile
        // time property. Even though it is not possible to create a
        // StorageClass::Immediate storage for an invalid immediate, the code
        // that branches depending on various types of storage needs to always
        // compile.
        struct ValidImmediateStorage {};
        struct InvalidImmediateStorage {};
        typedef typename std::conditional<CanBeInImmediateStorage<T>::value,
                                          ValidImmediateStorage,
                                          InvalidImmediateStorage>::type
            ImmediateFlavor;

        Storage(ExpressionTree::Data* data);

        void SetData(ExpressionTree::Data* data);
        void SetData(Storage& other);

        void ConvertImmediateToDirect(bool forModification, ValidImmediateStorage);
        void ConvertImmediateToDirect(bool forModification, InvalidImmediateStorage);

        void PrintImmediate(std::ostream& out, ValidImmediateStorage) const;
        void PrintImmediate(std::ostream& out, InvalidImmediateStorage) const;

        ExpressionTree::Data* m_data;
    };


    template <typename T>
    using Storage = typename ExpressionTree::Storage<T>;


    //*************************************************************************
    //
    // Template definitions for ExpressionTree
    //
    //*************************************************************************
    template <>
    class ExpressionTree::FreeListForRegister<false>
    {
    public:
        static auto Get(ExpressionTree& tree) -> decltype(tree.m_rxxFreeList) &
        {
            return tree.m_rxxFreeList;
        }
    };


    template <>
    class ExpressionTree::FreeListForRegister<true>
    {
    public:
        static auto Get(ExpressionTree& tree) -> decltype(tree.m_xmmFreeList) &
        {
            return tree.m_xmmFreeList;
        }
    };


    template <typename T>
    ExpressionTree::Storage<T> ExpressionTree::Direct()
    {
        auto & freeList = FreeListForType<T>::Get(*this);
        Storage<T> direct;

        if (freeList.GetFreeCount() > 0)
        {
            direct = Storage<T>::ForAnyFreeRegister(*this);
        }
        else
        {
            const unsigned id = freeList.GetAllocatedSpillable();
            direct = Direct<T>(Storage<T>::DirectRegister(id));
        }

        return direct;
    }


    template <typename T>
    ExpressionTree::Storage<T>
    ExpressionTree::Direct(typename Storage<T>::DirectRegister r)
    {
        typedef Storage<T>::FullRegister FullRegister;

        auto & code = GetCodeGenerator();
        auto & freeList = FreeListForType<T>::Get(*this);

        Assert(!IsPinned(r), "Attempted to obtain the pinned register %s", r.GetName());

        unsigned src = r.GetId();

        if (!freeList.IsAvailable(src))
        {
            typedef typename CanonicalRegisterType<FullRegister>::Type FullType;
            // TODO: If the storage is indirect, FullType may not be the correct
            // size. It will still get the right data due to the little endian
            // architecture, but if the process doesn't have access to the
            // additional bytes (f. ex. at the end of the allocated memory
            // block) it will trigger access violation. See also the comment
            // for the indirect Data constructor.
            auto registerStorage = Storage<FullType>
                ::ForAdditionalReferenceToRegister(*this, FullRegister(src));

            // Use another register if available or a temporary otherwise to
            // bump the current contents of the register.
            if (freeList.GetFreeCount() > 0)
            {
                auto destStorage = Storage<FullType>::ForAnyFreeRegister(*this);
                CodeGenHelpers::Emit<OpCode::Mov>(code,
                                                  destStorage.GetDirectRegister(),
                                                  registerStorage);

                // Swap storages for the target storage and for the register
                // (including all the references to it). Once the destStorage
                // variable goes out of scope, the register will be free.
                registerStorage.Swap(destStorage, Storage<FullType>::SwapType::AllReferences);
            }
            else
            {
                // It is not possible to spill an indirect to stack in one
                // step, so ensure that the source storage is direct.
                // If it was previously indirect, its register will be reused
                // because the conversion is not for modification and because
                // the register is not one of the base registers.
                registerStorage.ConvertToDirect(false);
                auto fullReg = registerStorage.GetDirectRegister();
                Assert(fullReg.IsSameHardwareRegister(r),
                       "Converting %s to direct without modification should "
                       "not have moved into a different register (%s)",
                       r.GetName(),
                       fullReg.GetName());

                // Make registerStorage the only owner. After it goes out of
                // scope, the register will be free.
                registerStorage.TakeSoleOwnershipOfDirect();
            }
        }

        return Storage<T>::ForFreeRegister(*this, r);
    }


    template <typename T>
    ExpressionTree::Storage<T> ExpressionTree::RIPRelative(__int32 offset)
    {
        return Storage<T>::ForSharedBaseRegister(*this, rip, offset);
    }


    template <typename T>
    ExpressionTree::Storage<T> ExpressionTree::Temporary()
    {
        static_assert(sizeof(T) <= sizeof(void*),
                      "The size of the variable is too large.");

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

        return Storage<T>::ForSharedBaseRegister(*this, GetBasePointer(), offset);
    }


    template <typename T>
    ExpressionTree::Storage<T> ExpressionTree::Immediate(T value)
    {
        return Storage<T>::ForImmediate(*this, value);
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


    template <unsigned SIZE, bool ISFLOAT>
    bool ExpressionTree::IsPinned(Register<SIZE, ISFLOAT> reg)
    {
        auto & freeList = FreeListForRegister<ISFLOAT>::Get(*this);

        return freeList.IsPinned(reg.GetId());
    }


    template <unsigned SIZE>
    bool ExpressionTree::IsAnySharedBaseRegister(Register<SIZE, false> r) const
    {
        return IsBasePointer(PointerRegister(r))
               || r.IsRIP()
               || r.IsStackPointer();
    }


    template <unsigned SIZE>
    bool ExpressionTree::IsAnySharedBaseRegister(Register<SIZE, true> /* r */) const
    {
        return false;
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
        NotifyDataRegisterChange(RegisterChangeType::Initialize);
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
        static_assert(CanBeInImmediateStorage<T>::value, "Invalid immediate type");
        static_assert(sizeof(T) <= sizeof(m_immediate), "Unsupported type.");
        *reinterpret_cast<T*>(&m_immediate) = value;

        // Note: no need to call NotifyDataRegisterChange() as this constructor
        // doesn't apply to registers.
    }


    template <typename T>
    T ExpressionTree::Data::GetImmediate() const
    {
        static_assert(CanBeInImmediateStorage<T>::value, "Invalid immediate type");
        static_assert(sizeof(T) <= sizeof(m_immediate), "Unsupported type.");
        Assert(m_storageClass == StorageClass::Immediate, "GetImmediate() called for non-immediate storage!");

        return *(reinterpret_cast<T const *>(&m_immediate));
    }


    template <bool ISFLOAT>
    void ExpressionTree::Data::NotifyDataRegisterChange(RegisterChangeType type)
    {
        Assert(m_storageClass != StorageClass::Immediate, "Invalid storage class");
        auto & freeList = FreeListForRegister<ISFLOAT>::Get(m_tree);

        switch (type)
        {
        case RegisterChangeType::Initialize:
            // The free list doesn't need to keep data for indirects relative
            // to shared base registers since there can be many of them for
            // the same register.
            if (!(m_storageClass == StorageClass::Indirect
                  && IsSharedBaseRegister()))
            {
                freeList.InitializeData(m_registerId, this);
            }
            break;

        case RegisterChangeType::Update:
            // Only initialization is allowed for the direct shared registers.
            Assert(!(m_storageClass == StorageClass::Direct
                     && IsSharedBaseRegister()),
                   "Cannot update data for shared register %u",
                   m_registerId);

            // At this point, any reference to shared register is indirect
            // and the free list doesn't need to know about such updates.
            if (!IsSharedBaseRegister())
            {
                freeList.UpdateData(m_registerId, this);
            }

            break;

        default:
            Assert(false, "Unknown register change type %u", type);
            break;
        }
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
    ExpressionTree::Storage<T>::Storage(Storage<void*>&& base,
                                        __int32 byteOffset)
        : m_data(nullptr)
    {
        // Load the base pointer into a register.
        base.ConvertToDirect(true);

        // Dereference it.
        base.m_data->ConvertDirectToIndirect(byteOffset);

        // Transfer ownership of datablock to this Storage.
        SetData(base.m_data);
        base.Reset();
    }


    template <typename T>
    Storage<T> ExpressionTree::Storage<T>::ForAdditionalReferenceToRegister(
            ExpressionTree& tree,
            DirectRegister reg)
    {
        auto & freeList = FreeListForType<T>::Get(tree);
        Assert(!freeList.IsAvailable(reg.GetId()),
               "Register %s must already be allocated",
               reg.GetName());

        return Storage<T>(freeList.GetData(reg.GetId()));
    }


    template <typename T>
    Storage<T> ExpressionTree::Storage<T>::ForAnyFreeRegister(ExpressionTree& tree)
    {
        auto & freeList = FreeListForType<T>::Get(tree);
        Storage<T>::DirectRegister r(freeList.Allocate());

        // TODO: Use allocator.
        Data* data = new Data(tree, r);

        return Storage<T>(data);
    }


    template <typename T>
    Storage<T> ExpressionTree::Storage<T>::ForFreeRegister(ExpressionTree& tree,
                                                           DirectRegister reg)
    {
        auto & freeList = FreeListForType<T>::Get(tree);
        freeList.Allocate(reg.GetId());

        // TODO: Use allocator.
        Data* data = new Data(tree, reg);

        return Storage<T>(data);
    }


    template <typename T>
    Storage<T> ExpressionTree::Storage<T>::ForSharedBaseRegister(
        ExpressionTree& tree,
        BaseRegister base,
        __int32 offset)
    {
        Assert(tree.IsAnySharedBaseRegister(base), "Register %s is not a shared base register", base.GetName());

        // TODO: Use allocator.
        return Storage<T>(new Data(tree, base, offset));
    }


    template <typename T>
    Storage<T> ExpressionTree::Storage<T>::ForImmediate(ExpressionTree& tree, T value)
    {
        // TODO: Use allocator.
        return Storage<T>(new Data(tree, value));
    }


    template <typename T>
    template <typename U>
    Storage<T> ExpressionTree::Storage<T>::AddressOfIndirect(Storage<U>&& indirect)
    {
        static_assert(std::is_pointer<T>::value, "T must be a pointer type");
        static_assert(std::is_same<U, typename std::remove_pointer<T>::type>::value,
                      "U must be the type T points to");

        // The storage being created.
        Storage<T> target;

        // Source information.
        const auto base = indirect.GetBaseRegister();
        const auto offset = indirect.GetOffset();

        auto & tree = indirect.m_data->GetTree();
        auto & code = tree.GetCodeGenerator();

        // Reuse the base register if possible. Since the indirect argument
        // is giving away the ownership, the register can be reused if that's
        // the only reference to it and if it's not one of the special
        // reserved/shared registers. Since T is a pointer type, the register
        // it needs is compatible to indirect's base PointerRegister.
        if (indirect.IsSoleDataOwner()
            && !tree.IsAnySharedBaseRegister(base))
        {
            // Get the address and convert the storage to indirect.
            code.Emit<OpCode::Lea>(base, base, offset);

            indirect.m_data->ConvertIndirectToDirect();
            target.SetData(indirect.m_data);
        }
        else
        {
            // Get a pin for the base register to make sure it doesn't get spilled
            // by the target register allocation.
            ReferenceCounter basePin = indirect.GetPin();
            target = tree.Direct<T>();
            code.Emit<OpCode::Lea>(target.GetDirectRegister(), base, offset);
        }

        // Take away ownership from the indirect storage.
        indirect.Reset();

        return target;
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
    bool ExpressionTree::Storage<T>::IsSoleDataOwner() const
    {
        return IsNull() || m_data->GetRefCount() == 1;
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
    template <typename U, typename ENABLED>
    U ExpressionTree::Storage<T>::GetImmediate() const
    {
        static_assert(std::is_same<T, U>::value, "U must not be specified explicitly");
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

        switch (m_data->GetStorageClass())
        {
        case StorageClass::Immediate:
            ConvertImmediateToDirect(forModification, ImmediateFlavor());
            break;

        case StorageClass::Direct:
            // If the storage is already direct, action is only necessary if it
            // needs to be modified but we're not the sole owner.
            if (!IsSoleDataOwner() && forModification)
            {
                auto dest = tree.Direct<T>();

                // There's a possibility that the current register may get spilled
                // by the allocation, so move from Storage instead of from register.
                CodeGenHelpers::Emit<OpCode::Mov>(code,
                                                  dest.GetDirectRegister(),
                                                  *this);
                SetData(dest);
            }
            break;

        case StorageClass::Indirect:
            {
                BaseRegister base = GetBaseRegister();

                // If we either fully own the storage or don't plan to make
                // modifications, the type of the base register is compatible
                // and it's not one of the reserved (shared) registers, then
                // the base register can be reused.
                if ((IsSoleDataOwner() || !forModification)
                    && BaseRegister::c_isFloat == DirectRegister::c_isFloat
                    && !tree.IsAnySharedBaseRegister(base))
                {
                    code.Emit<OpCode::Mov>(DirectRegister(base), base, GetOffset());
                    m_data->ConvertIndirectToDirect();
                }
                else
                {
                    // Otherwise, move the data to a newly allocated register,
                    // making sure that the base register doesn't get spilled.
                    Storage<T> dest;
                    {
                        ReferenceCounter basePin = GetPin();
                        dest = tree.Direct<T>();
                    }

                    code.Emit<OpCode::Mov>(dest.GetDirectRegister(), base, GetOffset());

                    // Let every owner benefit from moving to direct storage if
                    // possible. This is also necessary for the register to be
                    // fully released during spilling.
                    Swap(dest, forModification
                               ? Storage<T>::SwapType::Single
                               : Storage<T>::SwapType::AllReferences);
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
    void ExpressionTree::Storage<T>::ConvertImmediateToDirect(bool forModification, ValidImmediateStorage)
    {
        Assert(m_data->GetStorageClass() == StorageClass::Immediate, "Unexpected storage class");

        auto & tree = m_data->GetTree();
        auto & code = tree.GetCodeGenerator();

        // Allocate a register and load the immediate into it.
        auto dest = tree.Direct<T>();
        code.EmitImmediate<OpCode::Mov>(dest.GetDirectRegister(), m_data->GetImmediate<T>());

        // Let every owner benefit from moving to direct storage if possible.
        Swap(dest, forModification
                   ? Storage<T>::SwapType::Single
                   : Storage<T>::SwapType::AllReferences);
    }


    template <typename T>
    void ExpressionTree::Storage<T>::ConvertImmediateToDirect(bool /* forModification */, InvalidImmediateStorage)
    {
        // This should never be hit, it's impossible to compile an invalid immediate
        // with StorageClass::Immediate.
        Assert(m_data->GetStorageClass() == StorageClass::Immediate, "Unexpected storage class");
        Assert(false, "Unexpected occurrence of an invalid immediate storage");
    }


    template <typename T>
    template <typename U>
    void ExpressionTree::Storage<T>::Swap(Storage<U>& other, SwapType type)
    {
        switch (type)
        {
        case SwapType::Single:
            std::swap(m_data, other.m_data);
            break;

        case SwapType::AllReferences:
            m_data->SwapContents(other.m_data);
            break;

        default:
            Assert(false, "Unknown swap type %u", type);
            break;
        }
    }


    template <typename T>
    void ExpressionTree::Storage<T>::TakeSoleOwnershipOfDirect()
    {
        auto & tree = m_data->GetTree();

        Assert(GetStorageClass() == StorageClass::Direct,
               "Unexpected storage class %u",
               GetStorageClass());
        Assert(!tree.IsAnySharedBaseRegister(GetDirectRegister()),
               "Cannot take sole ownership of %s",
               GetDirectRegister().GetName());

        if (!IsSoleDataOwner())
        {
            auto & freeList = FreeListForType<T>::Get(tree);
            auto & code = tree.GetCodeGenerator();
            typedef typename CanonicalRegisterType<FullRegister>::Type FullType;

            // Use another register if available or a temporary otherwise to
            // bump the full contents of the register.
            Storage<FullType> destStorage
                = freeList.GetFreeCount() > 0
                  ? Storage<FullType>::ForAnyFreeRegister(tree)
                  : tree.Temporary<FullType>();

            CodeGenHelpers::Emit<OpCode::Mov>(code,
                                              destStorage,
                                              FullRegister(GetDirectRegister().GetId()));

            // After the swap, the destStorage variable will be the only one
            // still referring to the original register.
            Swap(destStorage, SwapType::AllReferences);

            // Make this instance another owner of the register. The other one
            // in stackStorage will go away after the closing brace.
            *this = Storage<T>(destStorage);
        }
    }


    template <typename T>
    ReferenceCounter ExpressionTree::Storage<T>::GetPin()
    {
        Assert(GetStorageClass() != StorageClass::Immediate,
               "Cannot pin a register for immediate storage");

        if (GetStorageClass() == StorageClass::Direct)
        {
            auto & freeList = FreeListForType<T>::Get(m_data->GetTree());

            return freeList.GetPin(m_data->GetRegisterId());
        }
        else
        {
            auto & freeList = FreeListForRegister<false>::Get(m_data->GetTree());

            return freeList.GetPin(m_data->GetRegisterId());
        }
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
                        else if (base.IsStackPointer())
                        {
                            // Note: this branch should be after the IsBasePointer()
                            // branch to also cover the cases when stack pointer
                            // is chosen to be the base pointer.
                            std::cout << "Don't release stack-relative indirect at "
                                      << m_data->GetOffset() << std::endl;
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
    void ExpressionTree::Storage<T>::PrintImmediate(std::ostream& out, ValidImmediateStorage) const
    {
        out << "immediate value " << std::hex << GetImmediate() << "h" << std::dec;
    }


    template <typename T>
    void ExpressionTree::Storage<T>::PrintImmediate(std::ostream& /* out */, InvalidImmediateStorage) const
    {
        // This should never be hit, it's impossible to compile an invalid immediate
        // with StorageClass::Immediate.
        Assert(m_data->GetStorageClass() == StorageClass::Immediate, "Unexpected storage class");
        Assert(false, "Unexpected occurrence of an invalid immediate storage");
    }


    //*************************************************************************
    //
    // template definitions for FreeList
    //
    //*************************************************************************
    template <unsigned SIZE>
    ExpressionTree::FreeList<SIZE>::FreeList()
        : m_usageMask(0),
          m_data(),
          m_pinCount()
    {
        m_allocatedRegisters.reserve(SIZE);
    }


    template <unsigned SIZE>
    void ExpressionTree::FreeList<SIZE>::AssertValidID(unsigned id) const
    {
        Assert(id < SIZE, "Register %u is out of range.", id);
    }


    template <unsigned SIZE>
    void ExpressionTree::FreeList<SIZE>::AssertValidData(unsigned id) const
    {
        AssertValidID(id);
        AssertValidData(id, m_data[id]);
    }


    template <unsigned SIZE>
    void ExpressionTree::FreeList<SIZE>::AssertValidData(unsigned id, Data* data) const
    {
        Assert(data != nullptr, "Unexpected null data at/intended for register %u", id);
        Assert(data->GetStorageClass() != StorageClass::Immediate,
               "Invalid storage class %u for data at/intended for register %u",
               data->GetStorageClass(),
               id);
        Assert(data->GetRegisterId() == id,
               "Mismatched register ID %u for data at/intended for register %u",
               data->GetRegisterId(),
               id);
    }


    template <unsigned SIZE>
    unsigned ExpressionTree::FreeList<SIZE>::GetFreeCount() const
    {
        return BitOp::GetNonZeroBitCount(GetFreeMask());
    }


    template <unsigned SIZE>
    bool ExpressionTree::FreeList<SIZE>::IsAvailable(unsigned id) const
    {
        AssertValidID(id);

        return BitOp::TestBit(GetFreeMask(), id);
    }


    template <unsigned SIZE>
    unsigned ExpressionTree::FreeList<SIZE>::Allocate()
    {
        unsigned id;

        const bool registerFound = BitOp::GetHighestBitSet(GetFreeMask(), &id);
        Assert(registerFound, "No free registers available");

        Allocate(id);

        return id;
    }


    // TODO: Perhaps this should return a Data*.
    // If nothing is displaced, returns nullptr.
    // Otherwise returns Data* for displaced.
    template <unsigned SIZE>
    void ExpressionTree::FreeList<SIZE>::Allocate(unsigned id)
    {
        AssertValidID(id);
        Assert(BitOp::TestBit(GetFreeMask(), id), "Register %u must be free", id);
        Assert(!IsPinned(id), "Register %u must be unpined when free", id);
        Assert(m_data[id] == nullptr, "Data for register %u must be null", id);

        BitOp::SetBit(&m_usageMask, id);
        m_allocatedRegisters.push_back(static_cast<unsigned __int8>(id));
    }


    template <unsigned SIZE>
    void ExpressionTree::FreeList<SIZE>::Release(unsigned id)
    {
        AssertValidID(id);
        Assert(BitOp::TestBit(GetUsedMask(), id), "Register %u must be allocated", id);
        Assert(!IsPinned(id), "Register %u must be unpinned before release", id);
        Assert(m_data[id] != nullptr, "Data for register %u must not be null", id);
        Assert(m_data[id]->GetRefCount() == 0, "Reference count for register %u must be zero", id);
        AssertValidData(id);

        m_data[id] = nullptr; 
        BitOp::ClearBit(&m_usageMask, id);

        auto it = std::find(m_allocatedRegisters.begin(),
                            m_allocatedRegisters.end(),
                            static_cast<unsigned __int8>(id));
        Assert(it != m_allocatedRegisters.end(), "Couldn't find allocation record for %u", id);
        m_allocatedRegisters.erase(it);
    }


    template <unsigned SIZE>
    void ExpressionTree::FreeList<SIZE>::InitializeData(unsigned id, Data* data)
    {
        AssertValidID(id);
        Assert(m_data[id] == nullptr, "Data for register %u must be clear", id);
        AssertValidData(id, data);

        m_data[id] = data;
    }


    template <unsigned SIZE>
    void ExpressionTree::FreeList<SIZE>::UpdateData(unsigned id, Data* data)
    {
        AssertValidID(id);
        Assert(m_data[id] != nullptr, "Data for register %u must not be clear", id);
        AssertValidData(id, data);

        m_data[id] = data;
    }


    template <unsigned SIZE>
    ExpressionTree::Data* ExpressionTree::FreeList<SIZE>::GetData(unsigned id) const
    {
        AssertValidData(id);

        return m_data[id];
    }


    template <unsigned SIZE>
    unsigned ExpressionTree::FreeList<SIZE>::GetUsedMask() const
    {
        return m_usageMask;
    }


    template <unsigned SIZE>
    unsigned ExpressionTree::FreeList<SIZE>::GetFreeMask() const
    {
        return ~m_usageMask & c_fullUsageMask;
    }


    template <unsigned SIZE>
    unsigned ExpressionTree::FreeList<SIZE>::GetAllocatedSpillable() const
    {
        unsigned numPinned = 0;

        // Start looking from the oldest allocated register. This is expected
        // to give best results as recently allocated registers are more likely
        // to be needed in the code that's currently being compiled.
        for (unsigned id : m_allocatedRegisters)
        {
            if (IsPinned(id))
            {
                numPinned++;
            }
            else
            {
                return id;
            }

        }

        throw std::runtime_error("Couldn't find any registers for spilling: "
                                 + std::to_string(m_allocatedRegisters.size())
                                 + " registers allocated, "
                                 + std::to_string(numPinned)
                                 + " of those are pinned");
    }

    template <unsigned SIZE>
    ReferenceCounter
    ExpressionTree::FreeList<SIZE>::GetPin(unsigned id)
    {
        AssertValidID(id);
        Assert(BitOp::TestBit(GetUsedMask(), id), "Register %u must be allocated to be pinned", id);
        AssertValidData(id);

        return ReferenceCounter(m_pinCount[id]);
    }


    template <unsigned SIZE>
    bool ExpressionTree::FreeList<SIZE>::IsPinned(unsigned id) const
    {
        AssertValidID(id);

        return m_pinCount[id] != 0;
    }
}
