#pragma once

#include "BinaryNode.h"
#include "CallNode.h"
#include "CastNode.h"
#include "ConditionalNode.h"
#include "ExpressionTree.h"             // Base class.
#include "FieldPointerNode.h"
#include "ImmediateNode.h"
#include "IndirectNode.h"
#include "NativeJIT/Model.h"
#include "Node.h"
#include "PackedMinMaxNode.h"
#include "ParameterNode.h"
#include "ReturnNode.h"
#include "Temporary/Allocator.h"


namespace NativeJIT
{
    class ExpressionTree;
    class FunctionBuffer;

    class ExpressionNodeFactory : public ExpressionTree
    {
    public:
        ExpressionNodeFactory(Allocators::IAllocator& allocator, FunctionBuffer& code);

        //
        // Leaf nodes
        //
        template <typename T> Node<T>& Immediate(T value);
        template <typename T> ParameterNode<T>& Parameter();


        //
        // Unary operators
        //
        template <typename TO, typename FROM> Node<TO>& Cast(Node<FROM>& value);
        template <typename T> Node<T>& Deref(Node<T*>& pointer);
        template <typename OBJECT, typename FIELD> Node<FIELD*>& FieldPointer(Node<OBJECT*>& object, FIELD OBJECT::*field);
        template <typename T> NodeBase& Return(Node<T>& value);


        //
        // Binary arithmetic operators
        //
        template <typename L, typename R> Node<L>& Add(Node<L>& left, Node<R>& right);
        template <typename L, typename R> Node<L>& Sub(Node<L>& left, Node<R>& right);
        template <typename L, typename R> Node<L>& Mul(Node<L>& left, Node<R>& right);

        // TODO: Implement following version of Add for arrays.
        //template <typename T, size_t SIZE, typename INDEX>
        //Node<T*> Add(Node<*(T[SIZE])>& array, Node<INDEX>& index);

        template <typename T, typename INDEX> Node<T*>& Add(Node<T*>& array, Node<INDEX>& index);


        //
        // Model related.
        //
        template <typename PACKED> Node<float>& ApplyModel(Node<Model<PACKED>*>& model, Node<PACKED>& packed);


        //
        // Relational operators
        //
        template <typename T> FlagExpressionNode<JccType::JG>& GreaterThan(Node<T>& left, Node<T>& right);

        template <JccType JCC, typename T>
        FlagExpressionNode<JCC>& Compare(Node<T>& left, Node<T>& right);


        //
        // Conditional operator
        //
        template <typename T, JccType JCC>
        Node<T>& Conditional(FlagExpressionNode<JCC>& condition, Node<T>& left, Node<T>& right);


        //
        // Call node
        //
        template <typename R>
        Node<R>& Call(Node<R (*)()>& function);

        template <typename R, typename P1>
        Node<R>& Call(Node<R (*)(P1)>& function, Node<P1>& param1);

        template <typename R, typename P1, typename P2>
        Node<R>& Call(Node<R (*)(P1, P2)>& function, Node<P1>& param1, Node<P2>& param2);

        template <typename R, typename P1, typename P2, typename P3>
        Node<R>& Call(Node<R (*)(P1, P2, P3)>& function,
                      Node<P1>& param1,
                      Node<P2>& param2,
                      Node<P3>& param3);

        template <typename R, typename P1, typename P2, typename P3, typename P4>
        Node<R>& Call(Node<R (*)(P1, P2, P3, P4)>& function,
                      Node<P1>& param1,
                      Node<P2>& param2,
                      Node<P3>& param3,
                      Node<P4>& param4);

        //
        // Packed operators
        //
        // TODO: use traits to ensure PACKED is a Packed type.
        template <typename PACKED>
        Node<PACKED>& PackedMax(Node<PACKED>& left, Node<PACKED>& right);

        template <typename PACKED>
        Node<PACKED>& PackedMin(Node<PACKED>& left, Node<PACKED>& right);

    private:
        template <OpCode OP, typename L, typename R> Node<L>& Binary(Node<L>& left, Node<R>& right);
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
    template <typename TO, typename FROM>
    Node<TO>& ExpressionNodeFactory::Cast(Node<FROM>& source)
    {
        return * new (m_allocator.Allocate(sizeof(CastNode<TO, FROM>))) CastNode<TO, FROM>(*this, source);
    }


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
    Node<L>& ExpressionNodeFactory::Sub(Node<L>& left, Node<R>& right)
    {
        return Binary<OpCode::Sub>(left, right);
    }


    template <typename L, typename R>
    Node<L>& ExpressionNodeFactory::Mul(Node<L>& left, Node<R>& right)
    {
        return Binary<OpCode::IMul>(left, right);
    }


    template <typename T, typename INDEX>
    Node<T*>& ExpressionNodeFactory::Add(Node<T*>& array, Node<INDEX>& index)
    {
        // TODO: Implement the following optimization for using Sal instead of Mul for sizeof(T) values
        // that are powers of 2.
        //
        // Need some sort of ShiftImmediate node. ExpressionNodeFactory::Shift(Node<T>, unsigned __int8 shift);
        // This would be different than a binary shift operator node that has two node children.
        //
        //if (sizeof(T) == 8)
        //{
        //    // TODO: Implement.
        //    throw 0;
        //}
        //else if (sizeof(T) == 4)
        //{
        //    auto & size = Immediate(static_cast<unsigned __int8>(2));
        //    auto & offset = Shl(index, size);
        //    return Binary<OpCode::Add>(array, offset);
        //}
        //else if (sizeof(T) == 2)
        //{
        //    // TODO: Implement.
        //    throw 0;
        //}
        //else if (sizeof(T) == 1)
        //{
        //    // TODO: Implement.
        //    throw 0;
        //}
        //else
        {
            auto & size = Immediate<INDEX>(sizeof(T));
            auto & offset = Mul(index, size);
            return Binary<OpCode::Add>(array, offset);
        }
    }


    //
    // Model related
    //
    template <typename PACKED>
    Node<float>& ExpressionNodeFactory::ApplyModel(Node<Model<PACKED>*>& model, Node<PACKED>& packed)
    {
        // TODO: Replace below code once new form of array + index add is implemented.
        auto & array = reinterpret_cast<Node<float*>&>(FieldPointer(model, &Model<PACKED>::m_data));
        auto & index = reinterpret_cast<Node<unsigned __int64>&>(packed);       // TODO: Should this be unsigned __int64?
        return Deref(Add(array, index));
    }


    //
    // Relational operators
    //
    template <typename T>
    FlagExpressionNode<JccType::JG>&
    ExpressionNodeFactory::GreaterThan(Node<T>& left, Node<T>& right)
    {
        return Compare<JccType::JG>(left, right);
    }


    template <JccType JCC, typename T>
    FlagExpressionNode<JCC>&
    ExpressionNodeFactory::Compare(Node<T>& left, Node<T>& right)
    {
        typedef RelationalOperatorNode<T, JCC> NodeType;

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
    template <typename R>
    Node<R>& ExpressionNodeFactory::Call(Node<R (*)()>& function)
    {
        typedef CallNode<R> NodeType;
        return * new (m_allocator.Allocate(sizeof(NodeType)))
                     NodeType(*this, function);
    }


    template <typename R, typename P1>
    Node<R>& ExpressionNodeFactory::Call(Node<R (*)(P1)>& function,
                                         Node<P1>& param1)
    {
        typedef CallNode<R, P1> NodeType;
        return * new (m_allocator.Allocate(sizeof(NodeType)))
                     NodeType(*this, function, param1);
    }


    template <typename R, typename P1, typename P2>
    Node<R>& ExpressionNodeFactory::Call(Node<R (*)(P1, P2)>& function,
                                         Node<P1>& param1,
                                         Node<P2>& param2)
    {
        typedef CallNode<R, P1, P2> NodeType;
        return * new (m_allocator.Allocate(sizeof(NodeType)))
                     NodeType(*this, function, param1, param2);
    }


    template <typename R, typename P1, typename P2, typename P3>
    Node<R>& ExpressionNodeFactory::Call(Node<R (*)(P1, P2, P3)>& function,
                                         Node<P1>& param1,
                                         Node<P2>& param2,
                                         Node<P3>& param3)
    {
        typedef CallNode<R, P1, P2, P3> NodeType;
        return * new (m_allocator.Allocate(sizeof(NodeType)))
                     NodeType(*this, function, param1, param2, param3);
    }


    template <typename R, typename P1, typename P2, typename P3, typename P4>
    Node<R>& ExpressionNodeFactory::Call(Node<R (*)(P1, P2, P3, P4)>& function,
                                         Node<P1>& param1,
                                         Node<P2>& param2,
                                         Node<P3>& param3,
                                         Node<P4>& param4)
    {
        typedef CallNode<R, P1, P2, P3, P4> NodeType;
        return * new (m_allocator.Allocate(sizeof(NodeType)))
                     NodeType(*this, function, param1, param2, param3, param4);
    }


    //
    // PackedMinMax
    //
    template <typename PACKED>
    Node<PACKED>& ExpressionNodeFactory::PackedMax(Node<PACKED>& left, Node<PACKED>& right)
    {
        typedef PackedMinMaxNode<PACKED, true> NodeType;
        return * new (m_allocator.Allocate(sizeof(NodeType)))
                     NodeType(*this, left, right);
    }


    template <typename PACKED>
    Node<PACKED>& ExpressionNodeFactory::PackedMin(Node<PACKED>& left, Node<PACKED>& right)
    {
        typedef PackedMinMaxNode<PACKED, false> NodeType;
        return * new (m_allocator.Allocate(sizeof(NodeType)))
                     NodeType(*this, left, right);
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
