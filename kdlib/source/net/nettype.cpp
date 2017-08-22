#include "stdafx.h"

#include <metahost.h>

#include "net/nettype.h"
#include "net/netmodule.h"
#include "net/metadata.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr getTypeClass(ICorDebugType*  typeClass)
{
    CComPtr<ICorDebugClass>  objClass;
    HRESULT  hres = typeClass->GetClass(&objClass);
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

    return getMetaDataProvider(module)->getTypeByToken(typeToken);
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr getTypeSZArray(COR_TYPEID typeId)
{
    COR_ARRAY_LAYOUT  arrayLayout;
    HRESULT  hres = g_netMgr->targetProcess5()->GetArrayLayout(typeId, &arrayLayout);
    if ( FAILED(hres) )
        throw DbgException("Failed ICorDebugProcess5::GetArrayLayout");

    TypeInfoPtr  derefType = getTypeById(arrayLayout.componentID);

    return TypeInfoPtr( new NetTypeArray( derefType ) );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr  getTypeById(COR_TYPEID typeId)
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
        return getTypeClass(objType);

    case ELEMENT_TYPE_SZARRAY:
        return getTypeSZArray(typeId);
    }

    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////




}
