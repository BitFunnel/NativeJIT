#include "stdafx.h"

#include "Storage.h"

namespace NativeJIT
{
    Data::Data(ExpressionTree& tree, Register<sizeof(void*), false> base, __int32 offset)
        : m_refCount(0),
          m_tree(tree),
          m_storageClass(Indirect),
          m_registerId(base.GetId()),
          m_offset(offset)
    {
    }


    Data* Data::ConvertToIndirect(unsigned registerId, __int32 offset)
    {
        m_storageClass = Indirect;
        m_registerId = registerId;
        m_offset = offset;
        return this;
    }
}
