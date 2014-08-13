#include "stdafx.h"
//
//#include "Function.h"
//#include "ExpressionNodeFactory.h"
//#include "ExpressionTree.h"
//
//
//namespace NativeJIT
//{
//    FunctionBase::FunctionBase(Allocators::IAllocator& allocator, FunctionBuffer& code)
//        : m_code(code),
//          m_tree(allocator, code),
//          m_factory(allocator, m_tree)
//    {
//    }
//
//
//    ExpressionNodeFactory& FunctionBase::GetFactory()
//    {
//        return m_factory;
//    }
//
//
//    void FunctionBase::Compile()
//    {
//        m_tree.Compile();
//    }
//
//
//    void const * FunctionBase::GetUntypedEntryPoint() const
//    {
//        return m_code.GetEntryPoint();
//    }
//}
