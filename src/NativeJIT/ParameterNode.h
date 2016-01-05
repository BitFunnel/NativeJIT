#pragma once

#include "ExpressionTree.h"
#include "Node.h"
#include "Temporary/Assert.h"


namespace NativeJIT
{
    template <typename T>
    class ParameterNode : public Node<T>
    {
    public:
        ParameterNode(ExpressionTree& tree, unsigned position);

        unsigned GetPosition() const;

        //
        // Overrides of NodeBase methods.
        //
        virtual void ReleaseReferencesToChildren() override;

        //
        // Overrides of Node methods.
        //
        virtual ExpressionTree::Storage<T> CodeGenValue(ExpressionTree& tree) override;

        virtual void Print() const override;

    private:
        // WARNING: This class is designed to be allocated by an arena allocator,
        // so its destructor will never be called. Therefore, it should hold no
        // resources other than memory from the arena allocator.
        ~ParameterNode();

        unsigned m_position;
    };


    //*************************************************************************
    //
    // Template definitions for ParameterNode
    //
    //*************************************************************************

    template <unsigned SIZE>
    void GetParameterRegister(unsigned id, Register<SIZE, false>& r)
    {
        // For now we only support the four parameters that are passed in registers.
        // No support for memory parameters.
        Assert(id < 4, "Exceeded maximum number of register parameters.");

        // Integer parameters are passed in RCX, RDX, R8, and R9.
        // TODO: Use constants to encode registers.
        const uint8_t idMap[] = {1, 2, 8, 9};

        r = Register<SIZE, false>(idMap[id]);
    }


    template <unsigned SIZE>
    void GetParameterRegister(unsigned id, Register<SIZE, true>& r)
    {
        // For now we only support the four parameters that are passed in registers.
        // No support for memory parameters.
        Assert(id < 4, "Exceeded maximum number of register parameters.");

        // Floating point parameters are passed in XMM0-XMM3.
        r = Register<SIZE, true>(id);
    }


    template <typename T>
    ParameterNode<T>::ParameterNode(ExpressionTree& tree, unsigned position)
        : Node(tree),
          m_position(position)
    {
        // Parameter nodes are always considered to be referenced (as a part of
        // the function being compiled) even when they are not referenced
        // explicitly.
        MarkReferenced();
        tree.AddParameter(*this, m_position);
    }


    template <typename T>
    unsigned ParameterNode<T>::GetPosition() const
    {
        return m_position;
    }


    template <typename T>
    void ParameterNode<T>::ReleaseReferencesToChildren()
    {
        // No children to release.
    }


    template <typename T>
    typename ExpressionTree::Storage<T> ParameterNode<T>::CodeGenValue(ExpressionTree& tree)
    {
        Storage<T>::DirectRegister reg;
        GetParameterRegister(m_position, reg);

        return tree.Direct<T>(reg);
    }


    template <typename T>
    void ParameterNode<T>::Print() const
    {
        PrintCoreProperties("ParameterNode");

        std::cout << ", position = " << m_position;
    }
}
