#include "stdafx.h"

#include <algorithm>    // For std::max

#include "NativeJIT/ExpressionTree.h"
#include "NativeJIT/Nodes/Node.h"
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
          m_isReferenced(false),
          m_hasBeenEvaluated(false)
    {
    }


    unsigned NodeBase::GetId() const
    {
        return m_id;
    }


    void NodeBase::IncrementParentCount()
    {
        LogThrowAssert(!HasBeenEvaluated(), "Cannot change the parent count after the node was evaluated");

        ++m_parentCount;
        MarkReferenced();
    }


    void NodeBase::DecrementParentCount()
    {
        LogThrowAssert(!HasBeenEvaluated(), "Cannot change the parent count after the node was evaluated");
        LogThrowAssert(m_parentCount > 0,
                       "Cannot decrement parent count of node %u with zero parents",
                       GetId());

        --m_parentCount;
        // Note: m_isReferenced is not affected by this, decrementing the parent
        // count is optimization-related call which doesn't change the
        // fact that a node is referenced at least conceptually. This is because
        // currently having a non-referenced node is considered to be an error,
        // and not a chance for removing such nodes.
    }


    bool NodeBase::HasBeenEvaluated() const
    {
        return m_hasBeenEvaluated;
    }


    void NodeBase::MarkEvaluated()
    {
        m_hasBeenEvaluated = true;
    }


    bool NodeBase::IsReferenced() const
    {
        return m_isReferenced;
    }


    void NodeBase::MarkReferenced()
    {
        m_isReferenced = true;
    }


    unsigned NodeBase::GetParentCount() const
    {
        return m_parentCount;
    }


    void NodeBase::CompileAsRoot(ExpressionTree& /*tree*/)
    {
        LogThrowAbort("Root of ExpressionTree must be a ReturnNode node.");
    }


    bool NodeBase::CanBeOptimizedAway() const
    {
        return GetParentCount() == 0;
    }


    void NodeBase::ReleaseReferencesToChildren()
    {
        LogThrowAbort("Don't know how to remove optimized away references for node with ID %u (orphan node?)",
                      GetId());
    }


    bool NodeBase::GetBaseAndOffset(NodeBase*& /* base */, int32_t& /* offset */) const
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
