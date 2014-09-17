#pragma once

#include "Node.h"


namespace NativeJIT
{
    //template <typename PACKED>
    //class ModelApplyNode : public Node<float>
    //{
    //public:
    //    ModelApplyNode(ExpressionTree& tree, Node<Model<PACKED>*>& model, Node<Packed>& packed);

    //    virtual ExpressionTree::Storage<float> CodeGenValue(ExpressionTree& tree) override;

    //    virtual unsigned LabelSubtree(bool isLeftChild) override;
    //    virtual void Print() const override;

    //private:
    //    static unsigned ComputeRegisterCount(unsigned leftCount, unsigned rightCount);

    //    Node<Model<PACKED>*>& m_model;
    //    Node<PACKED>& m_packed;
    //};


    ////*************************************************************************
    ////
    //// Template definitions for ModelApplyNode<PACKED>
    ////
    ////*************************************************************************
    //template <typename PACKED>
    //ModelApplyNode<PACKED>::ModelApplyNode(ExpressionTree& tree,
    //                                       Node<Model<PACKED>*>& model,
    //                                       Node<Packed>& packed)
    //    : Node(tree),
    //      m_model(model),
    //      m_packed(packed)
    //{
    //    model.IncrementParentCount();
    //    packed.IncrementParentCount();
    //}


    //template <typename PACKED>
    //virtual ExpressionTree::Storage<float>
    //ModelApplyNode<PACKED>::CodeGenValue(ExpressionTree& tree) override
    //{
    //    auto base = 
    //}


    //template <typename PACKED>
    //virtual unsigned ModelApplyNode<PACKED>::LabelSubtree(bool isLeftChild) override;


    //template <typename PACKED>
    //virtual void ModelApplyNode<PACKED>::Print() const override;


    template <typename FROM, typename TO>
    class CastNode : public Node<TO>
    {
    };


    template <typename PACKED>
    class CastNode<PACKED, unsigned __int64> : public Node<unsigned __int64>
    {
    public:
        CastNode(ExpressionTree& tree, Node<PACKED>& child);

        virtual ExpressionTree::Storage<unsigned __int64> CodeGenValue(ExpressionTree& tree) override;

        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    private:
        Node<PACKED>& m_child;
    };


    //*************************************************************************
    //
    // Template definitions for CastNode<PACKED, unsigned __int64>
    //
    //*************************************************************************
    template <typename PACKED>
    CastNode<PACKED, unsigned __int64>::CastNode(ExpressionTree& tree,
                                                 Node<PACKED>& child)
        : Node(tree),
          m_child(packed)
    {
        child.IncrementParentCount();
    }


    template <typename PACKED>
    typename ExpressionTree::Storage<unsigned __int64>
    CastNode<PACKED, unsigned __int64>::CodeGenValue(ExpressionTree& tree)
    {
        return m_child.CodeGenValue();
    }


    template <typename PACKED>
    unsigned CastNode<PACKED, unsigned __int64>::LabelSubtree(bool /*isLeftChild*/)
    {
        return m_child..LabelSubtree(true);
    }


    template <typename PACKED>
    void CastNode<PACKED, unsigned __int64>::Print() const
    {
        std::cout << "Cast(Packed -> unsigned __int64) id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", child = " << m_child.GetId();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }
}
