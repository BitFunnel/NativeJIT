#pragma once

#include "Node.h"


namespace NativeJIT
{
    template <typename T>
    class Indirect : public Node<T>
    {
    public:
        typedef typename Node<T*>::RegisterType BaseRegisterType;

        Indirect(ExpressionTree& tree, Node<T*>& base, unsigned __int64 offset);

        BaseRegisterType CodeGenBase(ExpressionTree& tree);
        bool IsBaseRegisterCached() const;

        //
        // Overrides of Node methods.
        //

        virtual RegisterType CodeGenValue(ExpressionTree& tree) override;
        virtual unsigned __int64 GetOffset() const override;
        virtual bool IsImmediate() const override;
        virtual bool IsIndirect() const override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    private:
        Node<T*>& m_base;
        unsigned __int64 m_offset;
    };


    //*************************************************************************
    //
    // Template definitions for IndirectBase
    //
    //*************************************************************************
    template <typename T>
    Indirect<T>::Indirect(ExpressionTree& tree,
                          Node<T*>& base,
                          unsigned __int64 offset)
        : Node(tree),
          m_base(base),
          m_offset(offset)
    {
        m_base.IncrementParentCount();
    }


    template <typename T>
    typename Indirect<T>::BaseRegisterType Indirect<T>::CodeGenBase(ExpressionTree& tree)
    {
        if (m_base.IsFieldPointer())
        {
            auto & base = reinterpret_cast<FieldPointerBase<T>&>(m_base);
            auto r = base.CodeGenBase(tree);
            return r;
        }
        else
        {
            BaseRegisterType r = m_base.CodeGenValue(tree);
            return r;
        }
    }


    template <typename T>
    bool Indirect<T>::IsBaseRegisterCached() const
    {
        return m_base.IsCached();
    }


    template <typename T>
    typename Node<T>::RegisterType Indirect<T>::CodeGenValue(ExpressionTree& tree)
    {
        if (IsCached())
        {
            RegisterType r = GetCacheRegister();
            ReleaseCache();
            return r;
        }
        else
        {
            BaseRegisterType base = CodeGenBase(tree);
            unsigned __int64 offset = m_base.GetOffset();

            if (IsBaseRegisterCached())
            {
                RegisterType dest = tree.AllocateRegister<RegisterType>();
                tree.GetCodeGenerator().Op("mov", dest, base, m_offset + offset);
                return dest;
            }
            else
            {
                // TODO: Reusing the base register for the result won't work where T is
                // float or double because the base register has an incompatible type.
                // Also, RegisterType(base) can fail if, for example, the code attempts
                // to convert RSI to 1-byte.
                tree.GetCodeGenerator().Op("mov", base, base, m_offset + offset);
                return RegisterType(base);
            }
        }
    }


    template <typename T>
    unsigned __int64 Indirect<T>::GetOffset() const
    {
        return m_offset;
    }


    template <typename T>
    bool Indirect<T>::IsImmediate() const
    {
        return false;
    }


    template <typename T>
    bool Indirect<T>::IsIndirect() const
    {
        return true;
    }


    template <typename T>
    unsigned Indirect<T>::LabelSubtree(bool isLeftChild)
    {
        // TODO: Should isLeftChild be passed down?
        SetRegisterCount(m_base.LabelSubtree(true));
        return GetRegisterCount();
    }


    template <typename T>
    void Indirect<T>::Print() const
    {
        std::cout << "Indirect id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", offset = " << m_offset;
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }
}
