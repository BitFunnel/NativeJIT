#pragma once

#include "Node.h"


namespace NativeJIT
{
    template <typename FIELD>
    class FieldPointerBase : public Node<FIELD*>
    {
    public:
        typedef typename Node<FIELD*>::RegisterType BaseRegisterType;

        FieldPointerBase(ExpressionTree& tree);

        virtual bool IsFieldPointer() const override;

        virtual Storage<FIELD*> CodeGenBase(ExpressionTree& tree) = 0;
    };


    template <typename OBJECT, typename FIELD>
    class FieldPointerNode : public FieldPointerBase<FIELD>
    {
    public:
        FieldPointerNode(ExpressionTree& treem, Node<OBJECT*>& base, FIELD OBJECT::*field);

        //
        // Overrides of FieldPointerBase methods
        //

        virtual Storage<FIELD*> CodeGenBase(ExpressionTree& tree) override;

        //
        // Overrides of Node methods
        //

        virtual Storage<FIELD*> CodeGenValue(ExpressionTree& tree) override;
        virtual unsigned __int64 GetOffset() const override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    private:
        static unsigned __int64 Offset(FIELD OBJECT::*field)
        {
            return reinterpret_cast<unsigned __int64>(&((static_cast<OBJECT*>(nullptr))->*field));
        }

        Node<OBJECT*>& m_base;
        unsigned __int64 m_offset;
    };


    //*************************************************************************
    //
    // Template definitions for FieldPointerBase
    //
    //*************************************************************************
    template <typename FIELD>
    FieldPointerBase<FIELD>::FieldPointerBase(ExpressionTree& tree)
        : Node(tree)
    {
    }


    template <typename FIELD>
    bool FieldPointerBase<FIELD>::IsFieldPointer() const
    {
        return true;
    }



    //*************************************************************************
    //
    // Template definitions for FieldPointerNode
    //
    //*************************************************************************
    template <typename OBJECT, typename FIELD>
    FieldPointerNode<OBJECT, FIELD>::FieldPointerNode(ExpressionTree& tree,
                                              Node<OBJECT*>& base,
                                              FIELD OBJECT::*field)
        : FieldPointerBase(tree),
          m_base(base),
          m_offset(Offset(field))
    {
        m_base.IncrementParentCount();
    }


    template <typename OBJECT, typename FIELD>
    typename Storage<FIELD*> FieldPointerNode<OBJECT, FIELD>::CodeGenBase(ExpressionTree& tree)
    {
        if (m_base.IsFieldPointer())
        {
            auto & base = reinterpret_cast<FieldPointerBase<FIELD>&>(m_base);
            return base.CodeGenBase(tree);
        }
        else
        {
            auto base = m_base.CodeGen(tree);
            return Storage<FIELD*>(tree, base);
        }
    }


    template <typename OBJECT, typename FIELD>
    typename Storage<FIELD*> FieldPointerNode<OBJECT, FIELD>::CodeGenValue(ExpressionTree& tree)
    {
        auto base = CodeGenBase(tree);
        unsigned __int64 offset = m_base.GetOffset();

        base.ConvertToValue(tree, true);
        tree.GetCodeGenerator().Op("add", base.GetDirectRegister(), m_offset + offset);

        return base;
    }


    template <typename OBJECT, typename FIELD>
    unsigned __int64 FieldPointerNode<OBJECT, FIELD>::GetOffset() const
    {
        return m_offset + m_base.GetOffset();
    }


    template <typename OBJECT, typename FIELD>
    unsigned FieldPointerNode<OBJECT, FIELD>::LabelSubtree(bool /*isLeftChild*/)
    {
        // TODO: Should isLeftChild be passed down?
        SetRegisterCount(m_base.LabelSubtree(true));
        return GetRegisterCount();
    }


    template <typename OBJECT, typename FIELD>
    void FieldPointerNode<OBJECT, FIELD>::Print() const
    {
        std::cout << "FieldPointerNode id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", offset = " << m_offset;
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }
}
