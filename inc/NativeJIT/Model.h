#pragma once

#include <type_traits>


namespace NativeJIT
{
    template <typename PACKED>
    class Model
    {
    public:
        typedef PACKED PackedType;

        Model();

        float Apply(PACKED packed) const;

        float& operator[](unsigned index);
        float const & operator[](unsigned index) const;

        float& operator[](PACKED packed);
        float const & operator[](PACKED packed) const;

        // m_data must be public or friend of NativeJIT::ExpressionNodeFactory.
        // Otherwise the JIT compiler cannot access m_data.
        // TODO: Policy decision. Do we require members be public or do we
        // use the friend keyword?.

        static const unsigned c_size = 1 << PACKED::c_totalBitCount;
        float m_data[c_size];
    };


    //*************************************************************************
    //
    // Template definitions for Model<PACKED>
    //
    //*************************************************************************
#pragma warning(push)
#pragma warning(disable:4351)
    template <typename PACKED>
    Model<PACKED>::Model()
        : m_data()
    {
    }
#pragma warning(pop)


    template <typename PACKED>
    float Model<PACKED>::Apply(PACKED packed) const
    {
        return m_data[packed.m_fields];
    }


    template <typename PACKED>
    float& Model<PACKED>::operator[](unsigned index)
    {
        return m_data[index];
    }


    template <typename PACKED>
    float const & Model<PACKED>::operator[](unsigned index) const
    {
        return m_data[index];
    }


    template <typename PACKED>
    float& Model<PACKED>::operator[](PACKED packed)
    {
        return m_data[packed.m_fields];
    }


    template <typename PACKED>
    float const & Model<PACKED>::operator[](PACKED packed) const
    {
        return m_data[packed.m_fields];
    }
}
