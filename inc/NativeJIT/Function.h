#pragma once

#include "NativeJIT/ExecutionPreconditionTest.h"
#include "NativeJIT/ExpressionNodeFactory.h"
#include "NativeJIT/TypePredicates.h"


namespace NativeJIT
{
    template <typename R>
    class FunctionBase : public ExpressionNodeFactory
    {
    public:
        FunctionBase(Allocators::IAllocator& allocator, FunctionBuffer& code);

        template <JccType JCC>
        void AddExecuteOnlyIfStatement(FlagExpressionNode<JCC>& condition,
                                       ImmediateNode<R>& otherwiseValue);

    private:
        Allocators::IAllocator& m_allocator;
    };


    template <typename R, typename P1 = void, typename P2 = void, typename P3 = void, typename P4 = void>
    class Function : public FunctionBase<R>
    {
    public:
        Function(Allocators::IAllocator& allocator, FunctionBuffer& code);

        ParameterNode<P1>& GetP1() const;
        ParameterNode<P2>& GetP2() const;
        ParameterNode<P3>& GetP3() const;
        ParameterNode<P4>& GetP4() const;

        typedef R (*FunctionType)(P1, P2, P3, P4);

        FunctionType Compile(Node<R>& expression);

        FunctionType GetEntryPoint() const;

    private:
        ParameterNode<P1>* m_p1;
        ParameterNode<P2>* m_p2;
        ParameterNode<P3>* m_p3;
        ParameterNode<P4>* m_p4;
    };


    template <typename R, typename P1, typename P2, typename P3>
    class Function<R, P1, P2, P3> : public FunctionBase<R>
    {
    public:
        Function(Allocators::IAllocator& allocator, FunctionBuffer& code);

        ParameterNode<P1>& GetP1() const;
        ParameterNode<P2>& GetP2() const;
        ParameterNode<P3>& GetP3() const;

        typedef R (*FunctionType)(P1, P2, P3);

        FunctionType Compile(Node<R>& expression);

        FunctionType GetEntryPoint() const;

    private:
        ParameterNode<P1>* m_p1;
        ParameterNode<P2>* m_p2;
        ParameterNode<P3>* m_p3;
    };


    template <typename R, typename P1, typename P2>
    class Function<R, P1, P2> : public FunctionBase<R>
    {
    public:
        Function(Allocators::IAllocator& allocator, FunctionBuffer& code);

        ParameterNode<P1>& GetP1() const;
        ParameterNode<P2>& GetP2() const;

        typedef R (*FunctionType)(P1, P2);

        FunctionType Compile(Node<R>& expression);

        FunctionType GetEntryPoint() const;

    private:
        ParameterNode<P1>* m_p1;
        ParameterNode<P2>* m_p2;
    };


    template <typename R, typename P1>
    class Function<R, P1> : public FunctionBase<R>
    {
    public:
        Function(Allocators::IAllocator& allocator, FunctionBuffer& code);

        ParameterNode<P1>& GetP1() const;

        typedef R (*FunctionType)(P1);

        FunctionType Compile(Node<R>& expression);

        FunctionType GetEntryPoint() const;

    private:
        ParameterNode<P1>* m_p1;
    };


    template <typename R>
    class Function<R> : public FunctionBase<R>
    {
    public:
        Function(Allocators::IAllocator& allocator, FunctionBuffer& code);

        typedef R (*FunctionType)();

        FunctionType Compile(Node<R>& expression);

        FunctionType GetEntryPoint() const;
    };


    // ParameterSlot allocates parameter index numbers which are used to map
    // parameters to registers. In the Windows ABI, a parameter's index numbers
    // is equal to its position in the parameter list, regardless of parameter
    // type. For example, in the function,
    //    void f(int a, float b, int c, float d)
    // the parameter indexes would be
    //    a:0, b:1, c:2, d:3
    // In the System V ABI, parameters indexes are allocated from two sequences,
    // one for integer types and one for floating point types. Given the
    // function definition above, on System V, the parameter indexes would be
    //    a:0, b:0, c:1, d:1
    class ParameterSlot
    {
    public:
        ParameterSlot()
          : m_ints(0)
#ifndef NATIVEJIT_PLATFORM_WINDOWS
            , m_floats(0)
#endif
        {
        }
        
        template <class T, typename std::enable_if<std::is_floating_point<T>::value>::type * = nullptr>
        unsigned Allocate()
        {
#ifdef NATIVEJIT_PLATFORM_WINDOWS
            return m_ints++;
#else
            return m_floats++;
#endif
        }
        
        
        template <class T, typename std::enable_if<!std::is_floating_point<T>::value>::type * = nullptr>
        unsigned Allocate()
        {
            return m_ints++;
        }
        
        
    private:
        unsigned m_ints;
#ifndef NATIVEJIT_PLATFORM_WINDOWS
        unsigned m_floats;
#endif
    };
    
    
    //*************************************************************************
    //
    // FunctionBase<R> template definitions.
    //
    //*************************************************************************
    template <typename R>
    FunctionBase<R>::FunctionBase(Allocators::IAllocator& allocator,
                                  FunctionBuffer& code)
        : ExpressionNodeFactory(allocator, code),
          m_allocator(allocator)
    {
        static_assert(IsValidParameter<R>::c_value, "R is an invalid type.");
    }


    template <typename R>
    template <JccType JCC>
    void FunctionBase<R>::AddExecuteOnlyIfStatement(FlagExpressionNode<JCC>& condition,
                                                    ImmediateNode<R>& otherwiseValue)
    {
        auto & test = PlacementConstruct<ExecuteOnlyIfStatement<R, JCC>>(condition, otherwiseValue);

        AddExecutionPreconditionTest(test);
    }


    //*************************************************************************
    //
    // Function<R, P1, P2, P3, P4> template definitions.
    //
    //*************************************************************************
    template <typename R, typename P1, typename P2, typename P3, typename P4>
    Function<R, P1, P2, P3, P4>::Function(Allocators::IAllocator& allocator,
                                          FunctionBuffer& code)
        : FunctionBase<R>(allocator, code)
    {
        static_assert(IsValidParameter<P1>::c_value, "P1 is an invalid type.");
        static_assert(IsValidParameter<P2>::c_value, "P2 is an invalid type.");
        static_assert(IsValidParameter<P3>::c_value, "P3 is an invalid type.");
        static_assert(IsValidParameter<P4>::c_value, "P4 is an invalid type.");

        ParameterSlot slot;
        m_p1 = &this->template Parameter<P1>(slot.Allocate<P1>());
        m_p2 = &this->template Parameter<P2>(slot.Allocate<P2>());
        m_p3 = &this->template Parameter<P3>(slot.Allocate<P3>());
        m_p4 = &this->template Parameter<P4>(slot.Allocate<P4>());
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
    typename Function<R, P1, P2, P3, P4>::FunctionType
    Function<R, P1, P2, P3, P4>::Compile(Node<R>& value)
    {
        this->template Return<R>(value);
        ExpressionTree::Compile();
        return GetEntryPoint();
    }


    template <typename R, typename P1, typename P2, typename P3, typename P4>
    typename Function<R, P1, P2, P3, P4>::FunctionType
    Function<R, P1, P2, P3, P4>::GetEntryPoint() const
    {
        return reinterpret_cast<FunctionType>(const_cast<void*>(this->GetUntypedEntryPoint()));
    }


    //*************************************************************************
    //
    // Function<R, P1, P2, P3> template definitions.
    //
    //*************************************************************************
    template <typename R, typename P1, typename P2, typename P3>
    Function<R, P1, P2, P3>::Function(Allocators::IAllocator& allocator,
                                      FunctionBuffer& code)
        : FunctionBase<R>(allocator, code)
    {
        static_assert(IsValidParameter<P1>::c_value, "P1 is an invalid type.");
        static_assert(IsValidParameter<P2>::c_value, "P2 is an invalid type.");
        static_assert(IsValidParameter<P3>::c_value, "P3 is an invalid type.");

        unsigned position = 0;
        m_p1 = &this->template Parameter<P1>(position++);
        m_p2 = &this->template Parameter<P2>(position++);
        m_p3 = &this->template Parameter<P3>(position++);
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
    typename Function<R, P1, P2, P3>::FunctionType
    Function<R, P1, P2, P3>::Compile(Node<R>& value)
    {
        this->template Return<R>(value);
        ExpressionTree::Compile();
        return GetEntryPoint();
    }


    template <typename R, typename P1, typename P2, typename P3>
    typename Function<R, P1, P2, P3>::FunctionType
    Function<R, P1, P2, P3>::GetEntryPoint() const
    {
        return reinterpret_cast<FunctionType>(const_cast<void*>(this->GetUntypedEntryPoint()));
    }


    //*************************************************************************
    //
    // Function<R, P1, P2> template definitions.
    //
    //*************************************************************************
    template <typename R, typename P1, typename P2>
    Function<R, P1, P2>::Function(Allocators::IAllocator& allocator,
                                  FunctionBuffer& code)
        : FunctionBase<R>(allocator, code)
    {
        static_assert(IsValidParameter<P1>::c_value, "P1 is an invalid type.");
        static_assert(IsValidParameter<P2>::c_value, "P2 is an invalid type.");

        unsigned position = 0;
        m_p1 = &this->template Parameter<P1>(position++);
        m_p2 = &this->template Parameter<P2>(position++);
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
    typename Function<R, P1, P2>::FunctionType
    Function<R, P1, P2>::Compile(Node<R>& value)
    {
        this->template Return<R>(value);
        ExpressionTree::Compile();
        return GetEntryPoint();
    }


    template <typename R, typename P1, typename P2>
    typename Function<R, P1, P2>::FunctionType
    Function<R, P1, P2>::GetEntryPoint() const
    {
        return reinterpret_cast<FunctionType>(const_cast<void*>(this->GetUntypedEntryPoint()));
    }


    //*************************************************************************
    //
    // Function<R, P1> template definitions.
    //
    //*************************************************************************
    template <typename R, typename P1>
    Function<R, P1>::Function(Allocators::IAllocator& allocator,
                              FunctionBuffer& code)
        : FunctionBase<R>(allocator, code)
    {
        static_assert(IsValidParameter<P1>::c_value, "P1 is an invalid type.");

        unsigned position = 0;
        m_p1 = &this->template Parameter<P1>(position++);
    }


    template <typename R, typename P1>
    ParameterNode<P1>& Function<R, P1>::GetP1() const
    {
        return *m_p1;
    }


    template <typename R, typename P1>
    typename Function<R, P1>::FunctionType
    Function<R, P1>::Compile(Node<R>& value)
    {
        this->template Return<R>(value);
        ExpressionTree::Compile();
        return GetEntryPoint();
    }


    template <typename R, typename P1>
    typename Function<R, P1>::FunctionType
    Function<R, P1>::GetEntryPoint() const
    {
        return reinterpret_cast<FunctionType>(const_cast<void*>(this->GetUntypedEntryPoint()));
    }


    //*************************************************************************
    //
    // Function<R> template definitions.
    //
    //*************************************************************************
    template <typename R>
    Function<R>::Function(Allocators::IAllocator& allocator,
                          FunctionBuffer& code)
        : FunctionBase<R>(allocator, code)
    {
    }


    template <typename R>
    typename Function<R>::FunctionType  Function<R>::Compile(Node<R>& value)
    {
        this->template Return<R>(value);
        ExpressionTree::Compile();
        return GetEntryPoint();
    }


    template <typename R>
    typename Function<R>::FunctionType Function<R>::GetEntryPoint() const
    {
        return reinterpret_cast<FunctionType>(const_cast<void*>(this->GetUntypedEntryPoint()));
    }
}
