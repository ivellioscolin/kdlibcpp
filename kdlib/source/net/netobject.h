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

    std::wstring str() override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    VarStorage getStorage() const override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    std::wstring  getRegisterName() const override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    MEMOFFSET_64 getAddress() const override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    MEMOFFSET_64 getDebugStart() const override
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    MEMOFFSET_64 getDebugEnd() const override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    size_t getSize() const override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    std::wstring getName() const override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    TypedVarPtr getElement( const std::wstring& fieldName ) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    TypedVarPtr getElement( size_t index ) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    VarStorage getElementStorage(const std::wstring& fieldName) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    VarStorage getElementStorage(size_t index ) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    MEMOFFSET_REL getElementOffset(const std::wstring& fieldName ) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    MEMOFFSET_REL getElementOffset(size_t index ) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    RELREG_ID getElementOffsetRelativeReg(const std::wstring& fieldName ) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    RELREG_ID getElementOffsetRelativeReg(size_t index ) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    unsigned long getElementReg(const std::wstring& fieldName) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    unsigned long getElementReg(size_t index) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    size_t getElementCount() override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    std::wstring getElementName( size_t index ) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    size_t getElementIndex(const std::wstring&  elementName) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    TypedVarPtr getMethod( const std::wstring &name, const std::wstring&  prototype = L"") override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    TypedVarPtr getMethod( const std::wstring &name, TypeInfoPtr prototype) override
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    TypeInfoPtr getType() const override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    NumVariant getValue() const override
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    void setValue(const NumVariant& value) {
        throw TypeException(L"Not applicable for managed objects");
    }

    std::wstring  getStrValue() const override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    void setStrValue(const std::wstring& value) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    std::wstring printValue() const override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    TypedVarPtr deref() override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    TypedVarPtr castTo(const std::wstring& typeName) const override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    TypedVarPtr castTo(const TypeInfoPtr &typeInfo) const override
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    void writeBytes(const DataAccessorPtr& stream, size_t pos = 0) const override
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    TypedValue call(const TypedValueList& arglst) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    void setElement( const std::wstring& fieldName, const TypedValue& value) override
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    void setElement( size_t index, const TypedValue& value ) override 
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    TypedVarList getInlineFunctions(MEMOFFSET_64 offset) override
    {
        throw TypeException(L"Not applicable for managed objects");
    }

    void getSourceLine(MEMOFFSET_64 offset, std::wstring& fileName, unsigned long& lineno) override
    {
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
