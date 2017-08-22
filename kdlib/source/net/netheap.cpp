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

            if ( minSize != 0 && typeObj->getSize() < minSize )
                continue;

            if ( maxSize != -1 && typeObj->getSize() > maxSize )
                continue;
        }

        elemCount++;
    }

    return elemCount;
}

///////////////////////////////////////////////////////////////////////////////

}
