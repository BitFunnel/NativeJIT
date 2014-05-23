#include "stdafx.h"

#include <iostream>

#include "Allocator.h"
#include "Examples.h"
#include "ExpressionTree.h"
#include "ExpressionNodeFactory.h"
#include "X64CodeGenerator.h"


namespace NativeJIT
{
    void TestRegisters()
    {
        for (unsigned i = 0 ; i < 16; ++i)
        {
            std::cout << Register<8, false>(i).GetName() << std::endl;
        }

        std::cout << std::endl;

        for (unsigned i = 0 ; i < 4; ++i)
        {
            std::cout << Register<1, false>(i).GetName() << std::endl;
        }
        for (unsigned i = 8 ; i < 16; ++i)
        {
            std::cout << Register<1, false>(i).GetName() << std::endl;
        }
    }


    void TestOperations()
    {
        X64CodeGenerator code(std::cout);

        Register<8, false> rax(0);
        Register<8, false> rbx(1);

        code.Op("add", rax, rbx);
        code.Op("mov", rax, 1234);
        code.Op("sub", rax, rbx, 789);
        code.Op("not", rax);
    }


    void TestNodes()
    {
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(code);

        Allocator allocator(10000);
        ExpressionNodeFactory factory(allocator, tree);

        auto & a = factory.Immediate(5ull);
        auto & b = factory.Parameter<unsigned __int64*>();
        auto & c = factory.Deref(b);

        auto & d = factory.Parameter<double>();

        tree.Print();
    }


    class InnerClass
    {
    public:
        unsigned __int64 m_a;
        unsigned __int64 m_b;
    };


    class OuterClass
    {
    public:
        long long m_p;
        InnerClass* m_innerPointer;
        InnerClass m_innerEmbedded;
        long long m_q;
    };


    void TestFieldPointerPrimitive()
    {
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(code);

        Allocator allocator(10000);
        ExpressionNodeFactory factory(allocator, tree);

        auto & a = factory.Parameter<InnerClass*>();
        auto & b = factory.FieldPointer(a, &InnerClass::m_b);
        auto & c = factory.Deref(b);

        factory.Return(c);

        tree.Compile();
    }


    void TestFieldPointerEmbedded()
    {
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(code);

        Allocator allocator(10000);
        ExpressionNodeFactory factory(allocator, tree);

        auto & a = factory.Parameter<OuterClass*>();
        auto & b = factory.FieldPointer(a, &OuterClass::m_innerEmbedded);
        auto & c = factory.FieldPointer(b, &InnerClass::m_b);
        auto & d = factory.Deref(c);

        factory.Return(d);

        tree.Compile();
    }


    void TestBinary()
    {
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(code);

        Allocator allocator(10000);
        ExpressionNodeFactory factory(allocator, tree);

        auto & a = factory.Parameter<unsigned __int64>();
        auto & b = factory.Parameter<unsigned __int64*>();
        auto & c = factory.Deref(b);
        auto & d = factory.Add(a, c);

        factory.Return(d);

        tree.Compile();
    }


    void TestFactory()
    {
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(code);

        Allocator allocator(10000);
        ExpressionNodeFactory factory(allocator, tree);

        auto & a = factory.Parameter<OuterClass*>();
        auto & b = factory.FieldPointer(a, &OuterClass::m_innerEmbedded);
        auto & c = factory.FieldPointer(b, &InnerClass::m_b);
        auto & d = factory.Deref(c);

        auto & e = factory.Immediate(1234ull);

        auto & f = factory.Add(e, d);

        factory.Return(f);

        tree.Compile();
    }


    void TestArray()
    {
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(code);

        Allocator allocator(10000);
        ExpressionNodeFactory factory(allocator, tree);

        auto & a = factory.Parameter<OuterClass*>();
        auto & b = factory.Parameter<unsigned __int64>();
        auto & c = factory.Add(a, b);
        auto & d = factory.FieldPointer(c, &OuterClass::m_q);
        auto & e = factory.Deref(d);

        factory.Return(e);

        tree.Compile();
    }


    void TestByte()
    {
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(code);

        Allocator allocator(10000);
        ExpressionNodeFactory factory(allocator, tree);

        auto & a = factory.Parameter<char>();
        auto & b = factory.Immediate<char>(123);
        auto & c = factory.Add(a, b);

        auto & d = factory.Parameter<char*>();
        auto & e = factory.Deref(d);

        auto & f = factory.Add(c, e);

        factory.Return(f);

        tree.Compile();
    }


    void RunTests()
    {
//        TestRegisters();
//        TestOperations();
//        TestNodes();
//        TestFieldPointerPrimitive();
//        TestFieldPointerEmbedded();
//        TestBinary();
//        TestFactory();
//        TestArray();
        TestByte();
//        JITExample1();
    }
}

int _tmain(int argc, char* argv[])
{
    NativeJIT::RunTests();

    return 0;
}

