#include "stdafx.h"

#include <iostream>

#include "Allocator.h"
#include "Examples.h"
#include "ExpressionNodeFactory.h"
#include "ExpressionTree.h"
#include "X64CodeGenerator.h"


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
        return 1234ull + value + *pointer + arrayOfA[index].m_embeddedObject.m_objectPointer->m_y;
    }


    void JITExample1()
    {
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(code);

        Allocator allocator(10000);
        ExpressionNodeFactory factory(allocator, tree);

        auto & arrayOfA = factory.Parameter<A*>();
        auto & index = factory.Parameter<unsigned __int64>();
        auto & pointer = factory.Parameter<unsigned __int64*>();
        auto & value = factory.Parameter<unsigned __int64>();

        auto & a = factory.Immediate(1234ull);
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

        tree.Compile();
    }
}
