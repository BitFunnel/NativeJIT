#pragma once

#include "Node.h"


namespace NativeJIT
{
    template <typename T>
    class IndirectNode : public Node<T>
    {
    public:
        typedef typename Node<T*>::RegisterType BaseRegisterType;

        IndirectNode(ExpressionTree& tree, Node<T*>& base, unsigned __int64 offset);

        Storage<T*> CodeGenBase2(ExpressionTree& tree);
        bool IsBaseRegisterCached() const;

        //
        // Overrides of Node methods.
        //

        virtual Storage<T> CodeGenValue2(ExpressionTree& tree) override;
        virtual unsigned __int64 GetOffset() const override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    private:
        Node<T*>& m_base;
        unsigned __int64 m_offset;
    };


    //*************************************************************************
    //
    // Template definitions for IndirectBase
    //
    //*************************************************************************
    template <typename T>
    IndirectNode<T>::IndirectNode(ExpressionTree& tree,
                                  Node<T*>& base,
                                  unsigned __int64 offset)
        : Node(tree),
          m_base(base),
          m_offset(offset)
    {
        m_base.IncrementParentCount();
    }


    template <typename T>
    typename Storage<T*> IndirectNode<T>::CodeGenBase2(ExpressionTree& tree)
    {
        if (m_base.IsFieldPointer())
        {
            auto & base = reinterpret_cast<FieldPointerBase<T>&>(m_base);
            return base.CodeGenBase2(tree);
        }
        else
        {
            return m_base.CodeGenValue2(tree);
        }
    }


    template <typename T>
    bool IndirectNode<T>::IsBaseRegisterCached() const
    {
        return m_base.IsCached();
    }


    template <typename T>
    typename Storage<T> IndirectNode<T>::CodeGenValue2(ExpressionTree& tree)
    {
        if (IsCached2())
        {
            auto result = GetCache2();
            ReleaseCache2();
            return result;
        }
        else
        {
            auto& base = CodeGenBase2(tree);
            unsigned __int64 offset = m_base.GetOffset();

            Storage<T> value(tree, base, m_offset + offset);
            //value.ConvertToValue(tree, true);

            return value;
        }
    }


    template <typename T>
    unsigned __int64 IndirectNode<T>::GetOffset() const
    {
        return m_offset;
    }


    template <typename T>
    unsigned IndirectNode<T>::LabelSubtree(bool isLeftChild)
    {
        // TODO: Should isLeftChild be passed down?
        SetRegisterCount(m_base.LabelSubtree(true));
        return GetRegisterCount();
    }


    template <typename T>
    void IndirectNode<T>::Print() const
    {
        std::cout << "IndirectNode id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", offset = " << m_offset;
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }
}
