#pragma once

// Disable warning: 'function' : unreferenced local function has been removed
#pragma warning(disable:4505)

#include <cstdint>
#include <iosfwd>   // Debugging output.

#include "NativeJIT/ExpressionTree.h"             // ExpressionTree::Storage<T> return type.
#include "NativeJIT/TypePredicates.h"
#include "Temporary/Assert.h"
#include "Temporary/NonCopyable.h"


namespace NativeJIT
{
    class ExpressionTree;

    // Used in NodeBase as an argument.
    template <typename T>
    class Node;

    class NodeBase : public NonCopyable
    {
    public:
        NodeBase(ExpressionTree& tree);

        unsigned GetId() const;

        // Increments the number of node's parents that will evaluate the node
        // through the Node<T>::CodeGen() method. The node will be evaluated
        // only once, but the result will also be stored in cache with a
        // matching number of references. The cache will be released once all
        // parents evaluate the node.
        // IMPORTANT: Currently, there's an assumption that if a node is created,
        // it must be placed inside the tree. TODO: Remove this assumption and
        // allow for optimizing away unused nodes.
        void IncrementParentCount();

        // Decrements the number of node's parents as set through
        // IncrementParentCount(). Used only when nodes are optimized away.
        void DecrementParentCount();

        unsigned GetParentCount() const;

        // Returns whether the node has been evaluated through the Node<T>::CodeGen
        // method.
        bool HasBeenEvaluated() const;
        void MarkEvaluated();

        // Returns whether the node is referenced. Node is considered to be
        // referenced if it has parents which will call its CodeGen() method
        // or through some alternate method. The IncrementParentCount() call
        // which implies the CodeGen() call will implicitly mark the node as
        // referenced. Parents that use the node through some other means
        // need to call MarkReferenced() explicitly.
        bool IsReferenced() const;
        void MarkReferenced();

        //
        // Non-pure virtual methods.
        //

        virtual void CompileAsRoot(ExpressionTree& tree);

        // Returns whether the node can be optimized away. Only nodes that have
        // no parents and are not used through some node-specific means can
        // be optimized away.
        virtual bool CanBeOptimizedAway() const;

        // When the node is optimized away, instructs it to undo the
        // IncreaseParentCount() calls it made in its constructor. Default
        // implementation throws.
        virtual void ReleaseReferencesToChildren();

        // For nodes that represent objects generated off of another base object
        // with an added offset, populates the base and offset out parameters
        // and returns true. Otherwise leaves the out parameters unchanged and
        // returns false (default implementation).
        // This allows to optimize a chain of references to the same object
        // by collapsing them to the topmost base object and adding the offsets
        // in the chain together.
        // Callers that override this method also need to override
        // ReleaseReferencesToChildren().
        virtual bool GetBaseAndOffset(NodeBase*& base, int32_t& offset) const;

        //
        // Pure virtual methods.
        //

        virtual void CodeGenCache(ExpressionTree& tree) = 0;
        virtual bool IsCached() const = 0;

        // Evaluates the node and regardless of its type returns a void* Storage.
        // This method is equivalent to Node<T>::CodeGen() with type erasure.
        virtual Storage<void*> CodeGenAsBase(ExpressionTree& tree) = 0;

        virtual unsigned LabelSubtree(bool isLeftChild) = 0;
        virtual void Print(std::ostream& out) const = 0;
        
    protected:
        // WARNING: This class is designed to be allocated by an arena allocator,
        // so its destructor will never be called. Therefore, it should hold no
        // resources other than memory from the arena allocator.
        // Would like to make this private and without method body, but it's
        // called implicitly by child class constructors if they throw.
        // Making it non-virtual because nothing should be deleting at all,
        // let alone through base pointer.
        /* virtual */ ~NodeBase() {}

    protected:
        // Calculates the number of registers needed by a node whose left
        // and right subtree require a certain number of registers according
        // to Sethi-Ullman algorithm.
        static unsigned ComputeRegisterCount(unsigned leftTreeCount, unsigned rightTreeCount);

        // Invokes CodeGen() method on both methods and assigns the result to the
        // matching storage. The order of CodeGen() calls is determined by the
        // estimated number of registers used: the node with higher requirements
        // will be evaluated first to minimize spilling when the second node
        // is evaluated.
        template <typename T1, typename T2>
        static void CodeGenInPreferredOrder(ExpressionTree& tree,
                                            Node<T1>& n1, Storage<T1>& s1,
                                            Node<T2>& n2, Storage<T2>& s2);

    private:
        unsigned m_id;

        // See the comments for the related accessor methods above for more information.
        unsigned m_parentCount;
        bool m_isReferenced;
        bool m_hasBeenEvaluated;
    };


    template <typename T>
    class Node : public NodeBase
    {
    public:
        typedef typename ExpressionTree::Storage<T>::DirectRegister RegisterType;

        Node(ExpressionTree& tree);

        // Returns the estimated number of registers needed to evaluate this
        // node. Used to determine preferrable order of node evaluation according
        // to the Sethi-Ullman algorithm.
        //
        // A negative value specifies that the number has not been calculated
        // yet. The LabelSubtree() method calculates the number of registers and
        // saves it using the SetRegisterCount() call.
        int GetRegisterCount() const;

        ExpressionTree::Storage<T> CodeGen(ExpressionTree& tree);

        //
        // Overrides of NodeBase methods.
        //

        virtual void CodeGenCache(ExpressionTree& tree) override;
        virtual bool IsCached() const override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;

    protected:
        // WARNING: This class is designed to be allocated by an arena allocator,
        // so its destructor will never be called. Therefore, it should hold no
        // resources other than memory from the arena allocator.
        // Would like to make this private and without method body, but it's
        // called implicitly by child class constructors if they throw.
        ~Node() {}

    protected:
        void SetRegisterCount(unsigned count);
        void PrintCoreProperties(std::ostream& out, char const *nodeName) const;

    private:
        unsigned m_cacheReferenceCount;
        ExpressionTree::Storage<T> m_cache;

        // See GetRegisterCount() method.
        int m_registerCount;

        virtual Storage<T> CodeGenValue(ExpressionTree& tree) = 0;
        virtual Storage<void*> CodeGenAsBase(ExpressionTree& tree) override;

        void SetCache(ExpressionTree::Storage<T> s);
        Storage<T> GetAndReleaseCache();
    };


    //*************************************************************************
    //
    // Template definitions for NodeBase
    //
    //*************************************************************************

    template <typename T1, typename T2>
    void NodeBase::CodeGenInPreferredOrder(ExpressionTree& tree,
                                           Node<T1>& n1, Storage<T1>& s1,
                                           Node<T2>& n2, Storage<T2>& s2)
    {
        // Evaluate the expression which uses more registers first.
        if (n1.GetRegisterCount() >= n2.GetRegisterCount())
        {
            s1 = n1.CodeGen(tree);
            s2 = n2.CodeGen(tree);
        }
        else
        {
            s2 = n2.CodeGen(tree);
            s1 = n1.CodeGen(tree);
        }
    }

    //*************************************************************************
    //
    // Template definitions for Node
    //
    //*************************************************************************

    template <typename T>
    Node<T>::Node(ExpressionTree& tree)
        : NodeBase(tree),
          m_cacheReferenceCount(0),
          m_registerCount(-1)
    {
    }


    template <typename T>
    void Node<T>::SetCache(ExpressionTree::Storage<T> s)
    {
        LogThrowAssert(!IsCached(), "Cache is already set for node with ID %u", GetId());
        LogThrowAssert(GetParentCount() > 0, "Cannot set cache for node %u with zero parents", GetId());

        m_cacheReferenceCount = GetParentCount();
        m_cache = s;
    }


    template <typename T>
    Storage<T> Node<T>::GetAndReleaseCache()
    {
        LogThrowAssert(IsCached(), "Cache has not been set for node ID %u", GetId());

        auto result = m_cache;
        --m_cacheReferenceCount;

        if (m_cacheReferenceCount == 0)
        {
            m_cache.Reset();
        }

        return result;
    }


    template <typename T>
    bool Node<T>::IsCached() const
    {
        // If cache is present, reference count cannot be zero. Similarly, when
        // the cache is empty, reference count must be zero.
        LogThrowAssert(m_cache.IsNull() == (m_cacheReferenceCount == 0),
                       "Mismatch in cached storage and cache reference count: "
                       "have cache %u, reference count: %u",
                       m_cache.IsNull(),
                       m_cacheReferenceCount);

        return !m_cache.IsNull();
    }


    template <typename T>
    void Node<T>::PrintCoreProperties(std::ostream& out, char const* nodeName) const
    {
        out << nodeName
            << " [id = " << GetId()
            << ", parents = " << GetParentCount()
            << ", register count = " << m_registerCount
            << ", ";

        if (IsCached())
        {
            out << "cached in ";
            m_cache.Print(out);
        }
        else
        {
            out << "not cached";
        }

        out << "]";
    }


    template <typename T>
    void Node<T>::CodeGenCache(ExpressionTree& tree)
    {
        LogThrowAssert(GetParentCount() > 0,
                       "Cannot evaluate node %u with no parents",
                       GetId());
        LogThrowAssert(!HasBeenEvaluated(),
                       "Tried to CodeGenValue() node with ID %u more than once",
                       GetId());
        MarkEvaluated();

        LabelSubtree(true);
        SetCache(CodeGenValue(tree));
    }


    template <typename T>
    unsigned Node<T>::LabelSubtree(bool isLeftChild)
    {
        if (GetRegisterCount() < 0)
        {
            if (isLeftChild)
            {
                SetRegisterCount(1);
            }
            else
            {
                SetRegisterCount(0);
            }
        }

        // WARNING: GetRegisterCount() may return a different value than passed to SetRegisterCount().
        return GetRegisterCount();
    }


    template <typename T>
    int Node<T>::GetRegisterCount() const
    {
        return m_registerCount;
    }


    template <typename T>
    void Node<T>::SetRegisterCount(unsigned count)
    {
        if (IsCached())
        {
            m_registerCount = 0;
        }
        else
        {
            m_registerCount = count;
        }
    }


    template <typename T>
    typename ExpressionTree::Storage<T> Node<T>::CodeGen(ExpressionTree& tree)
    {
        if (!IsCached())
        {
            CodeGenCache(tree);
        }

        return GetAndReleaseCache();
    }


    // Calls LogThrowAbort() if a constant condition is false.
    template <bool CONDITION>
    void AssertAtRuntime(char const * message);


    template <>
    inline void AssertAtRuntime<false>(char const * message)
    {
        LogThrowAbort(message);
    }


    template <>
    inline void AssertAtRuntime<true>(char const * /* message */)
    {
    }


    template <typename T>
    ExpressionTree::Storage<void*> Node<T>::CodeGenAsBase(ExpressionTree& tree)
    {
        // Use this rather than tag dispatch since CodeGenAsBase is a virtual method
        // so the method signature at base class would have to be changed as well.
        AssertAtRuntime<!RegisterStorage<T>::c_isFloat
                        && RegisterStorage<T>::c_size == sizeof(void*)>(
            "Invalid call to CodeGenAsBase");

        return ExpressionTree::Storage<void*>(CodeGen(tree));
    }
}
