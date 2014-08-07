#pragma once

#include "ExpressionTree.h"
#include "Node.h"
#include "Temporary/Assert.h"


namespace NativeJIT
{
    class ParameterBase
    {
    public:
        ParameterBase(ExpressionTree& tree);

        unsigned GetPosition() const;
        void PrintParameter() const;

        virtual void ReserveRegister(ExpressionTree& tree, RegisterFile& registers) = 0;

    private:
        unsigned m_position;
    };


    template <typename T>
    class ParameterNode : public Node<T>, ParameterBase
    {
    public:
        ParameterNode(ExpressionTree& tree);

        //
        // Overrides of Node methods.
        //
        virtual Storage<T> CodeGenValue(ExpressionTree& tree) override;

        virtual void Print() const override;


        //
        // Overrides of ParameterBase methods.
        //
        virtual void ReserveRegister(ExpressionTree& tree, RegisterFile& registers) override;
    };


    //*************************************************************************
    //
    // Template definitions for ParameterNode
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
        // TODO: Use constants to encode registers.
        static unsigned idMap[] = {1, 2, 8, 9};

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
    ParameterNode<T>::ParameterNode(ExpressionTree& tree)
        : Node(tree),
          ParameterBase(tree)
    {
    }


    template <typename T>
    typename Storage<T> ParameterNode<T>::CodeGenValue(ExpressionTree& /*tree*/)
    {
        Assert(IsCached(), "Must assign register before attempting code generation.");

        auto result = GetCache();
        ReleaseCache();
        return result;
    }


    template <typename T>
    void ParameterNode<T>::Print() const
    {
        std::cout << "ParameterNode id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", position = " << GetPosition();
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }


    template <typename T>
    void ParameterNode<T>::ReserveRegister(ExpressionTree& tree, RegisterFile& registers)
    {
        RegisterType r;
        GetParameterRegister(GetPosition(), r);
        registers.ReserveRegister(r);
        SetCache(Storage<T>(tree, r));
    }
}
