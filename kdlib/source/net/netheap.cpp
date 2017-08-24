#include "stdafx.h"

#include <metahost.h>

#include "kdlib/heap.h"
#include "kdlib/exceptions.h"

#include "net/net.h"
#include "net/netheap.h"
#include "net/nettype.h"
#include "net/metadata.h"

#include <boost/regex.hpp>

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

const boost::wregex  r1(L"\\?");
const boost::wregex  r2(L"\\*");
const boost::wregex  r3(L"\\.");

bool fnmatch( const std::wstring& pattern, const std::wstring& str)
{
    std::wstring mask = pattern;
    mask = boost::regex_replace(mask, r1, L".");
    mask = boost::regex_replace(mask, r2, L".*");
    mask = boost::regex_replace(mask, r3, L"\\.");

    boost::wsmatch  matchResult;
    boost::wregex  regEx(mask);
    return boost::regex_match(str, matchResult, regEx);
}

///////////////////////////////////////////////////////////////////////////////

}

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

TargetHeapPtr getManagedHeap()
{
    return boost::make_shared<NetHeap>();
}

///////////////////////////////////////////////////////////////////////////////

size_t  NetHeap::getCount(const std::wstring&  typeName, size_t minSize, size_t maxSize) const
{
    CComPtr<ICorDebugHeapEnum>   heapEnum;
    HRESULT  hres = g_netMgr->targetProcess5()->EnumerateHeap(&heapEnum);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugProcess5::EnumerateHeap");

    size_t  elemCount = 0;

    while(true)
    {
        COR_HEAPOBJECT   heapObj;
        hres = heapEnum->Next(1, &heapObj, NULL);

        if (FAILED(hres))
            throw DbgException("Failed ICorDebugHeapEnum::Next");

        if ( S_OK != hres )
            break;

        if ( !typeName.empty() || minSize != 0 || maxSize != -1 )
        {
            TypeInfoPtr  typeObj = getTypeById(heapObj.type);

            std::wstring  tn = typeObj->getName();

            if ( !typeName.empty() && !fnmatch(typeName, typeObj->getName() ))
                continue;

            if ( minSize != 0 && heapObj.size < minSize )
                continue;

            if ( maxSize != -1 && heapObj.size > maxSize )
                continue;
        }

        elemCount++;
    }

    return elemCount;
}

///////////////////////////////////////////////////////////////////////////////

TargetHeapEnumPtr  NetHeap::getEnum(const std::wstring&  typeName, size_t minSize, size_t maxSize) 
{
    return TargetHeapEnumPtr( new NetHeapEnum(typeName, minSize, maxSize) );
}

///////////////////////////////////////////////////////////////////////////////

NetHeapEnum::NetHeapEnum(const std::wstring&  typeName, size_t minSize, size_t maxSize) :
    m_typeMask(typeName),
    m_minSize(minSize),
    m_maxSize(maxSize)
{
    HRESULT  hres = g_netMgr->targetProcess5()->EnumerateHeap(&m_heapEnum);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugProcess5::EnumerateHeap");
}

///////////////////////////////////////////////////////////////////////////////

bool NetHeapEnum::Next(MEMOFFSET_64& addr, std::wstring& typeName, size_t& size)
{
    HRESULT  hres;

    while(true)
    {
        COR_HEAPOBJECT   heapObj;
        hres = m_heapEnum->Next(1, &heapObj, NULL);

        if (FAILED(hres))
            throw DbgException("Failed ICorDebugHeapEnum::Next");

        if ( S_OK != hres )
            return false;

        TypeInfoPtr  typeObj = getTypeById(heapObj.type);

        std::wstring  tn = typeObj->getName();

        if ( !m_typeMask.empty() && !fnmatch(m_typeMask, typeObj->getName() ))
            continue;

        if ( m_minSize != 0 && heapObj.size < m_minSize )
            continue;

        if ( m_maxSize != -1 && heapObj.size > m_maxSize )
            continue;

        addr = heapObj.address;
        typeName = tn;
        size = heapObj.size;

        return true;
    }

    throw DbgException("NetHeapEnum::Next undefined behaviour");
}

///////////////////////////////////////////////////////////////////////////////

}
