#pragma once

#include "Assert.h"
#include "ExpressionTree.h"
#include "Node.h"


namespace NativeJIT
{
    class ParameterBase
    {
    public:
        ParameterBase(ExpressionTree& tree);

        unsigned GetPosition() const;
        void PrintParameter() const;

        virtual void ReserveRegister(RegisterFile& registers) = 0;

    private:
        unsigned m_position;
    };


    template <typename T>
    class Parameter : public DirectValue<T>, ParameterBase
    {
    public:
        Parameter(ExpressionTree& tree);

        //
        // Overrides of Node methods.
        //
        virtual RegisterType CodeGenValue(ExpressionTree& tree) override;
        virtual void Print() const override;


        //
        // Overrides of ParameterBase methods.
        //
        virtual void ReserveRegister(RegisterFile& registers) override;
    };


    //*************************************************************************
    //
    // Template definitions for Parameter
    //
    //*************************************************************************
    template <unsigned SIZE, bool ISFLOAT>
    void GetParameterRegister(unsigned id, Register<SIZE, ISFLOAT>& r);


    template <unsigned SIZE>
    void GetParameterRegister(unsigned id, Register<SIZE, false>& r)
    {
        // For now we only support the four parameters that are passed in registers.
        // No support for memory parameters.
        Assert(id < 4, "Exceeded maximum number of register parameters.");

        // Integer parameters are passed in RCX, RDX, R8, and R9.
        static unsigned idMap[] = {2, 3, 8, 9};

        r = Register<SIZE, false>(idMap[id]);
    }


    template <unsigned SIZE>
    void GetParameterRegister(unsigned id, Register<SIZE, true>& r)
    {
        // For now we only support the four parameters that are passed in registers.
        // No support for memory parameters.
        Assert(id < 4, "Exceeded maximum number of register parameters.");

        // Floating point parameters are passed in XMM0-XMM3.
        r = Register<SIZE, true>(id);
    }


    template <typename T>
    Parameter<T>::Parameter(ExpressionTree& tree)
        : DirectValue(tree),
          ParameterBase(tree)
    {
    }


    template <typename T>
    typename Node<T>::RegisterType Parameter<T>::CodeGenValue(ExpressionTree& /*tree*/)
    {
        Assert(IsCached(), "Must assign register before attempting code generation.");

        RegisterType r = GetCacheRegister();
        ReleaseCache();

        return r;
    }


    template <typename T>
    void Parameter<T>::Print() const
    {
        std::cout << "Parameter id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", position = " << GetPosition();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }


    template <typename T>
    void Parameter<T>::ReserveRegister(RegisterFile& registers)
    {
        RegisterType r;
        GetParameterRegister(GetPosition(), r);
        registers.ReserveRegister(r);
        SetCacheRegister(r);
    }
}
