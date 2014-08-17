#include "stdafx.h"


#include "NativeJIT/FunctionBuffer.h"
#include "ParameterStage.h"
#include "Temporary/Assert.h"


namespace NativeJIT
{
    //*************************************************************************
    //
    // ParameterStage
    //
    //*************************************************************************
    ParameterStage::ParameterStage(FunctionBuffer& code)
        : m_code(code)
    {
    }


    void ParameterStage::EmitCall(void* functionPtr)
    {
        // Stage each of the parameters.
        for (unsigned i = 0; i < m_parameters.size(); ++i)
        {
            StageOneParameter();
        }

        // Save volatile registers that are in use.
        m_code.SaveVolatileRegisters();

        // Emit the call.
        m_code.Jmp(functionPtr);

        // Restore volatile registers.
        m_code.RestoreVolatileRegisters();

        // Restore any spilled registers.
        for (unsigned i = 0; i < m_parameters.size(); ++i)
        {
            m_parameters[i]->Restore();
        }
    }


    void ParameterStage::StageOneParameter()
    {
        ParameterBase* parameterToStage = nullptr;

        // Scan through the list examining parameters that haven't yet been staged.
        for (unsigned i = 0; i < m_parameters.size(); ++i)
        {
            if (!m_parameters[i]->IsStaged())
            {
                // Attempt to stage this parameter.
                parameterToStage = m_parameters[i];
                if (parameterToStage->TryEmitStaging(*this, false))
                {
                    return;
                }
            }
        }

        // If no parameter was successfully staged, then force the staging of one
        // of the unstaged parameters, by spilling its destination register.
        if (parameterToStage != nullptr)
        {
            // We found a parameter to stage, but were unable to emit the
            // staging code because the destination register was unavailable.
            parameterToStage->TryEmitStaging(*this, true);
        }
    }


    //*************************************************************************
    //
    // ParameterStage::ParameterInfo
    //
    //*************************************************************************
    //bool ParameterStage::ParameterInfo::IsStaged() const
    //{
    //    return m_isStaged;
    //}


    //bool ParameterStage::ParameterInfo::TryEmitStaging(bool /*spillIfNecessary*/)
    //{
    //    Assert(!IsStaged(), "Parameter already staged.");

    //    //if (true /* !m_tree.IsAvailable(Register<8, false>(m_dest) && spillIfNecessary */)
    //    //{
    //    //    // Spill.
    //    //}

    //    // If the destination register is not available, it could be for two reasons:
    //    //   1. Holding some value that is not a parameter to this function call.
    //    //      Just save the value on the stack and restore later.
    //    //   2. Holding another parameter to this function call.
    //    //      Need to spill and then update that parameter info with the new location.
    //    //      Could we instead swap with the other parameter?

    //    // Idea: polymorphic wrapper for Storage<T> that has virtual methods for spilling,
    //    // transferring, etc.
    //    //
    //    // IsStaged() - returns true if embedded storage register is correct.
    //    // TryEmitStaging(true):
    //    //   Case: Destination is available. Move Storage to destination. Free source.
    //    //   Case: Destination is not available - not used by ParameterStage. Spill destination. Mark for restore.
    //    //   Case: Destination is not available - is used by ParameterStage. Transfer destination to temp.

    //    return true;
    //}


    //void ParameterStage::ParameterInfo::Restore()
    //{
    //}
}
