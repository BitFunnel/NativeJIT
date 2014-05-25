#pragma once

#include "Allocator.h"
#include "BinaryNode.h"
#include "ConditionalNode.h"
#include "FieldPointerNode.h"
#include "ImmediateNode.h"
#include "IndirectNode.h"
#include "ParameterNode.h"
#include "Node.h"
#include "ReturnNode.h"


namespace Allocators
{
    class IAllocator;
}


namespace NativeJIT
{
    class ExpressionTree;

    class ExpressionNodeFactory
    {
    public:
        ExpressionNodeFactory(Allocators::IAllocator& allocator, ExpressionTree& tree);

        //
        // Leaf nodes
        //
        template <typename T> Node<T>& Immediate(T value);
        template <typename T> Node<T>& Parameter();


        //
        // Unary operators
        //
        template <typename T> Node<T>& Deref(Node<T*>& pointer);
        template <typename OBJECT, typename FIELD> Node<FIELD*>& FieldPointer(Node<OBJECT*>& object, FIELD OBJECT::*field);
        template <typename T> NodeBase& Return(Node<T>& value);


        //
        // Binary arithmetic operators
        //
        template <typename L, typename R> Node<L>& Add(Node<L>& left, Node<R>& right);
        template <typename L, typename R> Node<L>& Sub(Node<L>& left, Node<R>& right);
        template <typename L, typename R> Node<L>& Mul(Node<L>& left, Node<R>& right);

        template <typename T> Node<T*>& Add(Node<T*>& array, Node<unsigned __int64>& index);


        //
        // Relational operators
        //
        template <typename T> FlagExpressionNode<JccType::JG>& GreaterThan(Node<T>& left, Node<T>& right);


        //
        // Conditional operator
        //
        template <typename T, JccType JCC>
        Node<T>& Conditional(FlagExpressionNode<JCC>& condition, Node<T>& left, Node<T>& right);

    private:
        template <typename L, typename R> Node<L>& Binary(char const* operation, Node<L>& left, Node<R>& right);


        Allocators::IAllocator& m_allocator;
        ExpressionTree& m_tree;
    };


    //*************************************************************************
    //
    // Template definitions for ExpressionNodeFactory.
    //
    //*************************************************************************

    //
    // Leaf nodes
    //
    template <typename T>
    Node<T>& ExpressionNodeFactory::Immediate(T value)
    {
        return * new (m_allocator.Allocate(sizeof(ImmediateNode<T>))) ImmediateNode<T>(m_tree, value);
    }


    template <typename T>
    Node<T>& ExpressionNodeFactory::Parameter()
    {
        return * new (m_allocator.Allocate(sizeof(ParameterNode<T>))) ParameterNode<T>(m_tree);
    }


    //
    // Unary operators
    //
    template <typename T>
    Node<T>& ExpressionNodeFactory::Deref(Node<T*>& pointer)
    {
        return * new (m_allocator.Allocate(sizeof(IndirectNode<T>))) IndirectNode<T>(m_tree, pointer, 0);
    }


    template <typename OBJECT, typename FIELD>
    Node<FIELD*>& ExpressionNodeFactory::FieldPointer(Node<OBJECT*>& object, FIELD OBJECT::*field)
    {
        return * new (m_allocator.Allocate(sizeof(FieldPointerNode<OBJECT, FIELD>))) FieldPointerNode<OBJECT, FIELD>(m_tree, object, field);
    }


    template <typename T>
    NodeBase& ExpressionNodeFactory::Return(Node<T>& value)
    {
        return * new (m_allocator.Allocate(sizeof(ReturnNode<T>))) ReturnNode<T>(m_tree, value);
    }


    //
    // Binary arithmetic operators
    //
    template <typename L, typename R>
    Node<L>& ExpressionNodeFactory::Add(Node<L>& left, Node<R>& right)
    {
        return Binary("add", left, right);
    }


    template <typename L, typename R>
    Node<L>& ExpressionNodeFactory::Mul(Node<L>& left, Node<R>& right)
    {
        return Binary("mul", left, right);
    }


    template <typename T>
    Node<T*>& ExpressionNodeFactory::Add(Node<T*>& array, Node<unsigned __int64>& index)
    {
        auto & size = Immediate<unsigned __int64>(sizeof(T));
        auto & offset = Mul(index, size);
        return Binary("add", array, offset);
    }


    //
    // Relational operators
    //
    template <typename T>
    FlagExpressionNode<JccType::JG>& ExpressionNodeFactory::GreaterThan(Node<T>& left, Node<T>& right)
    {
        typedef RelationalOperatorNode<T, JccType::JG> NodeType;
        return * new (m_allocator.Allocate(sizeof(NodeType))) NodeType(m_tree, left, right);
    }


    //
    // Conditional operator
    //
    template <typename T, JccType JCC>
    Node<T>& ExpressionNodeFactory::Conditional(FlagExpressionNode<JCC>& condition,
                                                Node<T>& left,
                                                Node<T>& right)
    {
        typedef ConditionalNode<T, JCC> NodeType;
        return * new (m_allocator.Allocate(sizeof(NodeType))) NodeType(m_tree, condition, left, right);
    }



    //
    // Private methods.
    //
    template <typename L, typename R>
    Node<L>& ExpressionNodeFactory::Binary(char const* operation, Node<L>& left, Node<R>& right)
    {
        return * new (m_allocator.Allocate(sizeof(BinaryNode<L, R>))) 
                     BinaryNode<L, R>(m_tree, operation, left, right);
    }
}
