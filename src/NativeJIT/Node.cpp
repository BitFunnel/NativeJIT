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
          m_parentCount(0),
          m_isEvaluated(false),
          m_isInsideTree(false)
    {
    }


    unsigned NodeBase::GetId() const
    {
        return m_id;
    }


    void NodeBase::IncrementParentCount()
    {
        Assert(!IsEvaluated(), "Cannot change the parent count after the node was evaluated");

        ++m_parentCount;
        MarkInsideTree();
    }


    void NodeBase::DecrementParentCount()
    {
        Assert(!IsEvaluated(), "Cannot change the parent count after the node was evaluated");
        Assert(m_parentCount > 0,
               "Cannot decrement parent count of node %u with zero parents",
               GetId());

        --m_parentCount;
        // Note: m_isInsideTree is not affected by this, decrementing the parent
        // count is optimization-related call which doesn't change the
        // fact that a node at least conceptually belongs inside the tree.
    }


    bool NodeBase::IsEvaluated() const
    {
        return m_isEvaluated;
    }


    void NodeBase::MarkEvaluated()
    {
        m_isEvaluated = true;
    }


    bool NodeBase::IsInsideTree() const
    {
        return m_isInsideTree;
    }


    void NodeBase::MarkInsideTree()
    {
        m_isInsideTree = true;
    }


    unsigned NodeBase::GetParentCount() const
    {
        return m_parentCount;
    }


    void NodeBase::CompileAsRoot(ExpressionTree& /*tree*/)
    {
        Assert(false, "Root of ExpressionTree must be a ReturnNode node.");
    }


    bool NodeBase::CanBeOptimizedAway() const
    {
        return GetParentCount() == 0;
    }


    void NodeBase::ReleaseReferencesToChildren()
    {
        Assert(false,
               "Don't know how to remove optimized away references for node with ID %u (orphan node?)",
               GetId());
    }


    bool NodeBase::GetBaseAndOffset(NodeBase*& /* base */, __int32& /* offset */) const
    {
        return false;
    }


    unsigned NodeBase::ComputeRegisterCount(unsigned leftTreeCount, unsigned rightTreeCount)
    {
        if (leftTreeCount != rightTreeCount)
        {
            return (std::max)(leftTreeCount, rightTreeCount);
        }
        else
        {
            return leftTreeCount + 1;
        }
    }
}
