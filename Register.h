#pragma once

#include "Assert.h"

namespace NativeJIT
{
    class RegisterBase
    {
    protected:
        static unsigned c_sizes[9];
        static char const * c_names[2][4][16];
    };


    template <unsigned SIZE, bool ISFLOAT>
    class Register : public RegisterBase
    {
    public:
        Register()
            : m_id(0)
        {
        }


        Register(unsigned id)
            : m_id(id)
        {
            Assert(id < 16, "Register id cannot exceed 15.");
        }


        template <unsigned SIZE>
        Register(Register<SIZE, false> r)
            : m_id(r.GetId())
        {
        }


        unsigned GetId() const
        {
            return m_id;
        }


        char const * GetName() const
        {
            return c_names[ISFLOAT ? 1 : 0][c_sizes[SIZE]][m_id];
        }

    private:
        unsigned m_id;
    };
}
