#pragma once

#include <boost/smart_ptr.hpp>

#include "kdlib/dbgtypedef.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class TargetHeap;
typedef boost::shared_ptr<TargetHeap>  TargetHeapPtr;

class TargetHeapEnum;
typedef boost::shared_ptr<TargetHeapEnum>  TargetHeapEnumPtr;

///////////////////////////////////////////////////////////////////////////////

class TargetHeap  
{
public:

    virtual size_t  getCount(const std::wstring&  typeName=L"", size_t minSize = 0, size_t maxSize = -1) const = 0;

    virtual TargetHeapEnumPtr  getEnum(const std::wstring&  typeName=L"", size_t minSize = 0, size_t maxSize = -1)  = 0;
};

class TargetHeapEnum
{
public:

    virtual bool Next(MEMOFFSET_64& addr, std::wstring& typeName, size_t&  typeSize) = 0;
};

TargetHeapPtr getManagedHeap();

///////////////////////////////////////////////////////////////////////////////

}

