#include "stdafx.h"

#include "Function.h"
#include "ExpressionNodeFactory.h"
#include "ExpressionTree.h"


namespace NativeJIT
{
    FunctionBase::FunctionBase(Allocators::IAllocator& allocator, std::ostream& out)
        : m_code(out),
          m_tree(allocator, m_code),
          m_factory(allocator, m_tree)
    {
    }


    ExpressionNodeFactory& FunctionBase::GetFactory()
    {
        return m_factory;
    }


    void FunctionBase::Compile()
    {
        m_tree.Compile();
    }
}
