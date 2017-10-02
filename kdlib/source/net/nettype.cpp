#include "stdafx.h"

#include <sstream>
#include <iomanip>

#include <metahost.h>

#include "kdlib/memaccess.h"

#include "net/nettype.h"
#include "net/netmodule.h"
#include "net/metadata.h"
#include "net/netobject.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr getNetTypeClass(COR_TYPEID typeId)
{
    return TypeInfoPtr( NetTypeClass::createById(typeId) );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr getNetTypeArray(COR_TYPEID typeId)
{
    COR_ARRAY_LAYOUT  arrayLayout;
    HRESULT  hres = g_netMgr->targetProcess5()->GetArrayLayout(typeId, &arrayLayout);
    if ( FAILED(hres) )
        throw DbgException("Failed ICorDebugProcess5::GetArrayLayout");

    return TypeInfoPtr( new NetTypeArray( arrayLayout ) );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr  getNetTypeById(COR_TYPEID typeId)
{
    CComPtr<ICorDebugType>  objType;
    HRESULT  hres = g_netMgr->targetProcess5()->GetTypeForTypeID(typeId, &objType);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugProcess5::GetTypeForTypeID");

    CorElementType  elementType;
    hres = objType->GetType(&elementType);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugType::GetType");

    switch( elementType )
    {
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_VALUETYPE:
        return getNetTypeClass(typeId);

    case ELEMENT_TYPE_SZARRAY:
    case ELEMENT_TYPE_ARRAY:
        return getNetTypeArray(typeId);

    }

    NOT_IMPLEMENTED();
}


///////////////////////////////////////////////////////////////////////////////


NetTypeClass*  NetTypeClass::createById(COR_TYPEID typeId)
{
    CComPtr<ICorDebugType>  objType;
    HRESULT  hres = g_netMgr->targetProcess5()->GetTypeForTypeID(typeId, &objType);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugProcess5::GetTypeForTypeID");

    CComPtr<ICorDebugClass>  objClass;
    hres = objType->GetClass(&objClass);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugType::GetClass");

    CComPtr<ICorDebugModule>  module;
    hres = objClass->GetModule(&module);
    if (FAILED(hres) )
        throw DbgException("Failed ICorDebugClass::GetModule");

    mdTypeDef  typeToken;
    hres = objClass->GetToken(&typeToken);
    if ( FAILED(hres) )
        throw DbgException("Failed ICorDebugClass::GetToken");

    MetaDataProviderPtr  metaDataProvder = getMetaDataProvider(module);

    std::wstring   typeName = metaDataProvder->getTypeNameByToken(typeToken);

    return new NetTypeClass(typeId,metaDataProvder,typeName,typeToken);
}

///////////////////////////////////////////////////////////////////////////////

std::wstring NetTypeClass::str()
{
    std::wstringstream  sstr;

    sstr << "Managed class : " << getName() << std::endl;
    
    size_t  fieldCount = getElementCount();

    for ( size_t i = 0; i < fieldCount; ++i )
    {
        if ( isStaticMember(i) )
        {
            NOT_IMPLEMENTED();
        }
        else
        {
            sstr << L"   +" << std::right << std::setw(4) << std::setfill(L'0') << std::hex << getElementOffset(i);
            sstr << L" " << std::left << std::setw(24) << std::setfill(L' ') << getElementName(i) << L':';
        }

        sstr << L" " << std::left << getElement(i)->getName();
        sstr << std::endl;
    }

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

size_t NetTypeClass::getSize()
{
    COR_TYPE_LAYOUT  typeLayout;
    HRESULT  hres = g_netMgr->targetProcess5()->GetTypeLayout(m_typeId, &typeLayout);

    if (FAILED(hres))
        throw DbgException("Failed ICorDebugProcess5::GetTypeLayout");

    return typeLayout.objectSize;
}

///////////////////////////////////////////////////////////////////////////////

void NetTypeClass::getFields()
{
    COR_TYPE_LAYOUT  typeLayout;
    HRESULT  hres = g_netMgr->targetProcess5()->GetTypeLayout(m_typeId, &typeLayout);

    if (FAILED(hres))
        throw DbgException("Failed ICorDebugProcess5::GetTypeLayout");

    std::vector<COR_FIELD>  corFields(typeLayout.numFields);

    ULONG32  fieldsCount = typeLayout.numFields;

    hres = g_netMgr->targetProcess5()->GetTypeFields(m_typeId, fieldsCount, &corFields.front(), &fieldsCount);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugProcess5::GetTypeFields");

    for ( auto field : corFields )
    {
        m_fields.push_back( TypeFieldPtr( new NetTypeField(field, m_metaProvider) ) );
    }
}

///////////////////////////////////////////////////////////////////////////////


//NetTypeField::NetTypeField(MetaDataProviderPtr& metaProvider, const std::wstring& name, mdFieldDef token) :
//    TypeField(name),
//    m_metaProvider(metaProvider),
//    m_token(token)
//{
//    DWORD  fieldAttr = m_metaProvider->getFieldAttr(m_token);
//
//    m_staticMember = ( fieldAttr & fdStatic ) != 0;
//
//    if ( !m_staticMember )
//    {
//        m_offset = metaProvider->getFieldOffset(token);
//    }
//}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr  NetTypeField::getTypeInfo()
{
    switch(m_fieldType)
    {
    case ELEMENT_TYPE_CHAR:
        return loadType(L"Char");

    case ELEMENT_TYPE_I1:
        return loadType(L"Int1B");

    case ELEMENT_TYPE_U1:
        return loadType(L"UInt1B");

    case ELEMENT_TYPE_I2:
        return loadType(L"Int2B");

    case ELEMENT_TYPE_U2:
        return loadType(L"UInt1B");

    case ELEMENT_TYPE_I4:
        return loadType(L"Int4B");

    case ELEMENT_TYPE_U4:
        return loadType(L"UInt4B");

    case ELEMENT_TYPE_I8:
        return loadType(L"Int8B");

    case ELEMENT_TYPE_U8:
        return loadType(L"UInt8B");

    case ELEMENT_TYPE_R4:
        return loadType(L"Float");

    case ELEMENT_TYPE_R8:
        return loadType(L"Double");

    case ELEMENT_TYPE_CLASS:

         NOT_IMPLEMENTED();
 //       return TypeInfoPtr( new NetTypeClass(m_metaProvider,
 //           m_metaProvider->getFieldClassToken(m_token) ) );

    case ELEMENT_TYPE_ARRAY:
    case ELEMENT_TYPE_SZARRAY:
        return TypeInfoPtr( getNetTypeArray(m_typeId)  );
    }

    NOT_IMPLEMENTED();
}


///////////////////////////////////////////////////////////////////////////////

//std::wstring NetTypeBase::getName()
//{
//    switch(m_baseType)
//    {
//    case ELEMENT_TYPE_CHAR:
//        return L"Char";
//    case ELEMENT_TYPE_I1:
//        return L"Int1B";
//    case ELEMENT_TYPE_U1:
//        return L"UInt1B";
//    case ELEMENT_TYPE_I2:
//        return L"Int2B";
//    case ELEMENT_TYPE_U2:
//        return L"UInt2B";
//    case ELEMENT_TYPE_I4:
//        return L"Int4B";
//    case ELEMENT_TYPE_U4:
//        return L"UInt4B";
//    case ELEMENT_TYPE_I8:
//        return L"Int8B";
//    case ELEMENT_TYPE_U8:
//        return L"UInt8B";
//    case ELEMENT_TYPE_R4:
//        return L"Float";
//    case ELEMENT_TYPE_R8:
//        return L"Double";
//    case ELEMENT_TYPE_BOOLEAN:
//        return L"Bool";
//    }
//
//    NOT_IMPLEMENTED();
//}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr NetTypeArray::getElementType()
{
    return getNetTypeById(m_arrayLayout.componentID);
}

///////////////////////////////////////////////////////////////////////////////

}
