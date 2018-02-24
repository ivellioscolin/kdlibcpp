
#include "stdafx.h"

#include "kdlib/tagged.h"
#include "win/dbgmgr.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

std::list<TaggedId> enumTagged()
{
    ULONG64 handle{};
    auto hres = g_dbgMgr->dataspace->StartEnumTagged(&handle);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugDataSpaces3::StartEnumTagged", hres);

    const auto enumDelete = 
        [](ULONG64 *handle)
        {
            const auto hres = g_dbgMgr->dataspace->EndEnumTagged(*handle);
            if (S_OK != hres)
                throw DbgEngException(L"IDebugDataSpaces3::EndEnumTagged", hres);
        };
    std::unique_ptr<ULONG64, decltype(enumDelete)> endGuard{&handle, enumDelete};

    std::list<TaggedId> result;
    for (; ; )
    {
        TaggedId id{};
        ULONG tmp{};
        hres = g_dbgMgr->dataspace->GetNextTagged(handle, &id, &tmp);
        if (S_FALSE == hres)
            break;

        if (S_OK != hres)
            throw DbgEngException(L"IDebugDataSpaces3::GetNextTagged", hres);

        result.emplace_back( std::move(id) );
    }
    return std::move(result);
}

///////////////////////////////////////////////////////////////////////////////

TaggedBuffer loadTaggedBuffer(TaggedId id)
{
    ULONG size{};
    auto hres = g_dbgMgr->dataspace->ReadTagged(&id, 0, nullptr, 0, &size);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugDataSpaces4::ReadTagged", hres);

    if (!size)
        return TaggedBuffer{};

    TaggedBuffer result(size);

    hres = g_dbgMgr->dataspace->ReadTagged(&id, 0, result.data(), size, &size);
    if (S_OK != hres)
        throw DbgEngException(L"IDebugDataSpaces4::ReadTagged", hres);

    return std::move(result);
}

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end
