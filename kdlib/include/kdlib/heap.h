#pragma once

#include <boost/smart_ptr.hpp>

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class TargetHeap;
typedef boost::shared_ptr<TargetHeap>  TargetHeapPtr;

///////////////////////////////////////////////////////////////////////////////

class TargetHeap  
{
public:

    virtual size_t  getCount(const std::wstring&  typeName=L"", size_t minSize = 0, size_t maxSize = -1) const = 0;
};

TargetHeapPtr getManagedHeap();

///////////////////////////////////////////////////////////////////////////////

}

