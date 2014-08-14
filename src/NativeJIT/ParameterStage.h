#pragma once

#include <vector>

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

        class ParameterInfo
        {
        public:
            bool IsStaged() const;
            bool TryEmitStaging(bool spillIfNecessary);
            void Restore();

        private:
            Data* m_data;
            bool m_isStaged;
        };

        FunctionBuffer& m_code;
        std::vector<ParameterInfo> m_parameters;
    };
}
