#pragma once

#include <type_traits>

#include "CastNode.h"
#include "Node.h"


namespace NativeJIT
{
    template <typename T>
    struct RequiresRIPRelativeImmediate
    {
        // Floating point instructions don't allow for immediate values,
        // so RIP-relative indirect must be used. Also, many instructions taking
        // an integer immediate (such as Add, Cmp etc) do not have a form which
        // takes a 64-bit immediate and need a RIP-relative indirect.
        static const bool c_value = RegisterStorage<T>::c_isFloat
                                    || RegisterStorage<T>::c_size == 8;
    };


    template <typename T, typename ENABLE = void>
    class ImmediateNode : public Node<T>
    {
    public:
        static_assert(IsValidImmediate<T>::value, "Invalid type for an immediate node");

        ImmediateNode(ExpressionTree& tree, T value)
            : Node(tree),
              m_value(value)
        {
        }


        //
        // Overrides of Node methods
        //
        virtual void Print() const override
        {
            std::cout << "ImmediateNode id=" << GetId();
            std::cout << ", parents = " << GetParentCount();
            std::cout << ", value = " << m_value;
            std::cout << ", ";
            PrintRegisterAndCacheInfo();
        }


        virtual ExpressionTree::Storage<T> CodeGenValue(ExpressionTree& tree) override
        {
            return tree.Immediate(m_value);
        }

    private:
        T m_value;
    };


    //*************************************************************************
    //
    // Template specializations for ImmediateNode<double>
    //
    //*************************************************************************
    class RIPRelativeImmediate
    {
    public:
        virtual void EmitStaticData(ExpressionTree& tree) = 0;
    };


    template <typename T>
    class ImmediateNode<T, typename std::enable_if<RequiresRIPRelativeImmediate<T>::c_value>::type>
        : public Node<T>,
          public RIPRelativeImmediate
    {
    public:
        ImmediateNode(ExpressionTree& tree, T value)
            : Node<T>(tree),
              m_value(value)
        {
            tree.AddRIPRelative(*this);

            // m_offset will be initialized with the correct value during pass0
            // of compilation in the call to EmitStaticData().
            m_offset = 0;
        }


        //
        // Overrides of Node methods
        //
        virtual void Print() const override
        {
            std::cout << "ImmediateNode id=" << GetId();
            std::cout << ", parents = " << GetParentCount();
            std::cout << ", value = " << m_value;
            std::cout << "  RIP-indirect, ";
            std::cout << ", ";
            PrintRegisterAndCacheInfo();
        }


        virtual ExpressionTree::Storage<T> CodeGenValue(ExpressionTree& tree) override
        {
            return tree.RIPRelative<T>(m_offset);
        }


        //
        // Overrides of RIPRelativeImmediate methods
        //
        virtual void EmitStaticData(ExpressionTree& tree) override
        {
            auto & code = tree.GetCodeGenerator();
            code.AdvanceToAlignment<T>();
            m_offset = code.CurrentPosition();

            // Emit the value using a canonical type since the EmitValueBytes
            // method intentionally has a limited number of input types. Basic
            // types will be unchanged, but f. ex. function pointers will be
            // emitted as unsigned __int64.
            code.EmitValueBytes(Casting::ForcedCast<typename CanonicalRegisterStorageType<T>::Type>(m_value));
        }

    private:
        T m_value;
        __int32 m_offset;
    };
}
