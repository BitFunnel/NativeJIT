#pragma once

#include "ExpressionNodeFactory.h"


namespace NativeJIT
{
    template <typename R, typename P1 = void, typename P2 = void, typename P3 = void, typename P4 = void>
    class Function : public ExpressionNodeFactory
    {
    public:
        Function(Allocators::IAllocator& allocator, FunctionBuffer& code);

        ParameterNode<P1>& GetP1() const;
        ParameterNode<P2>& GetP2() const;
        ParameterNode<P3>& GetP3() const;
        ParameterNode<P4>& GetP4() const;

        void Return(Node<R>& value);

        typedef R (*FunctionType)(P1, P2, P3, P4);

        FunctionType Compile(Node<R>& expression);

        FunctionType GetEntryPoint() const;

    private:
        ParameterNode<P1>* m_p1;
        ParameterNode<P2>* m_p2;
        ParameterNode<P3>* m_p3;
        ParameterNode<P4>* m_p4;

        NodeBase* m_return;         // TODO: does this value really need to be stored?
    };


    template <typename R, typename P1, typename P2, typename P3>
    class Function<R, P1, P2, P3> : public ExpressionNodeFactory
    {
    public:
        Function(Allocators::IAllocator& allocator, FunctionBuffer& code);

        ParameterNode<P1>& GetP1() const;
        ParameterNode<P2>& GetP2() const;
        ParameterNode<P3>& GetP3() const;

        void Return(Node<R>& value);

        typedef R (*FunctionType)(P1, P2, P3);

        FunctionType Compile(Node<R>& expression);

        FunctionType GetEntryPoint() const;

    private:
        ParameterNode<P1>* m_p1;
        ParameterNode<P2>* m_p2;
        ParameterNode<P3>* m_p3;

        NodeBase* m_return;         // TODO: does this value really need to be stored?
    };


    template <typename R, typename P1, typename P2>
    class Function<R, P1, P2> : public ExpressionNodeFactory
    {
    public:
        Function(Allocators::IAllocator& allocator, FunctionBuffer& code);

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
    class Function<R, P1> : public ExpressionNodeFactory
    {
    public:
        Function(Allocators::IAllocator& allocator, FunctionBuffer& code);

        ParameterNode<P1>& GetP1() const;

        void Return(Node<R>& value);

        typedef R (*FunctionType)(P1);

        FunctionType Compile(Node<R>& expression);

        FunctionType GetEntryPoint() const;

    private:
        ParameterNode<P1>* m_p1;

        NodeBase* m_return;         // TODO: does this value really need to be stored?
    };


    template <typename R>
    class Function<R> : public ExpressionNodeFactory
    {
    public:
        Function(Allocators::IAllocator& allocator, FunctionBuffer& code);

        void Return(Node<R>& value);

        typedef R (*FunctionType)();

        FunctionType Compile(Node<R>& expression);

        FunctionType GetEntryPoint() const;

    private:
        NodeBase* m_return;         // TODO: does this value really need to be stored?
    };


    //*************************************************************************
    //
    // Function<R, P1, P2, P3, P4> template definitions.
    //
    //*************************************************************************
    template <typename R, typename P1, typename P2, typename P3, typename P4>
    Function<R, P1, P2, P3, P4>::Function(Allocators::IAllocator& allocator,
                                          FunctionBuffer& code)
        : ExpressionNodeFactory(allocator, code)
    {
        static_assert(std::is_pod<R>::value, "R must be a POD type.");
        static_assert(std::is_pod<P1>::value, "P1 must be a POD type.");
        static_assert(std::is_pod<P2>::value, "P2 must be a POD type.");
        static_assert(std::is_pod<P3>::value, "P3 must be a POD type.");
        static_assert(std::is_pod<P4>::value, "P4 must be a POD type.");

        m_p1 = &Parameter<P1>();
        m_p2 = &Parameter<P2>();
        m_p3 = &Parameter<P3>();
        m_p4 = &Parameter<P4>();
    }


    template <typename R, typename P1, typename P2, typename P3, typename P4>
    ParameterNode<P1>& Function<R, P1, P2, P3, P4>::GetP1() const
    {
        return *m_p1;
    }


    template <typename R, typename P1, typename P2, typename P3, typename P4>
    ParameterNode<P2>& Function<R, P1, P2, P3, P4>::GetP2() const
    {
        return *m_p2;
    }


    template <typename R, typename P1, typename P2, typename P3, typename P4>
    ParameterNode<P3>& Function<R, P1, P2, P3, P4>::GetP3() const
    {
        return *m_p3;
    }


    template <typename R, typename P1, typename P2, typename P3, typename P4>
    ParameterNode<P4>& Function<R, P1, P2, P3, P4>::GetP4() const
    {
        return *m_p4;
    }


    template <typename R, typename P1, typename P2, typename P3, typename P4>
    void Function<R, P1, P2, P3, P4>::Return(Node<R>& value)
    {
        // TODO: Fix name aliasing between Function and is base class.
        m_return = &ExpressionNodeFactory::Return<R>(value);
    }


    template <typename R, typename P1, typename P2, typename P3, typename P4>
    typename Function<R, P1, P2, P3, P4>::FunctionType
    Function<R, P1, P2, P3, P4>::Compile(Node<R>& value)
    {
        m_return = &ExpressionNodeFactory::Return<R>(value);
        ExpressionTree::Compile();
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }


    template <typename R, typename P1, typename P2, typename P3, typename P4>
    typename Function<R, P1, P2, P3, P4>::FunctionType
    Function<R, P1, P2, P3, P4>::GetEntryPoint() const
    {
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }


    //*************************************************************************
    //
    // Function<R, P1, P2, P3> template definitions.
    //
    //*************************************************************************
    template <typename R, typename P1, typename P2, typename P3>
    Function<R, P1, P2, P3>::Function(Allocators::IAllocator& allocator,
                                      FunctionBuffer& code)
        : ExpressionNodeFactory(allocator, code)
    {
        static_assert(std::is_pod<R>::value, "R must be a POD type.");
        static_assert(std::is_pod<P1>::value, "P1 must be a POD type.");
        static_assert(std::is_pod<P2>::value, "P2 must be a POD type.");
        static_assert(std::is_pod<P3>::value, "P3 must be a POD type.");

        m_p1 = &Parameter<P1>();
        m_p2 = &Parameter<P2>();
        m_p3 = &Parameter<P3>();
    }


    template <typename R, typename P1, typename P2, typename P3>
    ParameterNode<P1>& Function<R, P1, P2, P3>::GetP1() const
    {
        return *m_p1;
    }


    template <typename R, typename P1, typename P2, typename P3>
    ParameterNode<P2>& Function<R, P1, P2, P3>::GetP2() const
    {
        return *m_p2;
    }


    template <typename R, typename P1, typename P2, typename P3>
    ParameterNode<P3>& Function<R, P1, P2, P3>::GetP3() const
    {
        return *m_p3;
    }


    template <typename R, typename P1, typename P2, typename P3>
    void Function<R, P1, P2, P3>::Return(Node<R>& value)
    {
        // TODO: Fix name aliasing between Function and is base class.
        m_return = &ExpressionNodeFactory::Return<R>(value);
    }


    template <typename R, typename P1, typename P2, typename P3>
    typename Function<R, P1, P2, P3>::FunctionType
    Function<R, P1, P2, P3>::Compile(Node<R>& value)
    {
        m_return = &ExpressionNodeFactory::Return<R>(value);
        ExpressionTree::Compile();
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }


    template <typename R, typename P1, typename P2, typename P3>
    typename Function<R, P1, P2, P3>::FunctionType
    Function<R, P1, P2, P3>::GetEntryPoint() const
    {
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }


    //*************************************************************************
    //
    // Function<R, P1, P2> template definitions.
    //
    //*************************************************************************
    template <typename R, typename P1, typename P2>
    Function<R, P1, P2>::Function(Allocators::IAllocator& allocator,
                                  FunctionBuffer& code)
        : ExpressionNodeFactory(allocator, code)
    {
        static_assert(std::is_pod<R>::value, "R must be a POD type.");
        static_assert(std::is_pod<P1>::value, "P1 must be a POD type.");
        static_assert(std::is_pod<P2>::value, "P2 must be a POD type.");

        m_p1 = &Parameter<P1>();
        m_p2 = &Parameter<P2>();
    }


    template <typename R, typename P1, typename P2>
    ParameterNode<P1>& Function<R, P1, P2>::GetP1() const
    {
        return *m_p1;
    }


    template <typename R, typename P1, typename P2>
    ParameterNode<P2>& Function<R, P1, P2>::GetP2() const
    {
        return *m_p2;
    }


    template <typename R, typename P1, typename P2>
    void Function<R, P1, P2>::Return(Node<R>& value)
    {
        // TODO: Fix name aliasing between Function and is base class.
        m_return = &ExpressionNodeFactory::Return<R>(value);
    }


    template <typename R, typename P1, typename P2>
    typename Function<R, P1, P2>::FunctionType
    Function<R, P1, P2>::Compile(Node<R>& value)
    {
        m_return = &ExpressionNodeFactory::Return<R>(value);
        ExpressionTree::Compile();
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }


    template <typename R, typename P1, typename P2>
    typename Function<R, P1, P2>::FunctionType
    Function<R, P1, P2>::GetEntryPoint() const
    {
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }


    //*************************************************************************
    //
    // Function<R, P1> template definitions.
    //
    //*************************************************************************
    template <typename R, typename P1>
    Function<R, P1>::Function(Allocators::IAllocator& allocator,
                              FunctionBuffer& code)
        : ExpressionNodeFactory(allocator, code)
    {
        static_assert(std::is_pod<R>::value, "R must be a POD type.");
        static_assert(std::is_pod<P1>::value, "P1 must be a POD type.");

        m_p1 = &Parameter<P1>();
    }


    template <typename R, typename P1>
    ParameterNode<P1>& Function<R, P1>::GetP1() const
    {
        return *m_p1;
    }


    template <typename R, typename P1>
    void Function<R, P1>::Return(Node<R>& value)
    {
        // TODO: Fix name aliasing between Function and is base class.
        m_return = &ExpressionNodeFactory::Return<R>(value);
    }


    template <typename R, typename P1>
    typename Function<R, P1>::FunctionType
    Function<R, P1>::Compile(Node<R>& value)
    {
        m_return = &ExpressionNodeFactory::Return<R>(value);
        ExpressionTree::Compile();
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }


    template <typename R, typename P1>
    typename Function<R, P1>::FunctionType
    Function<R, P1>::GetEntryPoint() const
    {
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }


    //*************************************************************************
    //
    // Function<R> template definitions.
    //
    //*************************************************************************
    template <typename R>
    Function<R>::Function(Allocators::IAllocator& allocator,
                          FunctionBuffer& code)
        : ExpressionNodeFactory(allocator, code)
    {
        static_assert(std::is_pod<R>::value, "R must be a POD type.");
    }


    template <typename R>
    void Function<R>::Return(Node<R>& value)
    {
        // TODO: Fix name aliasing between Function and is base class.
        m_return = &ExpressionNodeFactory::Return<R>(value);
    }


    template <typename R>
    typename Function<R>::FunctionType  Function<R>::Compile(Node<R>& value)
    {
        m_return = &ExpressionNodeFactory::Return<R>(value);
        ExpressionTree::Compile();
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }


    template <typename R>
    typename Function<R>::FunctionType Function<R>::GetEntryPoint() const
    {
        return reinterpret_cast<FunctionType>(GetUntypedEntryPoint());
    }
}
