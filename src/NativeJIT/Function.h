#pragma once
//
//#include <ostream>
//
//#include "ExpressionNodeFactory.h"
//#include "ExpressionTree.h"
//#include "NativeJIT/FunctionBuffer.h"
//#include "ParameterNode.h"
//
//
//namespace NativeJIT
//{
//    class ExpressionNodeFactory;
//    class X64CodeGenerator;
//
//    class FunctionBase
//    {
//    public:
//        FunctionBase(Allocators::IAllocator& allocator, FunctionBuffer& code);
//
//        // TODO: Can't be const if m_factory is embedded.
//        ExpressionNodeFactory& GetFactory();
//
//        void Compile();
//
//        void  const *GetUntypedEntryPoint() const;
//
//    private:
//        // TODO: Find a better solution to this problem.
//        // WARNING: m_code is a parameter to the constructor of m_tree.
//        // Therefore, m_code, must be declared before m_tree.
//        FunctionBuffer& m_code;
//
//        // TODO: Find a better solution to this problem.
//        // WARNING: m_tree is a parameter to the constructor of m_factory.
//        // Therefore, m_tree, must be declared before m_factory.
//        ExpressionTree m_tree;
//
//    protected:
//        ExpressionNodeFactory m_factory;
//    };
//
//
//    template <typename R, typename P1 = void, typename P2 = void>
//    class Function : public FunctionBase
//    {
//    public:
//        Function(Allocators::IAllocator& allocator, FunctionBuffer& code);
//
//        ParameterNode<P1>& GetP1() const;
//        ParameterNode<P2>& GetP2() const;
//
//        void Return(Node<R>& value);
//
//        typedef R (*FunctionType)(P1, P2);
//
//        FunctionType GetEntryPoint() const;
//
//    private:
//        ParameterNode<P1>* m_p1;
//        ParameterNode<P2>* m_p2;
//        NodeBase* m_return;
//    };
//
//
//    template <typename R, typename P1, typename P2>
//    Function<R, P1, P2>::Function(Allocators::IAllocator& allocator, FunctionBuffer& code)
//        : FunctionBase(allocator, code)
//    {
//        m_p1 = &m_factory.Parameter<P1>();
//        m_p2 = &m_factory.Parameter<P2>();
//    }
//
//
//    template <typename R, typename P1, typename P2>
//    ParameterNode<P1>& Function<R, P1, P2>::GetP1() const
//    {
//        return *m_p1;
//    }
//
//
//    template <typename R, typename P1, typename P2>
//    ParameterNode<P2>& Function<R, P1, P2>::GetP2() const
//    {
//        return *m_p2;
//    }
//
//
//    template <typename R, typename P1, typename P2>
//    void Function<R, P1, P2>::Return(Node<R>& value)
//    {
//        m_return = &m_factory.Return<R>(value);
//    }
//
//
//    template <typename R, typename P1, typename P2>
//    typename Function<R, P1, P2>::FunctionType Function<R, P1, P2>::GetEntryPoint() const
//    {
//        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
//    }
//
//
//    template <typename R, typename P1>
//    class Function<R, P1, void>
//    {
//    public:
//        Function();
//
//        ParameterNode<P1>& GetP1() const;
//
//        void Return(Node<R>& value);
//
//    private:
//        ExpressionNodeFactory& m_factory;
//
//        ParameterNode<P1>* m_p1;
//        NodeBase* m_return;
//    };
//
//
//    template <typename R, typename P1>
//    class FunctionNode1
//    {
//    public:
//        FunctionNode1(ExpressionNodeFactory& factory);
//
//        ParameterNode<P1>& GetP1() const;
//
//        void Return(Node<R>& value);
//
//    private:
//        ExpressionNodeFactory& m_factory;
//
//        ParameterNode<P1>* m_p1;
//        NodeBase* m_return;             // TODO: Why is this NodeBase?
//    };
//
//
//    template <typename R, typename P1, typename P2>
//    class FunctionNode2
//    {
//    public:
//        FunctionNode2(ExpressionNodeFactory& factory);
//
//        ParameterNode<P1>& GetP1() const;
//
//        ParameterNode<P2>& GetP2() const;
//
//        void Return(Node<R>& value);
//
//    private:
//        ExpressionNodeFactory& m_factory;
//
//        ParameterNode<P1>* m_p1;
//        ParameterNode<P2>* m_p2;
//        NodeBase* m_return;
//    };
//
//
//    //*************************************************************************
//    //
//    // Template definitions for Function1.
//    //
//    //*************************************************************************
//    template <typename R, typename P1>
//    FunctionNode1<R, P1>::FunctionNode1(ExpressionNodeFactory& factory)
//        : m_factory(factory),
//          m_return(nullptr)
//    {
//        m_p1 = &factory.Parameter<P1>();
//    }
//
//
//    template <typename R, typename P1>
//    ParameterNode<P1>& FunctionNode1<R, P1>::GetP1() const
//    {
//        return *m_p1;
//    }
//
//
//    template <typename R, typename P1>
//    void FunctionNode1<R, P1>::Return(Node<R>& value)
//    {
//        m_return = &m_factory.Return<R>(value);
//    }
//
//
//    //*************************************************************************
//    //
//    // Template definitions for Function2.
//    //
//    //*************************************************************************
//    template <typename R, typename P1, typename P2>
//    FunctionNode2<R, P1, P2>::FunctionNode2(ExpressionNodeFactory& factory)
//        : m_factory(factory),
//          m_return(nullptr)
//    {
//        m_p1 = &factory.Parameter<P1>();
//        m_p2 = &factory.Parameter<P2>();
//    }
//
//
//    template <typename R, typename P1, typename P2>
//    ParameterNode<P1>& FunctionNode2<R, P1, P2>::GetP1() const
//    {
//        return *m_p1;
//    }
//
//
//    template <typename R, typename P1, typename P2>
//    ParameterNode<P2>& FunctionNode2<R, P1, P2>::GetP2() const
//    {
//        return *m_p2;
//    }
//
//
//    template <typename R, typename P1, typename P2>
//    void FunctionNode2<R, P1, P2>::Return(Node<R>& value)
//    {
//        m_return = &m_factory.Return<R>(value);
//    }
//}
