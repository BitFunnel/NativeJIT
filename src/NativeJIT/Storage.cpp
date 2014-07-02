#include "stdafx.h"

#include "Storage.h"

namespace NativeJIT
{
    Data::Data(ExpressionTree& tree, Register<sizeof(void*), false> base, size_t offset)
        : m_refCount(0),
          m_tree(tree),
          m_storageClass(Indirect),
          m_registerId(base.GetId()),
          m_offset(offset)
    {
    }
}
