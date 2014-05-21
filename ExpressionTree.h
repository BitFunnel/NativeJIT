#pragma once

#include <vector>

#include "Register.h"


namespace NativeJIT
{
    class NodeBase;
    class ParameterBase;
    class X64CodeGenerator;


    class RegisterFile
    {
    public:
        RegisterFile(unsigned rxxCount, unsigned xmmCount);

        template <unsigned SIZE>
        void ReserveRegister(Register<SIZE, false> r);

        template <unsigned SIZE>
        void ReserveRegister(Register<SIZE, true> r);

        unsigned GetRXXRegisterCount() const;
        unsigned GetXMMRegisterCount() const;

        unsigned GetReservedRXX() const;
        unsigned GetReservedXMM() const;

    private:
        unsigned m_rxxCount;
        unsigned m_xmmCount;
        unsigned m_reservedRXX;
        unsigned m_reservedXMM;
    };


    class ExpressionTree
    {
    public:
        ExpressionTree(X64CodeGenerator& code);

        unsigned AddNode(NodeBase& node);
        unsigned AddParameter(ParameterBase& parameter);

        void AllocateRegister(Register<1, false>& r);
        void AllocateRegister(Register<2, false>& r);
        void AllocateRegister(Register<4, false>& r);
        void AllocateRegister(Register<8, false>& r);

        void AllocateRegister(Register<4, true>& r);
        void AllocateRegister(Register<8, true>& r);

        template <unsigned SIZE, bool ISFLOAT>
        Register<SIZE, ISFLOAT> CopyRegister(Register<SIZE, ISFLOAT> r);

        RegisterFile& GetParameterRegisters();

        X64CodeGenerator& GetCodeGenerator() const;

        void Print() const;
        void Compile();

    private:
        void Prologue();

        // Assigns registers to parameters.
        void Pass1();

        // Generates code that evaluates common subexpressions into registers.
        void Pass2();

        // Generates code to evaluate the entire tree.
        void Pass3();

        void Epilogue();


        X64CodeGenerator& m_code;

        std::vector<NodeBase*> m_topologicalSort;

        std::vector<ParameterBase*> m_parameters;
        RegisterFile m_parameterRegisters;

        std::vector<unsigned> m_rxxRegisters;
    };


    //*************************************************************************
    //
    // Template definitions for RegisterFile.
    //
    //*************************************************************************
    template <unsigned SIZE>
    void RegisterFile::ReserveRegister(Register<SIZE, false> r)
    {
        m_reservedRXX &= ~(1 << r.GetId());
    }


    template <unsigned SIZE>
    void RegisterFile::ReserveRegister(Register<SIZE, true> r)
    {
        m_reservedXMM &= ~(1 << r.GetId());
    }


    //*************************************************************************
    //
    // Template definitions for ExpressionTree.
    //
    //*************************************************************************
    template <unsigned SIZE, bool ISFLOAT>
    Register<SIZE, ISFLOAT> ExpressionTree::CopyRegister(Register<SIZE, ISFLOAT> src)
    {
        Register<SIZE, ISFLOAT> dest;
        AllocateRegister(dest);
        m_code.Op("mov", dest, src);
        return dest;
    }
}
