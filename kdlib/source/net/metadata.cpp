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

//TypeInfoPtr  MetaDataProvider::getTypeByToken(mdTypeDef typeDef)
//{
//    return TypeInfoPtr( new NetTypeClass( shared_from_this(), getTypeNameByToken(typeDef), typeDef ) );//}


CorElementType MetaDataProvider::getFieldElementType(mdFieldDef fieldDef)
{
    std::vector<wchar_t>  fieldNameBuf(0x100);
    ULONG  fieldNameLen;
    DWORD  flags;
    mdTypeDef  classTypeDef;
    DWORD  valueFlag;

    PCCOR_SIGNATURE  sigBlob;
    ULONG  sigBlobSize;

    HRESULT  hres = m_metaDataImport->GetFieldProps(
        fieldDef,
        &classTypeDef,
        &fieldNameBuf.front(),
        fieldNameBuf.size(),
        &fieldNameLen,
        &flags,
        &sigBlob,
        &sigBlobSize,
        &valueFlag,
        NULL,
        NULL);

    if ( FAILED(hres) )
        throw DbgException("Failed IMetaDataImport::GetFieldProps");

    ULONG callingConv = (CorCallingConvention)CorSigUncompressCallingConv(sigBlob);

    return CorSigUncompressElementType(sigBlob);

}

CorElementType MetaDataProvider::getFieldSZArrayElementType(mdFieldDef fieldDef)
{
    std::vector<wchar_t>  fieldNameBuf(0x100);
    ULONG  fieldNameLen;
    DWORD  flags;
    mdTypeDef  classTypeDef;
    DWORD  valueFlag;

    PCCOR_SIGNATURE  sigBlob;
    ULONG  sigBlobSize;

    HRESULT  hres = m_metaDataImport->GetFieldProps(
        fieldDef,
        &classTypeDef,
        &fieldNameBuf.front(),
        fieldNameBuf.size(),
        &fieldNameLen,
        &flags,
        &sigBlob,
        &sigBlobSize,
        &valueFlag,
        NULL,
        NULL);

    if ( FAILED(hres) )
        throw DbgException("Failed IMetaDataImport::GetFieldProps");

    ULONG callingConv = (CorCallingConvention)CorSigUncompressCallingConv(sigBlob);

    CorSigUncompressElementType(sigBlob);

    return CorSigUncompressElementType(sigBlob);
}


mdTypeDef MetaDataProvider::getFieldClassToken(mdFieldDef fieldDef)
{
    std::vector<wchar_t>  fieldNameBuf(0x100);
    ULONG  fieldNameLen;
    DWORD  flags;
    mdTypeDef  classTypeDef;
    DWORD  valueFlag;

    PCCOR_SIGNATURE  sigBlob;
    ULONG  sigBlobSize;

    HRESULT  hres = m_metaDataImport->GetFieldProps(
        fieldDef,
        &classTypeDef,
        &fieldNameBuf.front(),
        fieldNameBuf.size(),
        &fieldNameLen,
        &flags,
        &sigBlob,
        &sigBlobSize,
        &valueFlag,
        NULL,
        NULL);

    if ( FAILED(hres) )
        throw DbgException("Failed IMetaDataImport::GetFieldProps");

    CorSigUncompressCallingConv(sigBlob);

    CorSigUncompressElementType(sigBlob);

    return CorSigUncompressToken(sigBlob);
}

DWORD  MetaDataProvider::getFieldAttr(mdFieldDef fieldDef)
{
    mdTypeDef  classTypeDef;
    DWORD  attr;

    HRESULT  hres = m_metaDataImport->GetFieldProps(
        fieldDef,
        &classTypeDef,
        NULL,
        0,
        NULL,
        &attr,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    if ( FAILED(hres) )
        throw DbgException("Failed IMetaDataImport::GetFieldProps");

    return attr;
}

std::wstring  MetaDataProvider::getFieldName(mdFieldDef fieldDef)
{
    std::vector<wchar_t>  fieldNameBuf(0x100);
    ULONG  fieldNameLen;

    mdTypeDef  typeDef;

    HRESULT  hres = m_metaDataImport->GetFieldProps(
        fieldDef,
        &typeDef,
        &fieldNameBuf.front(),
        fieldNameBuf.size(),
        &fieldNameLen,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    if ( FAILED(hres) )
        throw DbgException("Failed IMetaDataImport::GetFieldProps");

    return std::wstring(&fieldNameBuf.front());
}

std::wstring MetaDataProvider::getTypeNameByToken(mdTypeDef typeDef)
{
    std::vector<wchar_t>  typeNameBuf(0x100);
    ULONG  typeNameLength;
    DWORD  typeDefFlags;
    mdToken  baseType;

    BOOL validToken = m_metaDataImport->IsValidToken (typeDef);

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

        std::wstring  encloseTypeName = getTypeNameByToken(encloseClass);

        typeName = encloseTypeName + std::wstring(L".") + typeName;
    }

    return typeName;
}


mdTypeDef MetaDataProvider::getTypeTokenByName(const std::wstring& typeName)
{
    mdTypeDef  typeDef;
    HRESULT  hres = m_metaDataImport->FindTypeDefByName(typeName.c_str(), 0, &typeDef);
    if (FAILED(hres) )
        throw DbgException("Failed IMetaDataImport::FindTypeDefByName");

    return typeDef;
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

void MetaDataProvider::getFields(mdTypeDef typeDef, NetFieldList& fields)
{
    HCORENUM  enumFieldDefs = NULL;

    fields.clear();
 
    auto enumCloseFn = [=](HCORENUM*){ if ( enumFieldDefs ) m_metaDataImport->CloseEnum(enumFieldDefs); };
    std::unique_ptr<HCORENUM, decltype(enumCloseFn)>  enumCloser(&enumFieldDefs, enumCloseFn);

    while(true)
    {
        mdFieldDef  fieldDef;
        ULONG  fetched;
        HRESULT  hres = m_metaDataImport->EnumFields(&enumFieldDefs, typeDef, &fieldDef, 1, &fetched);
        
        if ( hres != S_OK )
            break;

        std::vector<wchar_t>  fieldNameBuf(0x100);
        ULONG  fieldNameLen;

        hres = m_metaDataImport->GetFieldProps(
            fieldDef,
            &typeDef,
            &fieldNameBuf.front(),
            fieldNameBuf.size(),
            &fieldNameLen,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL);

        if ( FAILED(hres) )
            throw DbgException("Failed IMetaDataImport::GetFieldProps");

        fields.push_back( std::make_pair( std::wstring(&fieldNameBuf.front()), fieldDef) );
    }
}

bool MetaDataProvider::isStaticField(mdFieldDef fieldDef)
{
    std::vector<wchar_t>  fieldNameBuf(0x100);
    ULONG  fieldNameLen;

    mdTypeDef  typeDef;

    DWORD  fieldAttr;

    HRESULT  hres = m_metaDataImport->GetFieldProps(
        fieldDef,
        &typeDef,
        &fieldNameBuf.front(),
        fieldNameBuf.size(),
        &fieldNameLen,
        &fieldAttr,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    if ( FAILED(hres) )
        throw DbgException("Failed IMetaDataImport::GetFieldProps");

    return (fieldAttr & CorFieldAttr::fdStatic) != 0;
}

}
