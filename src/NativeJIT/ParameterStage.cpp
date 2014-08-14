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
            m_parameters[i].Restore();
        }
    }


    void ParameterStage::StageOneParameter()
    {
        ParameterInfo* parameterToStage = nullptr;

        // Scan through the list examining parameters that haven't yet been staged.
        for (unsigned i = 0; i < m_parameters.size(); ++i)
        {
            if (!m_parameters[i].IsStaged())
            {
                // Attempt to stage this parameter.
                parameterToStage = &m_parameters[i];
                if (parameterToStage->TryEmitStaging(false))
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
            parameterToStage->TryEmitStaging(true);
        }
    }


    //*************************************************************************
    //
    // ParameterStage::ParameterInfo
    //
    //*************************************************************************
    bool ParameterStage::ParameterInfo::IsStaged() const
    {
        return m_isStaged;
    }


    bool ParameterStage::ParameterInfo::TryEmitStaging(bool /*spillIfNecessary*/)
    {
        Assert(!IsStaged(), "Parameter already staged.");

        //if (true /* !m_tree.IsAvailable(Register<8, false>(m_dest) && spillIfNecessary */)
        //{
        //    // Spill.
        //}

        return true;
    }


    void ParameterStage::ParameterInfo::Restore()
    {
    }
}
