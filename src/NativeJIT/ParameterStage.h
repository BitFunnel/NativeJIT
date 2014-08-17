#pragma once

#include <vector>

#include "Storage.h"
#include "Temporary/NonCopyable.h"


namespace NativeJIT
{
    class Data;
    class FunctionBuffer;
    template <typename T> class Storage;


    class ParameterStage : public NonCopyable
    {
    public:
        ParameterStage(FunctionBuffer& code);

        template <typename T>
        void AddParameter(Storage<T>& storage);

        void EmitCall(void* functionPtr);

    private:
        void StageOneParameter();

        template <unsigned SIZE, bool ISFLOAT>
        bool IsAvailable(Register<SIZE, ISFLOAT> r) const;

        class ParameterBase
        {
        public:
            virtual bool IsStaged() const = 0;
            virtual bool TryEmitStaging(ParameterStage& stage, bool spillIfNecessary) = 0;
            virtual void Restore() = 0;

        private:
            Data* m_data;
            bool m_isStaged;
        };


        template <typename T>
        class Parameter : public ParameterBase
        {
        public:
            Parameter(Storage<T>& storage);

            bool IsStaged() const override;
            bool TryEmitStaging(ParameterStage& stage, bool spillIfNecessary) override;
            void Restore() override;

        private:
            bool m_isStaged;
            Storage<T> m_storage;
            typename Storage<T>::DirectRegister m_destination;
        };

        FunctionBuffer& m_code;
        std::vector<ParameterBase*> m_parameters;
    };


    template <typename T>
    bool ParameterStage::Parameter<T>::IsStaged() const
    {
        return m_isStaged;
//        return (m_storage.GetClass() == Data::Direct) && (m_storage.GetDirectRegister() == m_register);
    }


    template <typename T>
    bool ParameterStage::Parameter<T>::TryEmitStaging(ParameterStage& stage, bool spillEnabled)
    {
        Assert(!IsStaged(), "Parameter already staged.");

        if (!stage.IsAvailable(m_destination))
        {
            if (spillEnabled)
            {
                // Note: register may be in used as direct or as indirect.

                // If used by parameter stage
                // else // not used by parameter stage
                ParameterBase *p = stage.Find(m_register);
                if (p != nullptr)
                {
                    // One of the parameters uses the destination register.
                    if (p->HasOneOwner())
                    {
                        // Relocate its value in order to free up the destination register.
                        p->Relocate();
                    }
                    else
                    {
                        // Push its value to memory so that it can be restored after the function call.
                        stage.Save(m_register);
                    }
                }
                else
                {
                    // Destination register in use, but not by one of the parameters.
                    stage.Save(m_register);

                    //// Destination register is available.
                    //p->Relocate(m_register);
                }
            }
            else
            {
                return false;
            }
        }

        return true;
    }


    template <typename T>
    void ParameterStage::Parameter<T>::Restore()
    {
    }

/*
Idea: could require that all Storage items be converted to value before staging.
  Downside is extra register movs.
  Upside is that indirect case is eliminated.

Idea: prove that two Storage items cannot reference the same register unless they share the same Data.


if (src == dest)
{
    // We're done. No further work needed.
}
else
{
    if (dest can be acquired)
    {
        // Acquire dest.
        // mov dest, src
        // release src
    }
    else
    {
        // Must wrest it away from other users.
        // Allocate temp register
        //   ISSUE: What if no temp registers available?
        //   ISSUE: What kind of storage should be allocated for temp? It is possible that a staged param
        //          references [dest], while we would like to allocate dest direct to use.
        // mov temp, dest
        for (each staged parameter Storage that uses dest)
        {
            // Replace with equivalent Storage that uses temp.
            //    Does Storage have safeguards against releasing the BP? Yes. Code releases temporary instead.
            // ISSUE: This only works if temp is a register and not a general Storage.
            // Special cases for replace
            //    Multiple staged parameters use the same RXX.
            //        e.g. same parameter passed twice
            //        e.g. one storage is RXX direct and another is [RXX] indirect, resulting in two distinct Data structures.
            //             Can this ever happen?
            //        e.g. one storage is [RXX + 1] and another is [RXX + 2]
            //             Can this ever happen?
            //    Storage is floating point [RXX] indirect. What type is temp? RXX or XMM?
        }

        if (dest can now be acquired)
        {
            // dest was only being used by staged parameters which have, at this point, all transitioned to temp.
            // Acquire dest.
            mov dest, src
        }
        else
        {
            // We've swiped dest out from under some other, non-parameter Storage.
            // Just need to restore the value of dest after the function call returns.
            // Add (dest, src) to restore list.
            mov dest, src
        }
    }
}

Idea:
  1. Enforce the fact that an allocated register can only be associated with a single Data.
  2. Maintain mapping from register to its Data*.
  3. Allocate(Register<>) method will spill existing Data* to [BP + temp] then allocate new Data and return as Storage.

  1. Implement register to Data* mapping, with special pointer values for available (nullptr) and permanantly
     reserved (some constant that can never be a Data*).
  2. Implement replacements for AllocateRegister() which return Storage. One version allocates any available
     register, the other allocates a requested register.
  3. Remove old AllocateRegister() code.
  4. Rework old Storage::Spill() method.
*/
}
