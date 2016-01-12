#pragma once

#include "NativeJIT/CodeGen/X64CodeGenerator.h"     // OpCode type.
#include "NativeJIT/Nodes/Node.h"


namespace NativeJIT
{
    // Implements a node for the SHLD instruction.
    template <typename T>
    class ShldNode : public Node<T>
    {
    public:
        ShldNode(ExpressionTree& tree, Node<T>& shiftee, Node<T>& filler, uint8_t bitCount);

        virtual Storage<T> CodeGenValue(ExpressionTree& tree) override;

        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print(std::ostream& out) const override;

    private:
        // WARNING: This class is designed to be allocated by an arena allocator,
        // so its destructor will never be called. Therefore, it should hold no
        // resources other than memory from the arena allocator.
        ~ShldNode();

        Node<T>& m_shiftee;
        Node<T>& m_filler;
        const uint8_t m_bitCount;
    };


    //*************************************************************************
    //
    // Template definitions for BinaryNode
    //
    //*************************************************************************
    template <typename T>
    ShldNode<T>::ShldNode(ExpressionTree& tree,
                          Node<T>& shiftee,
                          Node<T>& filler,
                          uint8_t bitCount)
        : Node<T>(tree),
          m_shiftee(shiftee),
          m_filler(filler),
          m_bitCount(bitCount)
    {
        m_shiftee.IncrementParentCount();
        m_filler.IncrementParentCount();
    }


    template <typename T>
    Storage<T> ShldNode<T>::CodeGenValue(ExpressionTree& tree)
    {
        auto & code = tree.GetCodeGenerator();

        Storage<T> shiftee;
        Storage<T> filler;

        this->CodeGenInPreferredOrder(tree,
                                      m_shiftee, shiftee,
                                      m_filler, filler);

        // Convert both arguments to direct registers. The filler value will
        // not be touched.
        {
            auto shifteeReg = shiftee.ConvertToDirect(true);
            ReferenceCounter shifteePin = shiftee.GetPin();
            auto fillerReg = filler.ConvertToDirect(false);

            code.EmitImmediate<OpCode::Shld>(shifteeReg, fillerReg, m_bitCount);
        }

        return shiftee;
    }


    template <typename T>
    unsigned ShldNode<T>::LabelSubtree(bool /* isLeftChild */)
    {
        const unsigned left = m_shiftee.LabelSubtree(true);
        const unsigned right = m_filler.LabelSubtree(false);

        this->SetRegisterCount(this->ComputeRegisterCount(left, right));

        // WARNING: GetRegisterCount() may return a different value than passed to SetRegisterCount().
        return this->GetRegisterCount();
    }


    template <typename T>
    void ShldNode<T>::Print(std::ostream& out) const
    {
        this->PrintCoreProperties(out, "Shld");

        out << ", shiftee = " << m_shiftee.GetId()
            << ", filler = " << m_filler.GetId()
            << ", bitCount = " << m_bitCount;
    }
}
