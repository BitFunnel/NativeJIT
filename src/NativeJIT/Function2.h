#pragma once

#include "ExpressionNodeFactory2.h"


namespace NativeJIT
{
    template <typename R, typename P1 = void, typename P2 = void>
    class Function2 : public ExpressionNodeFactory2
    {
    public:
        Function2(Allocators::IAllocator& allocator, FunctionBuffer& code);

        ParameterNode<P1>& GetP1() const;
        ParameterNode<P2>& GetP2() const;

        void Return(Node<R>& value);

        typedef R (*FunctionType)(P1, P2);

        FunctionType Compile(Node<R>& expression);

        FunctionType GetEntryPoint() const;

    private:
        ParameterNode<P1>* m_p1;
        ParameterNode<P2>* m_p2;
        NodeBase* m_return;         // TODO: does this value really need to be stored?
    };


    template <typename R, typename P1>
    class Function2<R, P1, void> : public ExpressionNodeFactory2
    {
    public:
        Function2(Allocators::IAllocator& allocator, FunctionBuffer& code);

        ParameterNode<P1>& GetP1() const;

        void Return(Node<R>& value);

        typedef R (*FunctionType)(P1);

        FunctionType Compile(Node<R>& expression);

        FunctionType GetEntryPoint() const;

    private:
        ParameterNode<P1>* m_p1;
        NodeBase* m_return;
    };


    template <typename R>
    class Function2<R, void, void> : public ExpressionNodeFactory2
    {
    public:
        Function2(Allocators::IAllocator& allocator, FunctionBuffer& code);

        void Return(Node<R>& value);

        typedef R (*FunctionType)();

        FunctionType Compile(Node<R>& expression);

        FunctionType GetEntryPoint() const;

    private:
        NodeBase* m_return;
    };


    //*************************************************************************
    //
    // Function2<R, P1, P2> template definitions.
    //
    //*************************************************************************
    template <typename R, typename P1, typename P2>
    Function2<R, P1, P2>::Function2(Allocators::IAllocator& allocator,
                                    FunctionBuffer& code)
        : ExpressionNodeFactory2(allocator, code)
    {
        m_p1 = &Parameter<P1>();
        m_p2 = &Parameter<P2>();
    }


    template <typename R, typename P1, typename P2>
    ParameterNode<P1>& Function2<R, P1, P2>::GetP1() const
    {
        return *m_p1;
    }


    template <typename R, typename P1, typename P2>
    ParameterNode<P2>& Function2<R, P1, P2>::GetP2() const
    {
        return *m_p2;
    }


    template <typename R, typename P1, typename P2>
    void Function2<R, P1, P2>::Return(Node<R>& value)
    {
        // TODO: Fix name aliasing between Function2 and is base class.
        m_return = &ExpressionNodeFactory2::Return<R>(value);
    }


    template <typename R, typename P1, typename P2>
    typename Function2<R, P1, P2>::FunctionType  Function2<R, P1, P2>::Compile(Node<R>& value)
    {
        m_return = &ExpressionNodeFactory2::Return<R>(value);
        ExpressionTree::Compile();
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }


    template <typename R, typename P1, typename P2>
    typename Function2<R, P1, P2>::FunctionType Function2<R, P1, P2>::GetEntryPoint() const
    {
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }


    //*************************************************************************
    //
    // Function2<R, P1> template definitions.
    //
    //*************************************************************************
    template <typename R, typename P1>
    Function2<R, P1>::Function2(Allocators::IAllocator& allocator,
                                FunctionBuffer& code)
        : ExpressionNodeFactory2(allocator, code)
    {
        m_p1 = &Parameter<P1>();
    }


    template <typename R, typename P1>
    ParameterNode<P1>& Function2<R, P1>::GetP1() const
    {
        return *m_p1;
    }


    template <typename R, typename P1>
    void Function2<R, P1>::Return(Node<R>& value)
    {
        // TODO: Fix name aliasing between Function2 and is base class.
        m_return = &ExpressionNodeFactory2::Return<R>(value);
    }


    template <typename R, typename P1>
    typename Function2<R, P1>::FunctionType  Function2<R, P1>::Compile(Node<R>& value)
    {
        m_return = &ExpressionNodeFactory2::Return<R>(value);
        ExpressionTree::Compile();
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }


    template <typename R, typename P1>
    typename Function2<R, P1>::FunctionType Function2<R, P1>::GetEntryPoint() const
    {
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }


    //*************************************************************************
    //
    // Function2<R> template definitions.
    //
    //*************************************************************************
    template <typename R>
    Function2<R>::Function2(Allocators::IAllocator& allocator,
                                      FunctionBuffer& code)
        : ExpressionNodeFactory2(allocator, code)
    {
    }


    template <typename R>
    void Function2<R>::Return(Node<R>& value)
    {
        // TODO: Fix name aliasing between Function2 and is base class.
        m_return = &ExpressionNodeFactory2::Return<R>(value);
    }


    template <typename R>
    typename Function2<R>::FunctionType  Function2<R>::Compile(Node<R>& value)
    {
        m_return = &ExpressionNodeFactory2::Return<R>(value);
        ExpressionTree::Compile();
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }


    template <typename R>
    typename Function2<R>::FunctionType Function2<R>::GetEntryPoint() const
    {
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }
}
