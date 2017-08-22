#pragma once

#include "kdlib/heap.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class NetHeap : public TargetHeap
{
protected:

    virtual size_t  getCount(const std::wstring&  typeName, size_t minSize = 0, size_t maxSize = -1) const;

};

///////////////////////////////////////////////////////////////////////////////

}
