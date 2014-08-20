#pragma once

#include "BinaryNode.h"
#include "CallNode.h"
#include "ConditionalNode.h"
#include "ExpressionTree.h"             // Base class.
#include "FieldPointerNode.h"
#include "ImmediateNode.h"
#include "IndirectNode.h"
#include "Node.h"
#include "ParameterNode.h"
#include "ReturnNode.h"
//#include "Temporary/Allocator.h"
//#include "Temporary/NonCopyable.h"


namespace NativeJIT
{
    class ExpressionTree;
    class FunctionBuffer;

    class ExpressionNodeFactory : public ExpressionTree
    {
    public:
        ExpressionNodeFactory(Allocators::IAllocator& allocator, FunctionBuffer& code);

    //    //
    //    // Leaf nodes
    //    //
        template <typename T> Node<T>& Immediate(T value);
        template <typename T> ParameterNode<T>& Parameter();


    //    //
    //    // Unary operators
    //    //
        template <typename T> Node<T>& Deref(Node<T*>& pointer);
        template <typename OBJECT, typename FIELD> Node<FIELD*>& FieldPointer(Node<OBJECT*>& object, FIELD OBJECT::*field);
        template <typename T> NodeBase& Return(Node<T>& value);


        //
        // Binary arithmetic operators
        //
        template <typename L, typename R> Node<L>& Add(Node<L>& left, Node<R>& right);
    //    template <typename L, typename R> Node<L>& Sub(Node<L>& left, Node<R>& right);
        template <typename L, typename R> Node<L>& Mul(Node<L>& left, Node<R>& right);

        template <typename T, typename INDEX> Node<T*>& Add(Node<T*>& array, Node<INDEX>& index);


        //
        // Relational operators
        //
        template <typename T> FlagExpressionNode<JccType::JG>& GreaterThan(Node<T>& left, Node<T>& right);


        //
        // Conditional operator
        //
        template <typename T, JccType JCC>
        Node<T>& Conditional(FlagExpressionNode<JCC>& condition, Node<T>& left, Node<T>& right);


        //
        // Call node
        //
        template <typename R, typename P1, typename P2>
        Node<R>& Call(Node<R (*)(P1,P2)>& function, Node<P1>& param1, Node<P2>& param2);

    private:
        template <OpCode OP, typename L, typename R> Node<L>& Binary(Node<L>& left, Node<R>& right);
    };


    ////*************************************************************************
    ////
    //// Template definitions for ExpressionNodeFactory.
    ////
    ////*************************************************************************

    //
    // Leaf nodes
    //
    template <typename T>
    Node<T>& ExpressionNodeFactory::Immediate(T value)
    {
        return * new (m_allocator.Allocate(sizeof(ImmediateNode<T>))) ImmediateNode<T>(*this, value);
    }


    template <typename T>
    ParameterNode<T>& ExpressionNodeFactory::Parameter()
    {
        return * new (m_allocator.Allocate(sizeof(ParameterNode<T>))) ParameterNode<T>(*this);
    }


    //
    // Unary operators
    //
    template <typename T>
    Node<T>& ExpressionNodeFactory::Deref(Node<T*>& pointer)
    {
        return * new (m_allocator.Allocate(sizeof(IndirectNode<T>))) IndirectNode<T>(*this, pointer, 0);
    }


    template <typename OBJECT, typename FIELD>
    Node<FIELD*>& ExpressionNodeFactory::FieldPointer(Node<OBJECT*>& object, FIELD OBJECT::*field)
    {
        return * new (m_allocator.Allocate(sizeof(FieldPointerNode<OBJECT, FIELD>)))
                     FieldPointerNode<OBJECT, FIELD>(*this, object, field);
    }


    template <typename T>
    NodeBase& ExpressionNodeFactory::Return(Node<T>& value)
    {
        return * new (m_allocator.Allocate(sizeof(ReturnNode<T>)))
                     ReturnNode<T>(*this, value);
    }


    //
    // Binary arithmetic operators
    //
    template <typename L, typename R>
    Node<L>& ExpressionNodeFactory::Add(Node<L>& left, Node<R>& right)
    {
        return Binary<OpCode::Add>(left, right);
    }


    template <typename L, typename R>
    Node<L>& ExpressionNodeFactory::Mul(Node<L>& left, Node<R>& right)
    {
        return Binary<OpCode::IMul>(left, right);
    }


    template <typename T, typename INDEX>
    Node<T*>& ExpressionNodeFactory::Add(Node<T*>& array, Node<INDEX>& index)
    {
        auto & size = Immediate<INDEX>(sizeof(T));
        auto & offset = Mul(index, size);
        return Binary<OpCode::Add>(array, offset);
    }


    //
    // Relational operators
    //
    template <typename T>
    FlagExpressionNode<JccType::JG>&
    ExpressionNodeFactory::GreaterThan(Node<T>& left, Node<T>& right)
    {
        typedef RelationalOperatorNode<T, JccType::JG> NodeType;
        return * new (m_allocator.Allocate(sizeof(NodeType)))
                     NodeType(*this, left, right);
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
        return * new (m_allocator.Allocate(sizeof(NodeType)))
                     NodeType(*this, condition, left, right);
    }


    //
    // Call external function
    //
    template <typename R, typename P1, typename P2>
    Node<R>& ExpressionNodeFactory::Call(Node<R (*)(P1,P2)>& function,
                                         Node<P1>& param1,
                                         Node<P2>& param2)
    {
        typedef CallNode<R, P1, P2> NodeType;
        return * new (m_allocator.Allocate(sizeof(NodeType)))
                     NodeType(*this, function, param1, param2);
    }


    //
    // Private methods.
    //
    template <OpCode OP, typename L, typename R>
    Node<L>& ExpressionNodeFactory::Binary(Node<L>& left, Node<R>& right)
    {
        return * new (m_allocator.Allocate(sizeof(BinaryNode<OP, L, R>))) 
                     BinaryNode<OP, L, R>(*this, left, right);
    }
}
