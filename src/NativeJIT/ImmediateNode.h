#pragma once

#include <type_traits>

#include "Node.h"


namespace NativeJIT
{
    template <typename T, ImmediateCategory IMMCATEGORY = ImmediateCategoryOf<T>::value>
    class ImmediateNode : public Node<T>
    {
    };


    //*************************************************************************
    //
    // Template specializations for ImmediateNode for CoreImmediate types.
    //
    //*************************************************************************
    template <typename T>
    class ImmediateNode<T, ImmediateCategory::CoreImmediate> : public Node<T>
    {
    public:
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
            PrintCoreProperties("ImmediateNode");

            std::cout << ", value = " << m_value;
        }


        virtual ExpressionTree::Storage<T> CodeGenValue(ExpressionTree& tree) override
        {
            return tree.Immediate(m_value);
        }

    private:
        T m_value;
    };
}


// Note: including CastNode.h in the section where template declarations are
// specified since only the implementation uses CastNode. Doing otherwise would
// create circular dependency for some headers.
// This is analog to including CastNode.h in ImmediateNode.cpp if ImmediateNode
// was a regular class rather than a template.
#include "CastNode.h"

namespace NativeJIT
{
    //*************************************************************************
    //
    // Template specializations for ImmediateNode for RIPRelativeImmediate types.
    //
    //*************************************************************************

    class RIPRelativeImmediate
    {
    public:
        virtual void EmitStaticData(ExpressionTree& tree) = 0;
    };


    template <typename T>
    class ImmediateNode<T, ImmediateCategory::RIPRelativeImmediate>
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
            PrintCoreProperties("ImmediateNode (RIP-indirect)");

            std::cout << ", value = " << m_value;
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
