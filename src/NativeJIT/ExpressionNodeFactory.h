#pragma once

#include "BinaryImmediateNode.h"
#include "BinaryNode.h"
#include "CallNode.h"
#include "CastNode.h"
#include "ConditionalNode.h"
#include "ExpressionTree.h"             // Base class.
#include "FieldPointerNode.h"
#include "ImmediateNode.h"
#include "IndirectNode.h"
#include "NativeJIT/BitOperations.h"
#include "NativeJIT/Model.h"
#include "Node.h"
#include "PackedMinMaxNode.h"
#include "ParameterNode.h"
#include "ReturnNode.h"
#include "ShldNode.h"
#include "StackVariableNode.h"
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
        template <typename T> ImmediateNode<T>& Immediate(T value);
        template <typename T> ParameterNode<T>& Parameter(unsigned position);

        // See StackVariableNode for important information about stack variable
        // lifetime.
        template <typename T> Node<T&>& StackVariable();


        //
        // Unary operators
        //
        template <typename T> Node<T*>& AsPointer(Node<T&>& reference);
        template <typename T> Node<T&>& AsReference(Node<T*>& pointer);
        template <typename TO, typename FROM> Node<TO>& Cast(Node<FROM>& value);
        template <typename T> Node<T>& Deref(Node<T*>& pointer);
        template <typename T> Node<T>& Deref(Node<T*>& pointer, __int32 index);
        template <typename T> Node<T>& Deref(Node<T&>& reference);

        // Note: OBJECT1 is there to allow for template deduction in all cases
        // since OBJECT may or may not be const, but OBJECT1 is never const
        // in FieldPointer(someObjectNode, &SomeObject::m_field) expression.
        // Otherwise, the following code would fail to compile:
        //
        // Node<SomeObject const *>& obj = ...;
        // FieldPointer(obj, &SomeObject::m_field)
        template <typename OBJECT, typename FIELD, typename OBJECT1 = OBJECT>
        Node<FIELD*>& FieldPointer(Node<OBJECT*>& object, FIELD OBJECT1::*field);

        template <typename T> NodeBase& Return(Node<T>& value);


        //
        // Binary arithmetic operators
        //
        template <typename L, typename R> Node<L>& Add(Node<L>& left, Node<R>& right);
        template <typename L, typename R> Node<L>& And(Node<L>& left, Node<R>& right);
        template <typename L, typename R> Node<L>& Mul(Node<L>& left, Node<R>& right);
        template <typename L, typename R> Node<L>& MulImmediate(Node<L>& left, R right);
        template <typename L, typename R> Node<L>& Or(Node<L>& left, Node<R>& right);
        template <typename L, typename R> Node<L>& Sal(Node<L>& left, R right);
        template <typename L, typename R> Node<L>& Sub(Node<L>& left, Node<R>& right);

        template <typename T, size_t SIZE, typename INDEX>
        Node<T*>& Add(Node<T(*)[SIZE]>& array, Node<INDEX>& index);

        template <typename T, typename INDEX> Node<T*>& Add(Node<T*>& array, Node<INDEX>& index);

        //
        // Ternary arithmetic operators
        template <typename T>
        Node<T>& Shld(Node<T>& shiftee, Node<T>& filler, unsigned __int8 bitCount);

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
        // Conditional operators
        //

        // WARNING: Both trueValue and falseValue are evaluated before testing the
        // condition so both must be legal to evaluate regardless of the result
        // of the condition. See the TODO note in ConditionalNode::CodeGenValue.
        template <typename T, JccType JCC>
        Node<T>& Conditional(FlagExpressionNode<JCC>& condition, Node<T>& trueValue, Node<T>& falseValue);

        // WARNING: Both trueValue and falseValue are evaluated before testing the
        // condition so both must be legal to evaluate regardless of the result
        // of the condition. See the TODO note in ConditionalNode::CodeGenValue.
        template <typename CONDT, typename T>
        Node<T>& IfNotZero(Node<CONDT>& conditionValue, Node<T>& trueValue, Node<T>& falseValue);

        // WARNING: Both thenValue and elseValue are evaluated before testing the
        // condition so both must be legal to evaluate regardless of the result
        // of the condition. See the TODO note in ConditionalNode::CodeGenValue.
        template <typename T>
        Node<T>& If(Node<bool>& conditionValue, Node<T>& thenValue, Node<T>& elseValue);


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
        template <OpCode OP, typename L, typename R> Node<L>& Binary(Node<L>& left, R right);
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
    ImmediateNode<T>& ExpressionNodeFactory::Immediate(T value)
    {
        return * new (m_allocator.Allocate(sizeof(ImmediateNode<T>))) ImmediateNode<T>(*this, value);
    }


    template <typename T>
    ParameterNode<T>& ExpressionNodeFactory::Parameter(unsigned position)
    {
        return * new (m_allocator.Allocate(sizeof(ParameterNode<T>))) ParameterNode<T>(*this, position);
    }


    template <typename T>
    Node<T&>& ExpressionNodeFactory::StackVariable()
    {
        return * new (m_allocator.Allocate(sizeof(StackVariableNode<T>))) StackVariableNode<T>(*this);
    }


    //
    // Unary operators
    //

    template <typename T>
    Node<T*>& ExpressionNodeFactory::AsPointer(Node<T&>& reference)
    {
        return Cast<T*>(reference);
    }


    template <typename T>
    Node<T&>& ExpressionNodeFactory::AsReference(Node<T*>& pointer)
    {
        return Cast<T&>(pointer);
    }


    template <typename TO, typename FROM>
    Node<TO>& ExpressionNodeFactory::Cast(Node<FROM>& source)
    {
        return * new (m_allocator.Allocate(sizeof(CastNode<TO, FROM>))) CastNode<TO, FROM>(*this, source);
    }


    template <typename T>
    Node<T>& ExpressionNodeFactory::Deref(Node<T*>& pointer)
    {
        return Deref(pointer, 0);
    }


    template <typename T>
    Node<T>& ExpressionNodeFactory::Deref(Node<T*>& pointer, __int32 index)
    {
        return * new (m_allocator.Allocate(sizeof(IndirectNode<T>))) IndirectNode<T>(*this, pointer, index);
    }


    template <typename T>
    Node<T>& ExpressionNodeFactory::Deref(Node<T&>& reference)
    {
        return Deref(AsPointer(reference));
    }


    template <typename OBJECT, typename FIELD, typename OBJECT1>
    Node<FIELD*>&
    ExpressionNodeFactory::FieldPointer(Node<OBJECT*>& object, FIELD OBJECT1::*field)
    {
        static_assert(std::is_same<typename std::remove_const<OBJECT>::type,
                                   typename std::remove_const<OBJECT1>::type>::value,
                      "Mismatch between the provided object type and field's parent object type");

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
    Node<L>& ExpressionNodeFactory::And(Node<L>& left, Node<R>& right)
    {
        return Binary<OpCode::And>(left, right);
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


    template <typename L, typename R>
    Node<L>& ExpressionNodeFactory::MulImmediate(Node<L>& left, R right)
    {
        Node<L>* result;

        if (right == 0)
        {
            result = &Immediate<L>(0);
        }
        else if (right == 1)
        {
            result = &left;
        }
        else if (BitOp::GetNonZeroBitCount(right) == 1)
        {
            // Note: not checking return value of GetLowestBitSet() as it's
            // guaranteed to return an index when a bit is set.
            unsigned bitIndex;
            BitOp::GetLowestBitSet(right, &bitIndex);

            result = &Sal(left, static_cast<unsigned __int8>(bitIndex));
        }
        else
        {
            result = &Binary<OpCode::IMul>(left, right);
        }

        return *result;
    }


    template <typename L, typename R>
    Node<L>& ExpressionNodeFactory::Or(Node<L>& left, Node<R>& right)
    {
        return Binary<OpCode::Or>(left, right);
    }


    template <typename L, typename R>
    Node<L>& ExpressionNodeFactory::Sal(Node<L>& left, R right)
    {
        return Binary<OpCode::Sal>(left, right);
    }


    template <typename T>
    Node<T>& ExpressionNodeFactory::Shld(Node<T>& shiftee, Node<T>& filler, unsigned __int8 bitCount)
    {
        return * new (m_allocator.Allocate(sizeof(ShldNode<T>))) 
            ShldNode<T>(*this, shiftee, filler, bitCount);
    }


    template <typename T, typename INDEX>
    Node<T*>& ExpressionNodeFactory::Add(Node<T*>& array, Node<INDEX>& index)
    {
        // Cast the index to UInt64 to make sure that the calculated offset
        // will not overflow. This will also make it possible to use OpCode::Add
        // on the result regardless of sizeof(INDEX) since both T* and UInt64
        // use the same register size.
        auto & index64 = Cast<unsigned __int64>(index);

        // The IMul instruction doesn't suport 64-bit immediates, but there's
        // also no need to support types whose size is larger than UINT32_MAX.
        static_assert(sizeof(T) <= UINT32_MAX, "Unsupported type");
        auto & offset = MulImmediate(index64, static_cast<unsigned __int32>(sizeof(T)));

        return Binary<OpCode::Add>(array, offset);
    }


    template <typename T, size_t SIZE, typename INDEX>
    Node<T*>& ExpressionNodeFactory::Add(Node<T(*)[SIZE]>& array, Node<INDEX>& index)
    {
        return Add(Cast<T*>(array), index);
    }


    //
    // Model related
    //
    template <typename PACKED>
    Node<float>& ExpressionNodeFactory::ApplyModel(Node<Model<PACKED>*>& model, Node<PACKED>& packed)
    {
        auto & array = FieldPointer(model, &Model<PACKED>::m_data);
        return Deref(Add(array, packed));
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
    // Conditional operators
    //
    template <typename T, JccType JCC>
    Node<T>& ExpressionNodeFactory::Conditional(FlagExpressionNode<JCC>& condition,
                                                Node<T>& trueValue,
                                                Node<T>& falseValue)
    {
        typedef ConditionalNode<T, JCC> NodeType;
        return * new (m_allocator.Allocate(sizeof(NodeType)))
                     NodeType(*this, condition, trueValue, falseValue);
    }


    template <typename CONDT, typename T>
    Node<T>& ExpressionNodeFactory::IfNotZero(Node<CONDT>& conditionValue, Node<T>& trueValue, Node<T>& falseValue)
    {
        // TODO: This can be achieved with a FlagExpressionNode implementation in
        // terms of the x64 test instruction, once the instruction is available.
        auto & conditionNode = Compare<JccType::JNE>(conditionValue, Immediate<CONDT>(0));

        return Conditional(conditionNode, trueValue, falseValue);
    }


    template <typename T>
    Node<T>& ExpressionNodeFactory::If(Node<bool>& conditionValue, Node<T>& thenValue, Node<T>& elseValue)
    {
        return IfNotZero(conditionValue, thenValue, elseValue);
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


    template <OpCode OP, typename L, typename R>
    Node<L>& ExpressionNodeFactory::Binary(Node<L>& left, R right)
    {
        return * new (m_allocator.Allocate(sizeof(BinaryImmediateNode<OP, L, R>))) 
                     BinaryImmediateNode<OP, L, R>(*this, left, right);
    }
}
