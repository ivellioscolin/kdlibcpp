#pragma once

#include <atlbase.h>
#include <CorDebug.h>

#include "kdlib/heap.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class NetHeap : public TargetHeap
{
protected:

    virtual size_t  getCount(const std::wstring&  typeName, size_t minSize = 0, size_t maxSize = -1) const;

    virtual TargetHeapEnumPtr  getEnum(const std::wstring&  typeName=L"", size_t minSize = 0, size_t maxSize = -1);

};

class NetHeapEnum : public TargetHeapEnum
{

public:

    NetHeapEnum(const std::wstring&  typeName=L"", size_t minSize = 0, size_t maxSize = -1);

private:

    virtual bool Next(MEMOFFSET_64& addr, std::wstring& typeName, size_t&  size);

private:

    std::wstring  m_typeMask;
    size_t  m_minSize;
    size_t  m_maxSize;
    CComPtr<ICorDebugHeapEnum>   m_heapEnum;
};

///////////////////////////////////////////////////////////////////////////////

}
