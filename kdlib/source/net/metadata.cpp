#include "stdafx.h"

#include <boost/make_shared.hpp>
#include <boost/regex.hpp>

#include <metahost.h>

#include "net/metadata.h"
#include "net/nettype.h"


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

MetaDataProviderPtr  getMetaDataProvider(ICorDebugModule*  module)
{
    return MetaDataProviderPtr( new MetaDataProvider(module) );
}

MetaDataProvider::MetaDataProvider(ICorDebugModule* module)
{
    HRESULT  hres = module->GetMetaDataInterface(IID_IMetaDataImport, (IUnknown**)&m_metaDataImport);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugModule::GetMetaDataInterface");
}

TypeInfoPtr  MetaDataProvider::getTypeByToken(mdTypeDef typeDef)
{
    return TypeInfoPtr( new NetTypeClass( shared_from_this(), getTypeNameByToken(typeDef), typeDef ) );
}


std::wstring MetaDataProvider::getTypeNameByToken(mdTypeDef typeDef)
{
    std::vector<wchar_t>  typeNameBuf(0x100);
    ULONG  typeNameLength;
    DWORD  typeDefFlags;
    mdToken  baseType;

    HRESULT  hres = m_metaDataImport->GetTypeDefProps(
        typeDef,
        &typeNameBuf.front(),
        typeNameBuf.size(),
        &typeNameLength,
        &typeDefFlags,
        &baseType );

    if ( FAILED(hres) )
        throw DbgException("Failed IMetaDataImport::GetTypeDefProps");

    std::wstring  typeName( &typeNameBuf.front());

    if ( (  typeDefFlags & tdVisibilityMask ) > tdPublic )
    {
        // nested class
        mdTypeDef  encloseClass;
        hres = m_metaDataImport->GetNestedClassProps(typeDef, &encloseClass);
        if ( FAILED(hres) )
            throw DbgException("Failed IMetaDataImport::GetNestedClassProps");

        TypeInfoPtr  encloseType = getTypeByToken(encloseClass);

        typeName = encloseType->getName() + std::wstring(L".") + typeName;
    }

    return typeName;
}


TypeNameList  MetaDataProvider::getTypeNames(const std::wstring& mask)
{
    HCORENUM  enumTypeDefs = NULL;
    TypeNameList  typeList;

    auto enumCloseFn = [=](HCORENUM*){ if ( enumTypeDefs ) m_metaDataImport->CloseEnum(enumTypeDefs); };
    std::unique_ptr<HCORENUM, decltype(enumCloseFn)>  enumCloser(&enumTypeDefs, enumCloseFn);

    while(true)
    {
        mdTypeDef  typeDef;
        ULONG  fetched;
        HRESULT  hres = m_metaDataImport->EnumTypeDefs(&enumTypeDefs, &typeDef, 1, &fetched);

        if ( FAILED(hres) )
            throw DbgException("Failed IMetaDataImport::EnumTypeDefs");

        if ( hres != S_OK )
            break;

        std::wstring  typeName = getTypeNameByToken(typeDef);

        if ( mask.empty() || fnmatch(mask, typeName) )
            typeList.push_back(typeName);
    } 

    return typeList;
}

}
