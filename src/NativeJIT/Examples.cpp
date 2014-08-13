#include "stdafx.h"

#include <iostream>

#include "Examples.h"
#include "ExpressionNodeFactory2.h"
#include "ExpressionTree.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "Temporary/Allocator.h"


namespace NativeJIT
{
    struct C
    {
        unsigned __int64 m_x;
        unsigned __int64 m_y;
    };

    struct B
    {
        unsigned __int64 m_p;
        C* m_objectPointer;
        unsigned __int64 m_q;
    };

    struct A
    {
        unsigned __int64 m_a;
        B m_embeddedObject;
        unsigned __int64 m_b;
    };


    unsigned __int64 Example1(A *arrayOfA, unsigned __int64 index, unsigned __int64* pointer, unsigned __int64 value)
    {
        return 0x7890ull + value + *pointer + arrayOfA[index].m_embeddedObject.m_objectPointer->m_y;
    }


    void JITExample1()
    {
        ExecutionBuffer allocator(5000);
        FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);
        ExpressionNodeFactory2 factory(allocator, code);

        auto & arrayOfA = factory.Parameter<A*>();
        auto & index = factory.Parameter<unsigned __int64>();
        auto & pointer = factory.Parameter<unsigned __int64*>();
        auto & value = factory.Parameter<unsigned __int64>();

        auto & a = factory.Immediate(0x7890ull);
        auto & b = factory.Add(value, a);

        auto & c = factory.Deref(pointer);
        auto & d = factory.Add(b, c);

        auto & e = factory.Add(arrayOfA, index);
        auto & f = factory.FieldPointer(e, &A::m_embeddedObject);
        auto & g = factory.FieldPointer(f, &B::m_objectPointer);
        auto & h = factory.Deref(g);
        auto & i = factory.FieldPointer(h, &C::m_y);
        auto & j = factory.Deref(i);

        auto & k = factory.Add(d, j);

        factory.Return(k);

        factory.Compile();

        typedef unsigned __int64 (*F)(A *, unsigned __int64, unsigned __int64*, unsigned __int64);

        F entry = reinterpret_cast<F>(code.GetEntryPoint());

        struct C structC;
        structC.m_y = 0x560000;

        unsigned __int64 index2 = 3;

        struct A arrayOfA2[5];
        arrayOfA2[index2].m_embeddedObject.m_objectPointer = &structC;

        unsigned __int64 int64Value = 0x34000000;
        unsigned __int64* pointer2 = &int64Value;

        unsigned __int64 value2 = 0x1200000000;

        unsigned __int64 result = entry(arrayOfA2, index2, pointer2, value2);

        std::cout << "Result = 0x" << std::hex << result << std::endl;
    }


    void ConditionalExample()
    {
        ExecutionBuffer allocator(5000);
        FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);
        ExpressionNodeFactory2 factory(allocator, code);

        auto & a = factory.Immediate(5ull);
        auto & b = factory.Immediate(6ull);
        auto & c = factory.GreaterThan(a, b);
        auto & d = factory.Conditional(c, a, b);

        factory.Return(d);

        factory.Compile();
    }
}
