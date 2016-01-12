#pragma once

#include <cstdint>

#include "NativeJIT/CodeGen/X64CodeGenerator.h" // JccType.
#include "NativeJIT/ExpressionTreeDecls.h"      // Base class.
#include "NativeJIT/Model.h"                    // Parameter.
#include "NativeJIT/Nodes/ImmediateNodeDecls.h" // Parameter too cumbersome to forward declare.


namespace NativeJIT
{
    template <JccType JCC>
    class FlagExpressionNode;

    template <typename T>
    class Node;

    class NodeBase;

    template <typename T>
    class ParameterNode;

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
        template <typename T> Node<T>& Deref(Node<T*>& pointer, int32_t index);
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
        Node<T>& Shld(Node<T>& shiftee, Node<T>& filler, uint8_t bitCount);

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
}
