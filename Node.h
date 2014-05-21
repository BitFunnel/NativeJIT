#pragma once

#include "Assert.h"
#include "Register.h"


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
        virtual bool IsCached() const = 0;
        virtual bool IsImmediate() const = 0;
        virtual bool IsIndirect() const = 0;
        virtual unsigned LabelSubtree(bool isLeftChild) = 0;
        virtual void Print() const = 0;


    protected:
        unsigned m_parentCount;

    private:
        unsigned m_id;
    };


    template <typename T> struct IsFloatingPointType : std::false_type {};
    template <> struct IsFloatingPointType<float> : std::true_type {};
    template <> struct IsFloatingPointType<double> : std::true_type {};


    template <typename T>
    class Node : public NodeBase
    {
    public:
        typedef Register<sizeof(T), IsFloatingPointType<T>::value> RegisterType;

        Node(ExpressionTree& tree);

        void SetCacheRegister(RegisterType r);
        void ReleaseCache();
        RegisterType GetCacheRegister() const;

        virtual RegisterType CodeGenValue(ExpressionTree& tree) = 0;


        //
        // Overrides of NodeBase methods.
        //

        virtual void CodeGenCache(ExpressionTree& tree) override;
        virtual bool IsCached() const override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;

    protected:
        unsigned GetRegisterCount() const;
        void SetRegisterCount(unsigned count);
        void PrintRegisterAndCacheInfo() const;

    private:
        bool m_isCached;
        unsigned m_cacheReferenceCount;
        RegisterType m_cacheRegister;

        unsigned m_registerCount;
    };


    template <typename T>
    class DirectValue : public Node<T>
    {
    public:
        DirectValue(ExpressionTree& tree);

        //
        // Overrides of Node methods
        //
        virtual bool IsImmediate() const override;
        virtual bool IsIndirect() const override;
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
          m_cacheRegister(0),
          m_registerCount(0)
    {
    }


    template <typename T>
    void Node<T>::SetCacheRegister(RegisterType r)
    {
        Assert(!IsCached(), "Cache register is already set.");

        m_cacheReferenceCount = m_parentCount;
        m_cacheRegister = r;
        m_isCached = true;
    }


    template <typename T>
    void Node<T>::ReleaseCache()
    {
        Assert(IsCached(), "Cache register has not been set.");

        --m_cacheReferenceCount;
        if (m_cacheReferenceCount == 0)
        {
            m_isCached = false;
        }
    }


    template <typename T>
    bool Node<T>::IsCached() const
    {
        return m_isCached;
    }


    template <typename T>
    typename Node<T>::RegisterType Node<T>::GetCacheRegister() const
    {
        Assert(IsCached(), "Node is not cached.");

        return m_cacheRegister;
    }


    template <typename T>
    void Node<T>::PrintRegisterAndCacheInfo() const
    {
        std::cout << "register count = " << m_registerCount;
        std::cout << ", ";

        if (IsCached())
        {
            std::cout << "cached in " << m_cacheRegister.GetName();
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
        SetCacheRegister(CodeGenValue(tree));
    }


    //template <typename T>
    //void Node<T>::CompileAsRoot(ExpressionTree& tree)
    //{
    //    CodeGenValue(tree);
    //}


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


    //*************************************************************************
    //
    // Template definitions for DirectValue
    //
    //*************************************************************************
    template <typename T>
    DirectValue<T>::DirectValue(ExpressionTree& tree)
        : Node(tree)
    {
    }


    template <typename T>
    bool DirectValue<T>::IsImmediate() const
    {
        return false;
    }


    template <typename T>
    bool DirectValue<T>::IsIndirect() const
    {
        return false;
    }


    //template <typename T>
    //bool DirectValue<T>::IsFieldPointer() const
    //{
    //    return false;
    //}
}
