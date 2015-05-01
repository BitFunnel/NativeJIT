#include "stdafx.h"

#include <algorithm>    // For std::max

#include "ExpressionTree.h"
#include "Node.h"
#include "Temporary/Assert.h"


namespace NativeJIT
{
    //*************************************************************************
    //
    // NodeBase
    //
    //*************************************************************************
    NodeBase::NodeBase(ExpressionTree& tree)
        : m_id(tree.AddNode(*this)),
          m_parentCount(0)
    {
    }


    unsigned NodeBase::GetId() const
    {
        return m_id;
    }


    // TODO: Figure out a way to make it less probable/impossible to forget to
    // call this.
    void NodeBase::IncrementParentCount()
    {
        ++m_parentCount;
    }


    unsigned NodeBase::GetParentCount() const
    {
        return m_parentCount;
    }


    void NodeBase::CompileAsRoot(ExpressionTree& /*tree*/)
    {
        Assert(false, "Root of ExpressionTree must be a ReturnNode node.");
    }


    __int32 NodeBase::GetOffset() const
    {
        return 0;
    }


    unsigned NodeBase::ComputeRegisterCount(unsigned leftTreeCount, unsigned rightTreeCount)
    {
        if (leftTreeCount != rightTreeCount)
        {
            return std::max(leftTreeCount, rightTreeCount);
        }
        else
        {
            return leftTreeCount + 1;
        }
    }
}
