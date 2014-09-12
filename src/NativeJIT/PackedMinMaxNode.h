#pragma once


#include "NativeJIT/Packed.h"
#include "NativeJIT/X64CodeGenerator.h"     // OpCode type.
#include "Node.h"


namespace NativeJIT
{
    // TODO: Implement distinct Min and Max operations. Currently class only does Max.
    // TODO: This class and Packed should probably move from NativeJIT to BitFunnel.Library.

    template <typename PACKED, bool ISMAX>
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
    template <typename PACKED, bool ISMAX>
    PackedMinMaxNode<PACKED, ISMAX>::PackedMinMaxNode(ExpressionTree& tree,
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


    template <typename PACKED, bool ISMAX>
    ExpressionTree::Storage<PACKED>
    PackedMinMaxNode<PACKED, ISMAX>::CodeGenValue(ExpressionTree& tree)
    {
        auto & code = tree.GetCodeGenerator();

        // TODO: BUGBUG: If the following two lines of code that allocate
        // RCX and load it are moved to just before topOfLoop, the code
        // generated becomes incorrect because existing register values
        // are bumped. Need to investigate why this happens. There may be a
        // bug in the register allocator. The reason is that l and r are
        // initialized by calls to GetDirectRegister() before RCX is dumped.

        // The bytes is RCX will how the number of bits in each field. The lowest
        // order byte has the width of the leftmost field. RCX is used because the
        // shift operations use CL as a parameter. RCX is shifted right by 8 bits
        // each iteration so that CL has the width of the current field.
        auto registerRCX = tree.Direct<unsigned __int64>(rcx);
        code.Emit<OpCode::Mov>(rcx, PACKED::c_fieldSizes);

        // Convert the left parameter into a value. We will be building the
        // result in this register.
        auto sLeft = m_left.CodeGen(tree);
        sLeft.ConvertToValue(tree, true);
        auto l = sLeft.GetDirectRegister();

        // Convert the right parameter into a value. We will destroy this
        // value as we construct the result in the left register.
        auto sRight = m_right.CodeGen(tree);
        sRight.ConvertToValue(tree, true);
        auto r = sRight.GetDirectRegister();

        // Shift bits of each argument all the way to the left.
        code.Emit<OpCode::Sal>(l, static_cast<unsigned __int8>(64 - PACKED::c_bitCount));
        code.Emit<OpCode::Sal>(r, static_cast<unsigned __int8>(64 - PACKED::c_bitCount));

        Label topOfLoop = code.AllocateLabel();
        Label keepLeftDigit = code.AllocateLabel();
        Label bottomOfLoop = code.AllocateLabel();

        // Loop over each field.
        code.PlaceLabel(topOfLoop);

        // Comparing l to r effectively compares the leftmost fields of the two values.
        code.Emit<OpCode::Cmp>(l, r);
        if (ISMAX)
        {
            code.Emit<JccType::JAE>(keepLeftDigit);
        }
        else
        {
            code.Emit<JccType::JB>(keepLeftDigit);
        }

        // Case: Want the keep the digit from the right parameter.
        //       Shift the high order bits from the right parameter into the low
        //       order bits of the left parameter.
        code.Emit<OpCode::Shld>(l, r);
        code.Jmp(bottomOfLoop);

        // Case: Want to keep the digit from left parameter.
        //       Just rotate it around to the low order bits.
        code.PlaceLabel(keepLeftDigit);
        code.Emit<OpCode::Rol>(l);

        // In either case, shift off the high order bits or the right parameter.
        code.PlaceLabel(bottomOfLoop);
        code.Emit<OpCode::Sal>(r);

        // If there are more digits, jump back to the top of the loop.
        code.Emit<OpCode::Shr>(rcx, static_cast<unsigned __int8>(8));
        code.Emit<JccType::JNZ>(topOfLoop);

        return sLeft;
    }


    template <typename PACKED, bool ISMAX>
    unsigned PackedMinMaxNode<PACKED, ISMAX>::LabelSubtree(bool isLeftChild)
    {
        // TODO: imlement.
        return 0;
    }


    template <typename PACKED, bool ISMAX>
    void PackedMinMaxNode<PACKED, ISMAX>::Print() const
    {
        // TODO: imlement.
        std::cout << "PackedMinMaxNode" << std::endl;
    }
}
