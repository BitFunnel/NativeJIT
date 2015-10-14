#pragma once

#pragma warning(disable:4505)

#include "ExpressionTree.h"             // ExpressionTree::Storage<T> return type.
#include "Temporary/Assert.h"
#include "Temporary/NonCopyable.h"
#include "TypePredicates.h"


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

        void IncrementParentCount();
        unsigned GetParentCount() const;

        //
        // Non-pure virtual methods.
        //

        virtual void CompileAsRoot(ExpressionTree& tree);

        // See Node<T>::CodeGenBase() for more information.
        virtual __int32 GetOffset() const;

        //
        // Pure virtual methods.
        //

        virtual void CodeGenCache(ExpressionTree& tree) = 0;
        virtual ExpressionTree::Storage<void*> CodeGenBase(ExpressionTree& tree) = 0;
        virtual bool IsCached() const = 0;
        virtual unsigned LabelSubtree(bool isLeftChild) = 0;
        virtual void Print() const = 0;


    protected:
        unsigned m_parentCount;

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
    };


    template <typename T>
    class Node : public NodeBase
    {
    public:
        typedef typename ExpressionTree::Storage<T>::DirectRegister RegisterType;

        Node(ExpressionTree& tree);

        void SetCache(ExpressionTree::Storage<T> s);
        ExpressionTree::Storage<T> GetCache();
        void ReleaseCache();

        unsigned GetRegisterCount() const;

        ExpressionTree::Storage<T> CodeGen(ExpressionTree& tree);

        // Generates the code to evaluate the node when T is a X* used
        // as a base pointer. The returned Storage points to some unknown type,
        // but once the GetOffset() value is added, it will turn into a
        // Storage<X*>. Optimizations of chained base pointer accesses within
        // the same object are possible by overriding this method, such as in
        // FieldPointerNode. The default implementation calls CodeGen() and
        // returns 0 for GetOffset(), which provides the correct behavior in
        // all cases, but less optimal in cases when optimizations are possible.
        virtual ExpressionTree::Storage<void*> CodeGenBase(ExpressionTree& tree);

        virtual ExpressionTree::Storage<T> CodeGenValue(ExpressionTree& tree) = 0;

        //
        // Overrides of NodeBase methods.
        //

        virtual void CodeGenCache(ExpressionTree& tree) override;
        virtual bool IsCached() const override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;

    protected:
        void SetRegisterCount(unsigned count);
        void PrintRegisterAndCacheInfo() const;

    private:
        bool m_isCached;
        unsigned m_cacheReferenceCount;
        ExpressionTree::Storage<T> m_cache;

        unsigned m_registerCount;
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
          m_isCached(false),
          m_cacheReferenceCount(0),
          m_registerCount(0)
    {
    }


    template <typename T>
    void Node<T>::SetCache(ExpressionTree::Storage<T> s)
    {
        Assert(!IsCached(), "Cache register is already set.");

        m_cacheReferenceCount = m_parentCount;
        m_cache = s;
    }


    template <typename T>
    ExpressionTree::Storage<T> Node<T>::GetCache()
    {
        return m_cache;
    }


    template <typename T>
    void Node<T>::ReleaseCache()
    {
        Assert(IsCached(), "Cache register has not been set.");

        --m_cacheReferenceCount;
        if (m_cacheReferenceCount == 0)
        {
            m_cache.Reset();
        }
    }


    template <typename T>
    bool Node<T>::IsCached() const
    {
        return !m_cache.IsNull();
    }



    template <typename T>
    void Node<T>::PrintRegisterAndCacheInfo() const
    {
        std::cout << "register count = " << m_registerCount;
        std::cout << ", ";

        if (IsCached())
        {
            std::cout << "cached in ";
            m_cache.Print(std::cout);
        }
        else
        {
            std::cout << "not cached";
        }
    }


    template <typename T>
    void Node<T>::CodeGenCache(ExpressionTree& tree)
    {
        LabelSubtree(true);
        SetCache(CodeGenValue(tree));
    }


    template <typename T>
    unsigned Node<T>::LabelSubtree(bool isLeftChild)
    {
        if (isLeftChild)
        {
            SetRegisterCount(1);
        }
        else
        {
            SetRegisterCount(0);
        }

        // WARNING: GetRegisterCount() may return a different value than passed to SetRegisterCount().
        return GetRegisterCount();
    }


    template <typename T>
    unsigned Node<T>::GetRegisterCount() const
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
        if (IsCached())
        {
            auto result = GetCache();
            ReleaseCache();
            return result;
        }
        else
        {
            return CodeGenValue(tree);
        }
    }


    template <typename T>
    ExpressionTree::Storage<void*> Node<T>::CodeGenBase(ExpressionTree& tree)
    {
        return ExpressionTree::Storage<void*>(CodeGen(tree));
    }
}
