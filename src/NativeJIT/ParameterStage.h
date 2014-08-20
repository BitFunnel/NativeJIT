#pragma once

#include <vector>

#include "CodeGenHelpers.h"             // Function called by template definition.
#include "ExpressionTree.h"             // ExpressionTree::Storage<T> embedded.
#include "Temporary/NonCopyable.h"      // Base class.


namespace NativeJIT
{
    template <typename T> class Node;
    template <typename T> class Storage;


    class ParameterStage : public NonCopyable
    {
    public:
        ParameterStage(FunctionBuffer& code);

        template <typename T>
        void AddParameter(Node<T>& expression);

        template <typename R, typename F>
        ExpressionTree::Storage<R> EmitCall(ExpressionTree& tree, Node<F>& function);

    private:

        class ParameterBase
        {
        public:
            ParameterBase(unsigned position);

            virtual void EmitStaging(ExpressionTree& tree) = 0;
            virtual void Release() = 0;

        protected:
            unsigned m_position;
        };


        template <typename T>
        class Parameter : public ParameterBase
        {
        public:
            Parameter(Node<T>& expression, unsigned position);

            virtual void EmitStaging(ExpressionTree& tree) override;
            virtual void Release() override;

            typename ExpressionTree::Storage<T>::DirectRegister GetRegister() const;

        private:
            Node<T>& m_expression;
            ExpressionTree::Storage<T> m_storage;
            typename ExpressionTree::Storage<T>::DirectRegister m_destination;
        };

        void SaveVolatiles(ExpressionTree& tree);
        void RestoreVolatiles(ExpressionTree& tree);

        template <unsigned SIZE>
        void RecordParameterRegister(Register<SIZE, false> r);


        FunctionBuffer& m_code;
        std::vector<ParameterBase*> m_parameters;

        // TODO: m_xmmParameters
        unsigned m_rxxParameters;

        // TODO: Figure out how to initialize with something like RAX | RCX | RDX | R8 | R9| R10| R11
        static const unsigned c_rxxVolatiles = 0xf07; //1111 0000 0111
    };


    //*************************************************************************
    //
    // Template definitions for ParameterStage
    //
    //*************************************************************************
    template <typename T>
    void ParameterStage::AddParameter(Node<T>& expression)
    {
        // TODO: Use arena allocator.
        Parameter<T>* parameter = new Parameter<T>(expression, static_cast<unsigned>(m_parameters.size()));
        m_parameters.push_back(parameter);
        RecordParameterRegister(parameter->GetRegister());
    }


    template <typename R, typename F>
    ExpressionTree::Storage<R> ParameterStage::EmitCall(ExpressionTree& tree, Node<F>& function)
    {
        // Stage each of the parameters.
        for (unsigned i = 0; i < m_parameters.size(); ++i)
        {
            m_parameters[i]->EmitStaging(tree);
        }

        ExpressionTree::Storage<R>::DirectRegister resultRegister;
        ExpressionTree::Storage<R> result = tree.Direct<R>(resultRegister);

        // Save volatile registers that are in use.
        std::cout << "Save volatile registers that are in use." << std::endl;
        // The registers RAX, RCX, RDX, R8, R9, R10, R11 are considered volatile and must be considered destroyed on function calls
        // XMM6:XMM15, YMM6:YMM15 must be preserved as needed by callee.

        // Emit the call.
        auto f = function.CodeGen(tree);
        f.ConvertToValue(tree, false);

        // m_code.SaveVolatileRegisters();
        SaveVolatiles(tree);

        // TODO: Support for indirect call.

        m_code.Emit<OpCode::Call>(f.GetDirectRegister());

        // Release parameter storage.
        f.Reset();
        for (unsigned i = 0; i < m_parameters.size(); ++i)
        {
            m_parameters[i]->Release();
        }

        // Restore volatile registers.
        std::cout << "Restore volatile registers." << std::endl;
        RestoreVolatiles(tree);

        return result;
    }


    template <unsigned SIZE>
    void ParameterStage::RecordParameterRegister(Register<SIZE, false> r)
    {
        m_rxxParameters |= (1ul << r.GetId());
    }


    //*************************************************************************
    //
    // Template definitions for ParameterStage::Parameter<T>
    //
    //*************************************************************************

    template <typename T>
    ParameterStage::Parameter<T>::Parameter(Node<T>& expression, unsigned position)
        : ParameterBase(position),
          m_expression(expression)
    {
        GetParameterRegister(m_position, m_destination);
    }


    template <typename T>
    void ParameterStage::Parameter<T>::EmitStaging(ExpressionTree& tree)
    {
        m_storage = m_expression.CodeGen(tree);

        if (m_storage.GetStorageClass() != StorageClass::Direct || m_storage.GetDirectRegister().GetId() != m_destination.GetId())
        {
            // Need to move the expression into the correct register.
            // NOTE: destination will be released in Parameter destructor.
            ExpressionTree::Storage<T> destination = tree.Direct<T>(m_destination);
            CodeGenHelpers::Emit<OpCode::Mov>(tree.GetCodeGenerator(), m_destination, m_storage);
            m_storage = destination;
        }
    }


    template <typename T>
    void ParameterStage::Parameter<T>::Release()
    {
        m_storage.Reset();
    }


    template <typename T>
    typename ExpressionTree::Storage<T>::DirectRegister ParameterStage::Parameter<T>::GetRegister() const
    {
        return m_destination;
    }
}
