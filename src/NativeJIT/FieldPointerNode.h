#pragma once

#include "Node.h"


namespace NativeJIT
{
    template <typename OBJECT, typename FIELD>
    class FieldPointerNode : public Node<FIELD*>
    {
    public:
        FieldPointerNode(ExpressionTree& tree, Node<OBJECT*>& base, FIELD OBJECT::*field);

        //
        // Overrides of Node methods
        //

        virtual ExpressionTree::Storage<void*> CodeGenBase(ExpressionTree& tree) override;
        virtual ExpressionTree::Storage<FIELD*> CodeGenValue(ExpressionTree& tree) override;
        virtual __int32 GetOffset() const override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    private:
        static __int32 Offset(FIELD OBJECT::*field)
        {
            return reinterpret_cast<__int32>(&((static_cast<OBJECT*>(nullptr))->*field));
        }

        Node<OBJECT*>& m_base;
        __int32 m_offset;
    };


    //*************************************************************************
    //
    // Template definitions for FieldPointerNode
    //
    //*************************************************************************
    template <typename OBJECT, typename FIELD>
    FieldPointerNode<OBJECT, FIELD>::FieldPointerNode(ExpressionTree& tree,
                                                      Node<OBJECT*>& base,
                                                      FIELD OBJECT::*field)
        : Node(tree),
          m_base(base),
          m_offset(Offset(field))
    {
        m_base.IncrementParentCount();
    }


    template <typename OBJECT, typename FIELD>
    typename ExpressionTree::Storage<void*> FieldPointerNode<OBJECT, FIELD>::CodeGenBase(ExpressionTree& tree)
    {
        return m_base.CodeGenBase(tree);
    }


    template <typename OBJECT, typename FIELD>
    typename ExpressionTree::Storage<FIELD*> FieldPointerNode<OBJECT, FIELD>::CodeGenValue(ExpressionTree& tree)
    {
        auto base = CodeGenBase(tree);
        base.ConvertToDirect(true);

        if (GetOffset() != 0)
        {
            tree.GetCodeGenerator().EmitImmediate<OpCode::Add>(base.GetDirectRegister(), GetOffset());
        }

        // With the added offset, the type changes from void* to FIELD*.
        return ExpressionTree::Storage<FIELD*>(base);
    }


    template <typename OBJECT, typename FIELD>
    __int32 FieldPointerNode<OBJECT, FIELD>::GetOffset() const
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
