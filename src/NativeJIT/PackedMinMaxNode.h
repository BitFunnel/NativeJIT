#pragma once


#include "NativeJIT/Packed.h"
#include "NativeJIT/X64CodeGenerator.h"     // OpCode type.
#include "Node.h"


namespace NativeJIT
{
    // TODO: Implement distinct Min and Max operations. Currently class only does Max.
    // TODO: This class and Packed should probably move from NativeJIT to BitFunnel.Library.

    template <typename PACKED>
    class PackedMinMaxNode : public Node<PACKED>
    {
    public:
        PackedMinMaxNode(ExpressionTree& tree,
                         Node<PACKED>& left,
                         Node<PACKED>& right);

        virtual ExpressionTree::Storage<PACKED> CodeGenValue(ExpressionTree& tree) override;

        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    private:
        Node<PACKED>& m_left;
        Node<PACKED>& m_right;
    };


    //*************************************************************************
    //
    // Template definitions for PackedMinMaxNode
    //
    //*************************************************************************
    template <typename PACKED>
    PackedMinMaxNode<PACKED>::PackedMinMaxNode(ExpressionTree& tree,
                                               Node<PACKED>& left,
                                               Node<PACKED>& right)
        : Node(tree),
          m_left(left),
          m_right(right)
    {
        static_assert(std::is_pod<PACKED>::value, "PACKED must be a POD type.");
        left.IncrementParentCount();
        right.IncrementParentCount();
    }


    template <typename PACKED>
    ExpressionTree::Storage<PACKED> PackedMinMaxNode<PACKED>::CodeGenValue(ExpressionTree& tree)
    {
        auto & code = tree.GetCodeGenerator();

        auto registerRCX = tree.Direct<unsigned __int64>(rcx);
        code.Emit<OpCode::Mov>(rcx, PACKED::c_fieldSizes);

        auto sLeft = m_left.CodeGen(tree);
        sLeft.ConvertToValue(tree, true);
        auto l = sLeft.GetDirectRegister();
        // TODO: Use correct shift
        code.Emit<OpCode::Sal>(l, static_cast<unsigned __int8>(64 - 12));

        auto sRight = m_right.CodeGen(tree);
        sRight.ConvertToValue(tree, true);
        auto r = sRight.GetDirectRegister();
        // TODO: Use correct shift
        code.Emit<OpCode::Sal>(r, static_cast<unsigned __int8>(64 - 12));

        Label topOfLoop = code.AllocateLabel();
        Label keepLeftDigit = code.AllocateLabel();
        Label bottomOfLoop = code.AllocateLabel();

        code.PlaceLabel(topOfLoop);

        code.Emit<OpCode::Cmp>(l, r);
        code.Emit<JccType::JAE>(keepLeftDigit);

        code.Emit<OpCode::Shld>(l, r);
        code.Jmp(bottomOfLoop);

        code.PlaceLabel(keepLeftDigit);
        code.Emit<OpCode::Rol>(l);

        code.PlaceLabel(bottomOfLoop);
        code.Emit<OpCode::Sal>(r);

        code.Emit<OpCode::Shr>(rcx, static_cast<unsigned __int8>(8));
        code.Emit<JccType::JNZ>(topOfLoop);

        return sLeft;
    }


    template <typename PACKED>
    unsigned PackedMinMaxNode<PACKED>::LabelSubtree(bool isLeftChild)
    {
        // TODO: imlement.
        return 0;
    }


    template <typename PACKED>
    void PackedMinMaxNode<PACKED>::Print() const
    {
        // TODO: imlement.
        std::cout << "PackedMinMaxNode" << std::endl;
    }
}
