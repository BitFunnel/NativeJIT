#pragma once

#include "Node.h"


namespace NativeJIT
{
    template <typename FIELD>
    class FieldPointerBase : public Node<FIELD*>
    {
    public:
        typedef typename Node<FIELD*>::RegisterType BaseRegisterType;

        FieldPointerBase(ExpressionTree& tree);

        virtual bool IsFieldPointer() const override;

        virtual BaseRegisterType CodeGenBase(ExpressionTree& tree) = 0;
    };


    template <typename OBJECT, typename FIELD>
    class FieldPointerNode : public FieldPointerBase<FIELD>
    {
    public:
        FieldPointerNode(ExpressionTree& treem, Node<OBJECT*>& base, FIELD OBJECT::*field);

        bool IsBaseRegisterCached() const;

        //
        // Overrides of FieldPointerBase methods
        //

        virtual BaseRegisterType CodeGenBase(ExpressionTree& tree) override;

        //
        // Overrides of Node methods
        //

        virtual RegisterType CodeGenValue(ExpressionTree& tree) override;
        virtual unsigned __int64 GetOffset() const override;
        virtual bool IsImmediate() const override;
        virtual bool IsIndirect() const override;
        virtual unsigned LabelSubtree(bool isLeftChild) override;
        virtual void Print() const override;

    private:
        static unsigned __int64 Offset(FIELD OBJECT::*field)
        {
            return reinterpret_cast<unsigned __int64>(&((static_cast<OBJECT*>(nullptr))->*field));
        }

        Node<OBJECT*>& m_base;
        unsigned __int64 m_offset;
    };


    //*************************************************************************
    //
    // Template definitions for FieldPointerBase
    //
    //*************************************************************************
    template <typename FIELD>
    FieldPointerBase<FIELD>::FieldPointerBase(ExpressionTree& tree)
        : Node(tree)
    {
    }


    template <typename FIELD>
    bool FieldPointerBase<FIELD>::IsFieldPointer() const
    {
        return true;
    }



    //*************************************************************************
    //
    // Template definitions for FieldPointerNode
    //
    //*************************************************************************
    template <typename OBJECT, typename FIELD>
    FieldPointerNode<OBJECT, FIELD>::FieldPointerNode(ExpressionTree& tree,
                                              Node<OBJECT*>& base,
                                              FIELD OBJECT::*field)
        : FieldPointerBase(tree),
          m_base(base),
          m_offset(Offset(field))
    {
        m_base.IncrementParentCount();
    }


    template <typename OBJECT, typename FIELD>
    bool FieldPointerNode<OBJECT, FIELD>::IsBaseRegisterCached() const
    {
        return m_base.IsCached();
    }


    template <typename OBJECT, typename FIELD>
    typename FieldPointerNode<OBJECT, FIELD>::BaseRegisterType FieldPointerNode<OBJECT, FIELD>::CodeGenBase(ExpressionTree& tree)
    {
        if (m_base.IsFieldPointer())
        {
            auto & base = reinterpret_cast<FieldPointerBase<FIELD>&>(m_base);
            auto r = base.CodeGenBase(tree);
            return r;
        }
        else
        {
            return m_base.CodeGenValue(tree);
        }
    }


    template <typename OBJECT, typename FIELD>
    typename FieldPointerNode<OBJECT, FIELD>::RegisterType FieldPointerNode<OBJECT, FIELD>::CodeGenValue(ExpressionTree& tree)
    {
        if (IsCached())
        {
            RegisterType r = GetCacheRegister();
            ReleaseCache();
            return r;
        }
        else
        {
            RegisterType base = CodeGenBase(tree);
            unsigned __int64 offset = m_base.GetOffset();

            if (m_base.IsCached())
            {
                base = tree.CopyRegister(base);
            }

            tree.GetCodeGenerator().Op("add", base, m_offset + offset);

            return base;
        }
    }


    template <typename OBJECT, typename FIELD>
    unsigned __int64 FieldPointerNode<OBJECT, FIELD>::GetOffset() const
    {
        return m_offset + m_base.GetOffset();
    }


    template <typename OBJECT, typename FIELD>
    bool FieldPointerNode<OBJECT, FIELD>::IsImmediate() const
    {
        return false;
    }


    template <typename OBJECT, typename FIELD>
    bool FieldPointerNode<OBJECT, FIELD>::IsIndirect() const
    {
        return false;
    }


    template <typename OBJECT, typename FIELD>
    unsigned FieldPointerNode<OBJECT, FIELD>::LabelSubtree(bool isLeftChild)
    {
        // TODO: Should isLeftChild be passed down?
        SetRegisterCount(m_base.LabelSubtree(true));
        return GetRegisterCount();
    }


    template <typename OBJECT, typename FIELD>
    void FieldPointerNode<OBJECT, FIELD>::Print() const
    {
        std::cout << "FieldPointerNode id=" << GetId();
        std::cout << ", parents = " << GetParentCount();
        std::cout << ", offset = " << m_offset;
        std::cout << ", ";
        PrintRegisterAndCacheInfo();
    }
}
