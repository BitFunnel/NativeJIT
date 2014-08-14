#include "stdafx.h"

#pragma warning(disable:4505)

#include <iostream>

#include "CallNode.h"
#include "ConditionalNode.h"
#include "Examples.h"
#include "ExpressionTree.h"
#include "ExpressionNodeFactory.h"
#include "Function.h"
#include "NativeJIT/ExecutionBuffer.h"
#include "NativeJIT/FunctionBuffer.h"
#include "Storage.h"
#include "Temporary/Allocator.h"


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


    //void TestImmediate(Allocators::IAllocator& allocator, FunctionBuffer& code)
    //{
    //    // Is this worth it, vx just manually resetting after the block (even if a try/catch isn't used)?
    //    AutoResetAllocator reset(allocator);

    //    {
    //        Function<unsigned __int64> function(allocator, code);

    //        auto & a = function.Immediate(0x1234ull);
    //        auto f = function.Compile(a);

    //        auto x = f();

    //        std::cout << "x = " << x << std::endl;
    //    }
    //}


    //void TestFieldPointerPrimitive()
    //{
    //    ExecutionBuffer allocator(5000);
    //    FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);
    //    ExpressionNodeFactory factory(allocator, code);

    //    auto & a = factory.Parameter<InnerClass*>();
    //    auto & b = factory.FieldPointer(a, &InnerClass::m_b);
    //    auto & c = factory.Deref(b);

    //    factory.Return(c);

    //    factory.Compile();
    //}


    //void TestFieldPointerEmbedded()
    //{
    //    ExecutionBuffer allocator(5000);
    //    FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);
    //    ExpressionNodeFactory factory(allocator, code);

    //    auto & a = factory.Parameter<OuterClass*>();
    //    auto & b = factory.FieldPointer(a, &OuterClass::m_innerEmbedded);
    //    auto & c = factory.FieldPointer(b, &InnerClass::m_b);
    //    auto & d = factory.Deref(c);

    //    factory.Return(d);

    //    factory.Compile();
    //}


    //void TestBinary()
    //{
    //    ExecutionBuffer allocator(5000);
    //    FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);
    //    ExpressionNodeFactory factory(allocator, code);

    //    auto & a = factory.Parameter<unsigned __int64>();
    //    auto & b = factory.Parameter<unsigned __int64*>();
    //    auto & c = factory.Deref(b);
    //    auto & d = factory.Add(a, c);

    //    factory.Return(d);

    //    factory.Compile();
    //}


    void TestFactory()
    {
        ExecutionBuffer allocator(5000);
        FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);
        ExpressionNodeFactory factory(allocator, code);

        auto & a = factory.Parameter<OuterClass*>();
        auto & b = factory.FieldPointer(a, &OuterClass::m_innerEmbedded);
        auto & c = factory.FieldPointer(b, &InnerClass::m_b);
        auto & d = factory.Deref(c);

        auto & e = factory.Immediate(1234ull);

        auto & f = factory.Add(e, d);

        factory.Return(f);

        factory.Compile();
    }


    //void TestArray()
    //{
    //    ExecutionBuffer allocator(5000);
    //    FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);
    //    ExpressionNodeFactory factory(allocator, code);

    //    auto & a = factory.Parameter<OuterClass*>();
    //    auto & b = factory.Parameter<unsigned __int64>();
    //    auto & c = factory.Add(a, b);
    //    auto & d = factory.FieldPointer(c, &OuterClass::m_q);
    //    auto & e = factory.Deref(d);

    //    factory.Return(e);

    //    factory.Compile();
    //}


    void TestByte()
    {
        ExecutionBuffer allocator(5000);
        FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);
        ExpressionNodeFactory factory(allocator, code);

        auto & a = factory.Parameter<char>();
        auto & b = factory.Immediate<char>(123);
        auto & c = factory.Add(a, b);

        auto & d = factory.Parameter<char*>();
        auto & e = factory.Deref(d);

        auto & f = factory.Add(c, e);

        factory.Return(f);

        factory.Compile();
    }


    void TestDouble()
    {
        //ExecutionBuffer allocator(5000);
        //FunctionBuffer code(allocator, 2000, 3, 0, false);
        //ExpressionTree tree(allocator, code);
        //ExpressionNodeFactory factory(allocator, tree);

        //auto & a = factory.Parameter<double>();
        //auto & b = factory.Immediate<double>(123);
        //auto & c = factory.Add(a, b);

        //auto & d = factory.Parameter<double*>();
        //auto & e = factory.Deref(d);

        //auto & f = factory.Add(c, e);

        //factory.Return(f);

        //tree.Compile();
    }


    void TestLabel()
    {
        ExecutionBuffer allocator(5000);
        FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);

        Label l1 = code.AllocateLabel();
        code.PlaceLabel(l1);
    }


    //void TestConditional()
    //{
    //    ExecutionBuffer allocator(5000);
    //    FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);
    //    ExpressionNodeFactory factory(allocator, code);

    //    auto & a = factory.Immediate(5ull);
    //    auto & b = factory.Immediate(6ull);

    //    auto & c = factory.Immediate(12345ull);

    //    IsTrue<unsigned __int64> d(factory, c);     // What is this line for?

    //    auto & e = factory.GreaterThan(a, b);
    //    auto & f = factory.Conditional(e, a, b);

    //    factory.Return(f);

    //    factory.Compile();
    //}


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
        ExecutionBuffer allocator(5000);
        FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);
        ExpressionNodeFactory factory(allocator, code);

        auto & a = factory.Immediate<unsigned __int64>(5);
        auto & b = factory.Immediate<unsigned __int64>(6);
        auto & c = factory.Add(a, b);
        factory.Return(c);

        factory.Compile();
    }


    // To test register spilling, it is necessary to modify ExpressionTree to only allow a single register.
    void TestSpill()
    {
        ExecutionBuffer allocator(5000);
        FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);
        ExpressionNodeFactory factory(allocator, code);

        auto & a = factory.Immediate<unsigned __int64>(5);
        auto & b = factory.Immediate<unsigned __int64>(6);
        auto & c = factory.Add(a, b);

        auto & d = factory.Immediate<unsigned __int64>(7);
        auto & e = factory.Immediate<unsigned __int64>(8);
        auto & f = factory.Add(d, e);

        auto & g = factory.Add(c, f);

        factory.Return(g);

        factory.Compile();
    }


    //void TestFunction1(Allocators::IAllocator& allocator, FunctionBuffer& code)
    //{
    //    AutoResetAllocator reset(allocator);

    //    {
    //        Function<__int64, __int64> function(allocator, code);

    //        auto & a = function.Add(function.GetP1(), function.Immediate<unsigned __int64>(5ull));
    //        auto f = function.Compile(a);

    //        auto x = f(1);
    //        std::cout << "x = " << x << std::endl;
    //    }
    //}


    //void TestFunction2(Allocators::IAllocator& allocator, FunctionBuffer& code)
    //{
    //    AutoResetAllocator reset(allocator);

    //    {
    //        Function<__int64, __int64, __int64> function(allocator, code);

    //        auto & a = function.Add(function.GetP2(), function.GetP1());
    //        auto f = function.Compile(a);

    //        auto x = f(1, 2);
    //        std::cout << "x = " << x << std::endl;
    //    }
    //}


    int SampleFunction(int x, int y)
    {
        return x + y;
    }


    void TestCall()
    {
        ExecutionBuffer allocator(5000);
        FunctionBuffer code(allocator, 2000, 10, 10, 3, 0, false);
        ExpressionNodeFactory factory(allocator, code);

        auto & a = factory.Immediate<int>(5);
        auto & b = factory.Immediate<int>(6);

        typedef int (*F)(int, int);
        auto &function = factory.Immediate<F>(SampleFunction);

        auto & d = factory.Call(function, a, b);

        factory.Return(d);
        factory.Compile();
    }


    void RunTests()
    {
        ExecutionBuffer executionBuffer(5000);
        FunctionBuffer code(executionBuffer, 2000, 10, 10, 3, 0, false);

        Allocator allocator(5000);



//        TestConditional();
//        JITExample1();
//        ConditionalExample();

//        TestRegisters();
//        TestOperations();
//        TestNodes();
//        TestFactory();
//        TestByte();
//        TestDouble();
//        TestLabel();
//        TestArray();
//        TestFieldPointerPrimitive();
//        TestFieldPointerEmbedded();
//        TestBinary();
//        TestStorage();
//        TestStorageIntegration();
//        TestSpill();

//        TestFunction1(allocator, code);
//        TestFunction2(allocator, code);
//        TestImmediate(allocator, code);
//        TestCall();

//        JITExample1();
    }
}


//int _tmain(int /*argc*/, char* /*argv*/[])
//{
//    NativeJIT::RunTests();
//
//    return 0;
//}
