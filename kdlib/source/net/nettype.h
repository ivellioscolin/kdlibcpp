#pragma once

#include "kdlib/typeinfo.h"
#include "kdlib/exceptions.h"

#include "net/net.h"
#include "net/metadata.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr  getTypeById(COR_TYPEID typeId);

///////////////////////////////////////////////////////////////////////////////

class NetTypeInfoBase : public TypeInfo
{
protected:

    virtual std::wstring str() {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getName() {
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

    virtual bool isVirtual() {
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

    virtual void appendField(const std::wstring &fieldName, TypeInfoPtr &fieldType ) {
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
};

///////////////////////////////////////////////////////////////////////////////

class NetTypeClass : public NetTypeInfoBase
{
public:

    NetTypeClass(MetaDataProviderPtr& metaProvider, const std::wstring& name, mdTypeDef token) :
        m_metaProvider(metaProvider),
        m_name(name),
        m_token(token)
    {}

public:

    virtual std::wstring getName() {
        return m_name;
    }

private:

    MetaDataProviderPtr  m_metaProvider;
    std::wstring  m_name;
    mdTypeDef  m_token;
};

///////////////////////////////////////////////////////////////////////////////

class NetTypeArray : public NetTypeInfoBase
{
public:

    NetTypeArray(const TypeInfoPtr& derefType) :
        m_derefType(derefType)
        {}

public:

    virtual std::wstring getName() {
        return m_derefType->getName() + L"[]";
    }

    virtual bool isArray() 
    {
        return true;
    }


private:

    TypeInfoPtr  m_derefType;
};

///////////////////////////////////////////////////////////////////////////////

}
