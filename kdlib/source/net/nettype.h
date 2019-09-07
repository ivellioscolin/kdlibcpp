#pragma once

#include <boost/enable_shared_from_this.hpp>

#include "kdlib/typeinfo.h"
#include "kdlib/exceptions.h"

#include "net/net.h"
#include "net/metadata.h"

#include "typeinfoimp.h"

namespace kdlib {


///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr  getNetTypeById(COR_TYPEID typeId);

///////////////////////////////////////////////////////////////////////////////

class NetTypeInfoBase : public TypeInfo, public boost::enable_shared_from_this<NetTypeInfoBase>
{
protected:

    virtual std::wstring str() {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getName() {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getScopeName() {
        NOT_IMPLEMENTED();
    }

    virtual std::pair<std::wstring, std::wstring> splitName() {
        NOT_IMPLEMENTED();
    }

    virtual size_t getSize() {
        NOT_IMPLEMENTED();
    }

    virtual size_t getPtrSize() {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr ptrTo( size_t ptrSize = 0 ) {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr deref() {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr arrayOf( size_t size ) {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr arrayOf() {
        NOT_IMPLEMENTED();
    }

    virtual bool isArray() {
        NOT_IMPLEMENTED();
    }

    virtual bool isPointer() {
        NOT_IMPLEMENTED();
    }

    virtual bool isVoid() {
        NOT_IMPLEMENTED();
    }

    virtual bool isBase() {
        NOT_IMPLEMENTED();
    }

    virtual bool isUserDefined() {
        NOT_IMPLEMENTED();
    }

    virtual bool isConstant() {
        NOT_IMPLEMENTED();
    }

    virtual bool isEnum() {
        NOT_IMPLEMENTED();
    }

    virtual bool isBitField() {
        NOT_IMPLEMENTED();
    }

    virtual bool isFunction() {
        NOT_IMPLEMENTED();
    }

    virtual bool isVtbl() {
        NOT_IMPLEMENTED();
    }

    virtual bool isNoType() {
        NOT_IMPLEMENTED();
    }

    virtual bool isTemplate() {
        return false;
    }

    virtual BITOFFSET getBitOffset() {
        NOT_IMPLEMENTED();
    }

    virtual BITOFFSET getBitWidth() {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr getBitType() {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr getElement( const std::wstring &name ) {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr getElement( size_t index ) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_REL getElementOffset( const std::wstring &name ) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_REL getElementOffset( size_t index ) {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getElementName( size_t index ) {
        NOT_IMPLEMENTED();
    }

    virtual size_t getElementIndex( const std::wstring &name ) {
        NOT_IMPLEMENTED();
    }

    virtual size_t getElementCount() {
        NOT_IMPLEMENTED();
    }

    virtual  MEMOFFSET_64 getElementVa( const std::wstring &name ) {
        NOT_IMPLEMENTED();
    }

    virtual  MEMOFFSET_64 getElementVa( size_t index ) {
        NOT_IMPLEMENTED();
    }
    
    virtual bool isStaticMember( const std::wstring &name ) {
        NOT_IMPLEMENTED();
    }

    virtual bool isStaticMember( size_t index ) {
        NOT_IMPLEMENTED();
    }

    virtual bool isVirtualMember( const std::wstring &name ) {
        NOT_IMPLEMENTED();
    }

    virtual bool isVirtualMember( size_t index ) {
        NOT_IMPLEMENTED();
    }

    virtual bool isConstMember(const std::wstring &name) {
        NOT_IMPLEMENTED();
    }

    virtual bool isConstMember(size_t index) {
        NOT_IMPLEMENTED();
    }

    virtual bool isVirtual() {
        NOT_IMPLEMENTED();
    }

    virtual bool isIncomplete() {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr getMethod( const std::wstring &name, const std::wstring&  prototype = L"") {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr getMethod( const std::wstring &name, TypeInfoPtr prototype) {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr getMethod( size_t index ) {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getMethodName(size_t index) {
        NOT_IMPLEMENTED();
    }

    virtual size_t getMethodsCount() {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr getBaseClass( const std::wstring& className) {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr getBaseClass( size_t index ) {
        NOT_IMPLEMENTED();
    }

    virtual size_t getBaseClassesCount() {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_REL getBaseClassOffset( const std::wstring &name ) {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_REL getBaseClassOffset( size_t index ) {
        NOT_IMPLEMENTED();
    }

    virtual bool isBaseClassVirtual( const std::wstring &name ) {
        NOT_IMPLEMENTED();
    }

    virtual bool isBaseClassVirtual( size_t index ) {
        NOT_IMPLEMENTED();
    }

    virtual void getBaseClassVirtualDisplacement( const std::wstring &name, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize )  {
        NOT_IMPLEMENTED();
    }

    virtual void getBaseClassVirtualDisplacement( size_t index, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize ) {
        NOT_IMPLEMENTED();
    }
    
    virtual NumVariant getValue() const {
        NOT_IMPLEMENTED();
    }

    virtual void getVirtualDisplacement( const std::wstring& fieldName, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize ) {
        NOT_IMPLEMENTED();
    }

    virtual void getVirtualDisplacement( size_t fieldIndex, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize ) {
        NOT_IMPLEMENTED();
    }

    virtual CallingConventionType getCallingConvention() {
        NOT_IMPLEMENTED();
    }

    virtual bool hasThis() {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr getReturnType() {
        NOT_IMPLEMENTED();
    }

    virtual void appendField(const std::wstring &fieldName, const TypeInfoPtr &fieldType ) {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr getClassParent() {
        NOT_IMPLEMENTED();
    }

    virtual size_t getAlignReq() {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr getVTBL() {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_REL getVtblOffset() {
        NOT_IMPLEMENTED();
    }

    virtual TypedVarPtr getVar(const DataAccessorPtr &dataSource) {
        NOT_IMPLEMENTED();
    }

    size_t getTemplateArgsCount() override
    {
        NOT_IMPLEMENTED();
    }

    std::wstring getTemplateArg(size_t index) override
    {
        NOT_IMPLEMENTED();
    }
};

///////////////////////////////////////////////////////////////////////////////

class NetTypeField : public TypeField
{
public:

    NetTypeField(const COR_FIELD&  corField, MetaDataProviderPtr& metaProvider) :
        TypeField( metaProvider->getFieldName(corField.token)),
        m_metaProvider(metaProvider),
        m_fieldType(corField.fieldType),
        m_token(corField.token),
        m_typeId(corField.id)
    {
        m_offset = corField.offset;
    }

private:

    TypeInfoPtr getTypeInfo();

private:

    MetaDataProviderPtr  m_metaProvider;

    CorElementType  m_fieldType;
    mdFieldDef m_token;
    COR_TYPEID m_typeId;
};

///////////////////////////////////////////////////////////////////////////////

class NetTypeClass : public TypeInfoFields
{
public:

    static NetTypeClass*  createById(COR_TYPEID typeId);

public:

    virtual std::wstring getName() {
        return m_name;
    }

    virtual std::wstring str();

    virtual size_t getSize();

private:

    NetTypeClass(COR_TYPEID& typeId, MetaDataProviderPtr& metaProvider, const std::wstring& name, mdTypeDef token) :
        TypeInfoFields(name),
        m_typeId(typeId),
        m_metaProvider(metaProvider),
        m_name(name),
        m_token(token)
    {}

    virtual void getFields();

    MetaDataProviderPtr  m_metaProvider;
    std::wstring  m_name;
    mdTypeDef  m_token;
    COR_TYPEID  m_typeId;
};

///////////////////////////////////////////////////////////////////////////////

class NetTypeArray : public NetTypeInfoBase
{

public:

    NetTypeArray(const COR_ARRAY_LAYOUT&  arrayLayout ) :
        m_arrayLayout(arrayLayout)
    {}


    virtual std::wstring getName() {
        return getElementType()->getName() + L"[]";
    }

    virtual bool isArray() 
    {
        return true;
    }

    virtual size_t getSize() 
    {
        return ptrSize();
    }


private:

    COR_ARRAY_LAYOUT  m_arrayLayout;

    TypeInfoPtr getElementType();

};

///////////////////////////////////////////////////////////////////////////////

class NetTypeString : public NetTypeInfoBase
{
    virtual std::wstring getName() {
        return  L"String";
    }
};

///////////////////////////////////////////////////////////////////////////////

}
