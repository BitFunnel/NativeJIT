#include "stdafx.h"

#include <iostream>

#include "Binary.h"
#include "ExpressionTree.h"
#include "FieldPointer.h"
#include "Immediate.h"
#include "Indirect.h"
#include "Parameter.h"
#include "Return.h"
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

        Immediate<long long> x(tree, 5);

        Parameter<long long*> y(tree);
        Parameter<double> z(tree);

        Indirect<long long> a(tree, y, 5);

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
    };


    void TestFieldPointerPrimitive()
    {
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(code);

        Parameter<InnerClass*> a(tree);
        FieldPointer<InnerClass, unsigned __int64> b(tree, a, &InnerClass::m_b);
        Indirect<unsigned __int64> c(tree, b, 0);
        Return<unsigned __int64> d(tree, c);

        tree.Compile();
    }


    void TestFieldPointerEmbedded()
    {
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(code);

        Parameter<OuterClass*> a(tree);
        FieldPointer<OuterClass, InnerClass> b(tree, a, &OuterClass::m_innerEmbedded);
        FieldPointer<InnerClass, unsigned __int64> c(tree, b, &InnerClass::m_b);
        Indirect<unsigned __int64> d(tree, c, 0);
        Return<unsigned __int64> e(tree, d);

        tree.Compile();
    }


    void TestBinary()
    {
        X64CodeGenerator code(std::cout);
        ExpressionTree tree(code);

        Parameter<unsigned __int64> a(tree);
//        Immediate<unsigned __int64> b(tree, 1234ULL);
        Parameter<unsigned __int64*> bptr(tree);
        Indirect<unsigned __int64> b(tree, bptr, 0);
        Binary<unsigned __int64, unsigned __int64> c(tree, "add", a, b);
        Return<unsigned __int64> d(tree, c);

        tree.Compile();
    }


    void RunTests()
    {
//        TestRegisters();
//        TestOperations();
//        TestNodes();
//        TestFieldPointerPrimitive();
//        TestFieldPointerEmbedded();
        TestBinary();
    }
}

int _tmain(int argc, char* argv[])
{
    NativeJIT::RunTests();

    return 0;
}

