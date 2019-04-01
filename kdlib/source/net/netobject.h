#pragma once

#include "kdlib/heap.h"

#include <map>

#include <atlbase.h>

#include "net.h"
#include "net/metadata.h"

#include "kdlib/typedvar.h"
#include "kdlib/exceptions.h"

///////////////////////////////////////////////////////////////////////////////

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class NetObject : public TypedVar
{
public:

    NetObject()
    {}

protected:

    std::wstring str() {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual VarStorage getStorage() const {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual std::wstring  getRegisterName() const {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual MEMOFFSET_64 getAddress() const {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual MEMOFFSET_64 getDebugStart() const {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual MEMOFFSET_64 getDebugEnd() const {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual size_t getSize() const {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual std::wstring getName() const {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual TypedVarPtr getElement( const std::wstring& fieldName ) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual TypedVarPtr getElement( size_t index ) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual VarStorage getElementStorage(const std::wstring& fieldName) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual VarStorage getElementStorage(size_t index ) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual MEMOFFSET_REL getElementOffset(const std::wstring& fieldName ) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual MEMOFFSET_REL getElementOffset(size_t index ) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual RELREG_ID getElementOffsetRelativeReg(const std::wstring& fieldName ) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual RELREG_ID getElementOffsetRelativeReg(size_t index ) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual unsigned long getElementReg(const std::wstring& fieldName) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual unsigned long getElementReg(size_t index) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual size_t getElementCount() {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual std::wstring getElementName( size_t index ) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual size_t getElementIndex(const std::wstring&  elementName) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual TypedVarPtr getMethod( const std::wstring &name, const std::wstring&  prototype = L"") {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual TypedVarPtr getMethod( const std::wstring &name, TypeInfoPtr prototype) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual TypeInfoPtr getType() const {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual NumVariant getValue() const {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual void setValue(const NumVariant& value) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual std::wstring  getStrValue() const {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual void setStrValue(const std::wstring& value) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual std::wstring printValue() const  {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual TypedVarPtr deref() {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual TypedVarPtr castTo(const std::wstring& typeName) const {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual TypedVarPtr castTo(const TypeInfoPtr &typeInfo) const {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual void writeBytes(const DataAccessorPtr& stream, size_t pos = 0) const {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual TypedValue call(const TypedValueList& arglst) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual void setElement( const std::wstring& fieldName, const TypedValue& value) {
        throw TypeException(L"Not applicable for managed objects");
    }

    virtual void setElement( size_t index, const TypedValue& value ) {
        throw TypeException(L"Not applicable for managed objects");
    }

};

///////////////////////////////////////////////////////////////////////////////

struct FieldDesc
{
    std::wstring  name;
    mdFieldDef  token;
    CComPtr<ICorDebugClass>  debugClass;
    bool isStatic;
};

typedef  std::vector< FieldDesc >  FieldsList;

class NetObjectClass : public NetObject
{
public:

    NetObjectClass(ICorDebugObjectValue* objectValue);

public:
    
     virtual TypedVarPtr getElement( const std::wstring& fieldName );

     virtual MEMOFFSET_64 getAddress() const {
        return m_address;
     }

     virtual std::wstring printValue() const;

     virtual std::wstring str();

private:

    CComPtr<ICorDebugObjectValue>  m_objectValue;

    CComPtr<ICorDebugType>   m_objectType;
 
    std::map<std::wstring, FieldDesc>  m_fieldCache;

    FieldsList  m_fields;

    MetaDataProviderPtr  m_metaDataProvder;

    MEMOFFSET_64  m_address;

    FieldsList getFieldsByType(ICorDebugType* corType);
};

///////////////////////////////////////////////////////////////////////////////

class NetObjectString : public NetObject
{
public:

    NetObjectString(ICorDebugStringValue* strValue) :
        m_strValue(strValue)
    {}

    virtual std::wstring  getStrValue() const;

    virtual MEMOFFSET_64 getAddress() const;

    virtual std::wstring printValue() const;

private:

    CComPtr<ICorDebugStringValue>  m_strValue;
};

///////////////////////////////////////////////////////////////////////////////

class NetObjectArray : public NetObject
{

public:

    NetObjectArray(ICorDebugArrayValue*  arrayValue,  std::vector<ULONG32> indices ={} );
 
private:


    virtual TypedVarPtr getElement( size_t index );

    virtual size_t getElementCount();

    virtual MEMOFFSET_64 getAddress() const;

    virtual std::wstring printValue() const;

private:

    CComPtr<ICorDebugArrayValue>  m_arrayValue;

    std::vector<ULONG32>  m_indices;

    std::vector<ULONG32>  m_dimensions;

    ULONG32  m_elementCount;
};

///////////////////////////////////////////////////////////////////////////////

class NetObjectEnum : public NetObject
{

public:
    NetObjectEnum(ICorDebugObjectValue*  value) :
        m_enumValue(value)
    {}

private:

    virtual NumVariant getValue() const;

    virtual MEMOFFSET_64 getAddress() const;

    virtual std::wstring printValue() const;

private:

    CComPtr<ICorDebugObjectValue>  m_enumValue;

};

///////////////////////////////////////////////////////////////////////////////

}
