#pragma once

#pragma warning(disable:4505)

#include "ExpressionTree.h"             // ExpressionTree::Storage<T> return type.
#include "Temporary/Assert.h"
#include "Temporary/NonCopyable.h"
#include "TypePredicates.h"


namespace NativeJIT
{
    class ExpressionTree;

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
        virtual __int32 GetOffset() const;
        virtual bool IsFieldPointer() const;

        //
        // Pure virtual methods.
        //

        virtual void CodeGenCache(ExpressionTree& tree) = 0;
        virtual bool IsCached() const = 0;
        virtual unsigned LabelSubtree(bool isLeftChild) = 0;
        virtual void Print() const = 0;


    protected:
        unsigned m_parentCount;

        // Calculates the number of registers needed by a node whose left
        // and right subtree require a certain number of registers according
        // to Sethi-Ullman algorithm.
        static unsigned ComputeRegisterCount(unsigned leftTreeCount, unsigned rightTreeCount);

    private:
        unsigned m_id;
    };


    template <typename T>
    class Node : public NodeBase
    {
    public:
        typedef Register<sizeof(T), IsFloatingPointType<T>::value> RegisterType;

        Node(ExpressionTree& tree);

        void SetCache(ExpressionTree::Storage<T> s);
        ExpressionTree::Storage<T> GetCache();
        void ReleaseCache();

        unsigned GetRegisterCount() const;

        ExpressionTree::Storage<T> CodeGen(ExpressionTree& tree);
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
}
