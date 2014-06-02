#pragma once

#include "Assert.h"
#include "Storage.h"
#include "TypePredicates.h"


namespace NativeJIT
{
    class ExpressionTree;

    class NodeBase
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
        virtual unsigned __int64 GetOffset() const;
        virtual bool IsFieldPointer() const;

        //
        // Pure virtual methods.
        //

        virtual void CodeGenCache(ExpressionTree& tree) = 0;
        virtual bool IsCached2() const = 0;
        virtual unsigned LabelSubtree(bool isLeftChild) = 0;
        virtual void Print() const = 0;


    protected:
        unsigned m_parentCount;

    private:
        unsigned m_id;
    };


    template <typename T>
    class Node : public NodeBase
    {
    public:
        typedef Register<sizeof(T), IsFloatingPointType<T>::value> RegisterType;

        Node(ExpressionTree& tree);

        void SetCache2(Storage<T> s);
        Storage<T> GetCache2();
        void ReleaseCache2();

        unsigned GetRegisterCount() const;

        virtual Storage<T> CodeGenValue2(ExpressionTree& tree) = 0;

        //
        // Overrides of NodeBase methods.
        //

        virtual void CodeGenCache(ExpressionTree& tree) override;
        virtual bool IsCached2() const override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;

    protected:
        void SetRegisterCount(unsigned count);
        void PrintRegisterAndCacheInfo() const;

    private:
        bool m_isCached;
        unsigned m_cacheReferenceCount;
        Storage<T> m_cache;

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
    void Node<T>::SetCache2(Storage<T> s)
    {
        Assert(!IsCached2(), "Cache register is already set.");

        m_cacheReferenceCount = m_parentCount;
        m_cache = s;
    }


    template <typename T>
    Storage<T> Node<T>::GetCache2()
    {
        return m_cache;
    }


    template <typename T>
    void Node<T>::ReleaseCache2()
    {
        Assert(IsCached2(), "Cache register has not been set.");

        --m_cacheReferenceCount;
        if (m_cacheReferenceCount == 0)
        {
            m_cache.Reset();
        }
    }


    template <typename T>
    bool Node<T>::IsCached2() const
    {
        return !m_cache.IsNull();
    }



    template <typename T>
    void Node<T>::PrintRegisterAndCacheInfo() const
    {
        std::cout << "register count = " << m_registerCount;
        std::cout << ", ";

        if (IsCached2())
        {
            std::cout << "cached in " << m_cache.GetDirectRegister().GetName();
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
        //SetCacheRegister(CodeGenValue(tree));
        SetCache2(CodeGenValue2(tree));
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
        if (IsCached2())
        {
            m_registerCount = 0;
        }
        else
        {
            m_registerCount = count;
        }
    }
}
