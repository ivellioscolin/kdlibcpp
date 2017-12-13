#include "stdafx.h"

#include <iomanip>

#include "kdlib/exceptions.h"
#include "kdlib/memaccess.h"

#include "net/netobject.h"
#include "net/net.h"
#include "net/nettype.h"

namespace kdlib {


///////////////////////////////////////////////////////////////////////////////

TypedVarPtr getManagedVar(MEMOFFSET_64 addr)
{

    CComPtr<ICorDebugObjectValue>  objectValue;

    HRESULT hres = g_netMgr->targetProcess5()->GetObject(addr, &objectValue);
    if ( FAILED(hres) )
        throw DbgException("Failed ICorDebugProcess5::GetObject");

    return TypedVarPtr( new NetObjectClass(objectValue) );
}

///////////////////////////////////////////////////////////////////////////////

template <typename T>
T getGenericValue(ICorDebugValue* value)
{
    CComQIPtr<ICorDebugGenericValue>  genValue = value;
    long long storage;
    HRESULT  hres = genValue->GetValue(&storage);
    if (FAILED(hres) )
        throw DbgException("Failed ICorDebugGenericValue::GetValue");
    return *reinterpret_cast<T*>(&storage);
}

///////////////////////////////////////////////////////////////////////////////

ICorDebugValue*  derefValue(ICorDebugValue* value)
{
    CComQIPtr<ICorDebugReferenceValue>   refValue = value;
    CComPtr<ICorDebugValue>   derefValue;
    HRESULT  hres = refValue->Dereference(&derefValue);
    if ( FAILED(hres) )
        throw DbgException("Failed ICorDebugReferenceValue::Dereference");

    return derefValue.Detach();
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr getElementValue(ICorDebugValue* value)
{
    CorElementType  valueType;
    HRESULT  hres = value->GetType( &valueType );
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugValue::GetType");

    switch(valueType)
    {
    case ELEMENT_TYPE_CHAR:
    case ELEMENT_TYPE_I1:
        return loadCharVar( getGenericValue<char>(value) );

    case ELEMENT_TYPE_U1:
        return loadUCharVar(getGenericValue<unsigned char>(value));

    case ELEMENT_TYPE_I2:
        return loadShortVar(getGenericValue<short>(value));

    case ELEMENT_TYPE_U2:
        return loadUShortVar(getGenericValue<unsigned short>(value));

    case ELEMENT_TYPE_I4:
        return loadLongVar(getGenericValue<long>(value));

    case ELEMENT_TYPE_U4:
        return loadULongVar(getGenericValue<unsigned long>(value));

    case ELEMENT_TYPE_I8:
        return loadLongLongVar(getGenericValue<long long>(value));

    case ELEMENT_TYPE_U8:
        return loadULongLongVar(getGenericValue<unsigned long long>(value));

    case ELEMENT_TYPE_R4:
        return loadFloatVar(getGenericValue<float>(value));

    case ELEMENT_TYPE_R8:
        return loadDoubleVar(getGenericValue<double>(value));

    case ELEMENT_TYPE_CLASS:
        return TypedVarPtr( new NetObjectClass(CComQIPtr<ICorDebugObjectValue>(derefValue(value))));

    case ELEMENT_TYPE_ARRAY:
    case ELEMENT_TYPE_SZARRAY:
        return TypedVarPtr( new NetObjectArray(CComQIPtr<ICorDebugArrayValue>(derefValue(value))));

    case ELEMENT_TYPE_STRING:
        return TypedVarPtr( new NetObjectString(CComQIPtr<ICorDebugStringValue>(derefValue(value))));

    case ELEMENT_TYPE_VALUETYPE:
        return TypedVarPtr( new NetObjectEnum(CComQIPtr<ICorDebugObjectValue>(value)));
    }

      NOT_IMPLEMENTED();

}

///////////////////////////////////////////////////////////////////////////////

NetObjectClass::NetObjectClass(ICorDebugObjectValue* objectValue)
{
    m_objectValue = objectValue;

    HRESULT  hres = objectValue->GetAddress(&m_address);
    if (FAILED(hres) )
        throw DbgException("Faild ICorDebugObjectValue::GetAddress");

    CComQIPtr<ICorDebugValue2>  objectValue2 = objectValue;
    hres = objectValue2->GetExactType(&m_objectType);
    if (FAILED(hres) )
        throw DbgException("Faild ICorDebugValue2::GetExactType");

    CComPtr<ICorDebugClass>  objectClass;
    hres = m_objectValue->GetClass(&objectClass);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugType::GetClass");

    CComPtr<ICorDebugModule>  module;
    hres = objectClass->GetModule(&module);
    if (FAILED(hres) )
        throw DbgException("Failed ICorDebugClass::GetModule");

    m_metaDataProvder = getMetaDataProvider(module);

    m_fields = getFieldsByType(m_objectType);
}

///////////////////////////////////////////////////////////////////////////////

FieldsList NetObjectClass::getFieldsByType(ICorDebugType* corType)
{
    FieldsList  fields;

    CComPtr<ICorDebugType>  baseType;
    HRESULT hres = corType->GetBase(&baseType);
    if (SUCCEEDED(hres) && baseType )
        fields = getFieldsByType(baseType);

    CComPtr<ICorDebugClass>  corClass;
    corType->GetClass(&corClass);

    mdTypeDef  typeDef;
    corClass->GetToken(&typeDef);
    
    NetFieldList  fieldList;
    m_metaDataProvder->getFields(typeDef, fieldList);

    for( auto field : fieldList )
    {
        fields.push_back( { field.first, field.second, corClass } );
    }

    return fields;
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr NetObjectClass::getElement( const std::wstring& fieldName )
{
    FieldDesc  fieldDesc;

    auto  cacheField = m_fieldCache.find(fieldName);

    if ( cacheField != m_fieldCache.end() )
    {
        fieldDesc = cacheField->second;
    }
    else
    {
        auto field = std::find_if( m_fields.begin(), m_fields.end(), [&](FieldDesc& f) { return f.name == fieldName; } );

        if ( field != m_fields.end() )
        {
            fieldDesc = *field;
            m_fieldCache.insert( std::make_pair( field->name, fieldDesc ) );
        }
        else
        {
            std::wstringstream   sstr;
            sstr << L"field \"" << fieldName << L" not found";
            throw TypeException( sstr.str() );
        }
    }

    CComPtr<ICorDebugValue>  fieldValue;
    HRESULT  hres = m_objectValue->GetFieldValue( fieldDesc.debugClass, fieldDesc.token, &fieldValue );
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugObjectValue::GetFieldValue");

    return getElementValue(fieldValue);
}

///////////////////////////////////////////////////////////////////////////////

std::wstring NetObjectClass::str()
{
    std::wstringstream  sstr;

    COR_TYPEID  typeId;
    HRESULT  hres = g_netMgr->targetProcess5()->GetTypeID(m_address, &typeId);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugProcess5::GetTypeID");

    TypeInfoPtr  typeInfo = getNetTypeById(typeId);

    sstr << L"Managed class: " << typeInfo->getName() << L" at " << std::hex << m_address << std::endl;

    for ( auto  field : m_fields )
    {
        CComPtr<ICorDebugValue>  fieldValue;
        MEMOFFSET_64  fieldAddress = 0;

        if ( SUCCEEDED(m_objectValue->GetFieldValue( field.debugClass, field.token, &fieldValue ) ) &&
            SUCCEEDED(fieldValue->GetAddress(&fieldAddress) ) )
        {
            MEMOFFSET_REL  fieldOffset = static_cast<MEMOFFSET_REL>(fieldAddress - this->getAddress());

            sstr << L"   +" << std::right << std::setw(4) << std::setfill(L'0') << std::hex << fieldOffset;
            sstr << L" " << std::left << std::setw(24) << std::setfill(L' ') << field.name << ':';
            sstr << L"   " << getElementValue(fieldValue)->printValue();
        }
        else
        {
            sstr << L"    ----  " << std::left << std::setw(24) << std::setfill(L' ') << field.name << ':';
            sstr << L"   " << L"Failed to get value";
        }

        sstr << std::endl;
    }

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

std::wstring NetObjectClass::printValue() const
{
    std::wstringstream  sstr;

    COR_TYPEID  typeId;
    HRESULT  hres = g_netMgr->targetProcess5()->GetTypeID(m_address, &typeId);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugProcess5::GetTypeID");

    TypeInfoPtr  typeInfo = getNetTypeById(typeId);

    sstr << L"Ñlass: " << typeInfo->getName() << L" at " << std::hex << m_address << std::endl;

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

NetObjectArray::NetObjectArray(ICorDebugArrayValue*  arrayValue,  std::vector<ULONG32> indices )
{
    m_arrayValue = arrayValue;
    m_indices = indices;

    ULONG32  rank = 0;

    HRESULT   hres = m_arrayValue->GetRank(&rank);
    if ( FAILED(hres) )
        throw  DbgException("Failed ICorDebugArrayValue::GetRank");

    m_dimensions.resize(rank);

    hres = m_arrayValue->GetDimensions(rank, &m_dimensions.front() );
    if ( FAILED(hres) )
        throw  DbgException("Failed ICorDebugArrayValue::GetDimensions");

    hres = m_arrayValue->GetCount(&m_elementCount);
    if ( FAILED(hres) )
        throw  DbgException("Failed ICorDebugArrayValue::GetCount");
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr NetObjectArray::getElement( size_t index )
{
    if ( index >= m_dimensions[ m_indices.size() ] )
        throw IndexException( index );

    auto  indices = m_indices;
    indices.push_back(index);

    if ( m_indices.size() + 1 < m_dimensions.size() )
    {
        return TypedVarPtr( new NetObjectArray(m_arrayValue, indices) );
    }

    size_t  rawIndex = 0;
    size_t  elements = m_elementCount;

    for ( size_t i = 0; i < m_dimensions.size(); ++i)
    {
        elements =  elements / m_dimensions[i];
        rawIndex += elements * indices[i];
    }

    CComPtr<ICorDebugValue>  elementValue;

    HRESULT  hres = m_arrayValue->GetElementAtPosition(rawIndex, &elementValue);
    if ( FAILED(hres) )
        throw  DbgException("Failed ICorDebugArrayValue::GetElementAtPosition");

    return getElementValue(elementValue);
}

///////////////////////////////////////////////////////////////////////////////

size_t NetObjectArray::getElementCount()
{
    return m_dimensions.back();
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 NetObjectArray::getAddress() const
{
    MEMOFFSET_64  address;
    HRESULT hres = m_arrayValue->GetAddress(&address);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugArrayValue::GetAddress");

    return address;
}

///////////////////////////////////////////////////////////////////////////////

std::wstring NetObjectArray::printValue() const
{
    std::wstringstream sstr;

    sstr << L"Array of ";

    CORDB_ADDRESS  address;
    HRESULT hres = m_arrayValue->GetAddress(&address);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugArrayValue::GetAddress");

    COR_TYPEID  typeId;
    hres = g_netMgr->targetProcess5()-> GetTypeID(address, &typeId);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugProcess5::GetTypeID");

    TypeInfoPtr  arrayType = getNetTypeById(typeId);

    sstr << arrayType->getName();

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

std::wstring  NetObjectString::getStrValue() const
{
    ULONG32  strLength = 0;

    HRESULT  hres = m_strValue->GetLength(&strLength);
    if ( FAILED(hres) )
        throw DbgException("Failed ICorDebugStringValue::GetLength");

    std::vector<wchar_t>  strBuffer(strLength);

    hres = m_strValue->GetString(strLength, &strLength, &strBuffer.front());
    if ( FAILED(hres) )
        throw DbgException("Failed ICorDebugStringValue::GetString");

    return std::wstring(&strBuffer.front(), strLength);
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 NetObjectString::getAddress() const
{
    MEMOFFSET_64  address;
    HRESULT hres = m_strValue->GetAddress(&address);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugStringValue::GetAddress");

    return address;
}

///////////////////////////////////////////////////////////////////////////////

std::wstring NetObjectString::printValue() const  
{
    std::wstringstream  sstr;
    sstr << '"' << getStrValue() << '"';
    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

NumVariant NetObjectEnum::getValue() const {

    CComQIPtr<ICorDebugGenericValue>  genValue = m_enumValue;
    long long storage;
    HRESULT  hres = genValue->GetValue(&storage);
    if ( FAILED(hres) )
        throw DbgException("Failed ICorDebugGenericValue::GetValue");

    return *reinterpret_cast<int*>(&storage);
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 NetObjectEnum::getAddress() const
{
    MEMOFFSET_64  address;
    HRESULT hres = m_enumValue->GetAddress(&address);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugObjectValue::GetAddress");

    return address;
}

///////////////////////////////////////////////////////////////////////////////

}

