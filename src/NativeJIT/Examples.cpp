#include "stdafx.h"

unsigned fooExamples;

//
//#include <iostream>
//
//#include "Examples.h"
//#include "ExpressionNodeFactory.h"
//#include "ExpressionTree.h"
//#include "NativeJIT/ExecutionBuffer.h"
//#include "NativeJIT/FunctionBuffer.h"
//#include "Temporary/Allocator.h"
//
//
//namespace NativeJIT
//{
//    struct C
//    {
//        uint64_t m_x;
//        uint64_t m_y;
//    };
//
//    struct B
//    {
//        uint64_t m_p;
//        C* m_objectPointer;
//        uint64_t m_q;
//    };
//
//    struct A
//    {
//        uint64_t m_a;
//        B m_embeddedObject;
//        uint64_t m_b;
//    };
//
//
//    uint64_t Example1(A *arrayOfA, uint64_t index, uint64_t* pointer, uint64_t value)
//    {
//        return 0x7890ull + value + *pointer + arrayOfA[index].m_embeddedObject.m_objectPointer->m_y;
//    }
//
//
//    void JITExample1()
//    {
//        ExecutionBuffer allocator(5000);
//        FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);
//        ExpressionNodeFactory factory(allocator, code);
//
//        auto & arrayOfA = factory.Parameter<A*>();
//        auto & index = factory.Parameter<uint64_t>();
//        auto & pointer = factory.Parameter<uint64_t*>();
//        auto & value = factory.Parameter<uint64_t>();
//
//        auto & a = factory.Immediate(0x7890ull);
//        auto & b = factory.Add(value, a);
//
//        auto & c = factory.Deref(pointer);
//        auto & d = factory.Add(b, c);
//
//        auto & e = factory.Add(arrayOfA, index);
//        auto & f = factory.FieldPointer(e, &A::m_embeddedObject);
//        auto & g = factory.FieldPointer(f, &B::m_objectPointer);
//        auto & h = factory.Deref(g);
//        auto & i = factory.FieldPointer(h, &C::m_y);
//        auto & j = factory.Deref(i);
//
//        auto & k = factory.Add(d, j);
//
//        factory.Return(k);
//
//        factory.Compile();
//
//        typedef uint64_t (*F)(A *, uint64_t, uint64_t*, uint64_t);
//
//        F entry = reinterpret_cast<F>(code.GetEntryPoint());
//
//        struct C structC;
//        structC.m_y = 0x560000;
//
//        uint64_t index2 = 3;
//
//        struct A arrayOfA2[5];
//        arrayOfA2[index2].m_embeddedObject.m_objectPointer = &structC;
//
//        uint64_t int64Value = 0x34000000;
//        uint64_t* pointer2 = &int64Value;
//
//        uint64_t value2 = 0x1200000000;
//
//        uint64_t result = entry(arrayOfA2, index2, pointer2, value2);
//
//        std::cout << "Result = 0x" << std::hex << result << std::endl;
//    }
//
//
//    void ConditionalExample()
//    {
//        ExecutionBuffer allocator(5000);
//        FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);
//        ExpressionNodeFactory factory(allocator, code);
//
//        auto & a = factory.Immediate(5ull);
//        auto & b = factory.Immediate(6ull);
//        auto & c = factory.GreaterThan(a, b);
//        auto & d = factory.Conditional(c, a, b);
//
//        factory.Return(d);
//
//        factory.Compile();
//    }
//}
