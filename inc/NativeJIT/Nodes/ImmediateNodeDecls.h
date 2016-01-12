#pragma once

#include "NativeJIT/Nodes/Node.h"
#include "NativeJIT/TypePredicates.h"


namespace NativeJIT
{
    template <typename T, ImmediateCategory IMMCATEGORY = ImmediateCategoryOf<T>::value>
    class ImmediateNode;


    //*************************************************************************
    //
    // Template specializations for ImmediateNode for InlineImmediate types.
    //
    //*************************************************************************
    template <typename T>
    class ImmediateNode<T, ImmediateCategory::InlineImmediate> : public Node<T>
    {
    public:
        ImmediateNode(ExpressionTree& tree, T value);

        //
        // Overrides of Node methods
        //
        virtual void Print(std::ostream& out) const override;
        virtual ExpressionTree::Storage<T> CodeGenValue(ExpressionTree& tree) override;

    private:
        // WARNING: This class is designed to be allocated by an arena allocator,
        // so its destructor will never be called. Therefore, it should hold no
        // resources other than memory from the arena allocator.
        ~ImmediateNode();

        T m_value;
    };


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
        ImmediateNode(ExpressionTree& tree, T value);

        //
        // Overrides of Node methods
        //
        virtual void Print(std::ostream& out) const override;
        virtual ExpressionTree::Storage<T> CodeGenValue(ExpressionTree& tree) override;


        //
        // Overrides of RIPRelativeImmediate methods
        //
        virtual void EmitStaticData(ExpressionTree& tree) override;

    private:
        // WARNING: This class is designed to be allocated by an arena allocator,
        // so its destructor will never be called. Therefore, it should hold no
        // resources other than memory from the arena allocator.
        ~ImmediateNode();

        T m_value;
        int32_t m_offset;
    };
}
