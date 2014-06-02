#include "stdafx.h"

#include <iostream>

#include "Allocator.h"
#include "Examples.h"
#include "ExpressionTree.h"
#include "ExpressionNodeFactory.h"
#include "Storage.h"
#include "X64CodeGenerator.h"


#include "ConditionalNode.h"

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


    //void TestOperations()
    //{
    //    X64CodeGenerator code(std::cout);

    //    Register<8, false> rax(0);
    //    Register<8, false> rbx(1);

    //    code.Op("add", rax, rbx);
    //    code.Op("mov", rax, 1234);
    //    code.Op("sub", rax, rbx, 789);
    //    code.Op("not", rax);
    //}


    void TestNodes()
    {
        Allocator allocator(10000);
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(allocator, code);
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
        Allocator allocator(10000);
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(allocator, code);
        ExpressionNodeFactory factory(allocator, tree);

        auto & a = factory.Parameter<InnerClass*>();
        auto & b = factory.FieldPointer(a, &InnerClass::m_b);
        auto & c = factory.Deref(b);

        factory.Return(c);

        tree.Compile();
    }


    void TestFieldPointerEmbedded()
    {
        Allocator allocator(10000);
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(allocator, code);
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
        Allocator allocator(10000);
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(allocator, code);
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
        Allocator allocator(10000);
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(allocator, code);
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
        Allocator allocator(10000);
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(allocator, code);
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
        Allocator allocator(10000);
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(allocator, code);
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


    void TestDouble()
    {
        Allocator allocator(10000);
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(allocator, code);
        ExpressionNodeFactory factory(allocator, tree);

        auto & a = factory.Parameter<double>();
        auto & b = factory.Immediate<double>(123);
        auto & c = factory.Add(a, b);

        auto & d = factory.Parameter<double*>();
        auto & e = factory.Deref(d);

        auto & f = factory.Add(c, e);

        factory.Return(f);

        tree.Compile();
    }


    void TestLabel()
    {
        X64CodeGenerator code(std::cout);

        Label l1 = code.AllocateLabel();
        code.PlaceLabel(l1);
    }


    void TestConditional()
    {
        Allocator allocator(10000);
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(allocator, code);
        ExpressionNodeFactory factory(allocator, tree);

        auto & a = factory.Immediate(5ull);
        auto & b = factory.Immediate(6ull);

        auto & c = factory.Immediate(12345ull);
        IsTrue<unsigned __int64> d(tree, c);

        auto & e = factory.GreaterThan(a, b);
        auto & f = factory.Conditional(e, a, b);

        factory.Return(f);

        tree.Compile();
    }


    template <typename T>
    Storage<T> GetStorage(ExpressionTree& tree)
    {
        auto r = tree.AllocateRegister<Storage<T>::BaseRegister>();
        return Storage<T>(tree, r, 1234ull);
    }


//    void TestStorage()
//    {
//        Allocator allocator(10000);
//        X64CodeGenerator code(std::cout);
//        ExpressionTree tree(allocator, code);
//        ExpressionNodeFactory factory(allocator, tree);
//
////        tree.Pass1();
//////        auto r = tree.AllocateRegister<Register<8, false>>();
////
////        {
////            //Storage<unsigned __int64> x(tree, r, 1234ull);
////            auto x = GetStorage<unsigned __int64>(tree);
////
////            {
////                auto y = x;
////
////                {
////                    auto z(y);
////                }
////            }
////        }
//
//        tree.Pass1();
//        auto r = tree.AllocateRegister<Register<8, false>>();
////        Storage<unsigned __int64> x(tree, r, 1234ull);
//        Storage<double> x(tree, r, 1234ull);
//
//        //auto y = x;
//
//        x.ConvertToValue(tree, false);
//        x.ConvertToValue(tree, true);
//    }


    void TestStorageIntegration()
    {
        Allocator allocator(10000);
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(allocator, code);
        ExpressionNodeFactory factory(allocator, tree);

        auto & a = factory.Immediate<unsigned __int64>(5);
        auto & b = factory.Immediate<unsigned __int64>(6);
        auto & c = factory.Add(a, b);
        factory.Return(c);

        tree.Compile();
    }


    // To test register spilling, it is necessary to modify ExpressionTree to only allow a single register.
    void TestSpill()
    {
        Allocator allocator(10000);
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(allocator, code);
        ExpressionNodeFactory factory(allocator, tree);

        auto & a = factory.Immediate<unsigned __int64>(5);
        auto & b = factory.Immediate<unsigned __int64>(6);
        auto & c = factory.Add(a, b);

        auto & d = factory.Immediate<unsigned __int64>(7);
        auto & e = factory.Immediate<unsigned __int64>(8);
        auto & f = factory.Add(d, e);

        auto & g = factory.Add(c, f);

        factory.Return(g);

        tree.Compile();
    }


    void RunTests()
    {
//        TestConditional();
        JITExample1();
//        ConditionalExample();

//        TestRegisters();
//        TestOperations();
//        TestNodes();
//        TestFactory();
        TestByte();
        TestDouble();
//        TestLabel();
//        TestArray();
//        TestFieldPointerPrimitive();
//        TestFieldPointerEmbedded();
//        TestBinary();
//        TestStorage();
//        TestStorageIntegration();
//        TestSpill();
    }
}


int _tmain(int argc, char* argv[])
{
    NativeJIT::RunTests();

    return 0;
}

