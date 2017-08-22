#pragma once

//#include <atlbase.h>
//
//#include "net.h"
//
//#include "kdlib/typedvar.h"
//#include "kdlib/exceptions.h"
//
/////////////////////////////////////////////////////////////////////////////////
//
//namespace kdlib {
//
/////////////////////////////////////////////////////////////////////////////////
//
//class NetObject : public TypedVar
//{
//public:
//
//    NetObject(IXCLRDataValue* value) :
//        m_clrDataValue(value)
//    {}
//
//protected:
//
//    std::wstring str() {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual VarStorage getStorage() const {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual std::wstring  getRegisterName() const {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual MEMOFFSET_64 getAddress() const {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual MEMOFFSET_64 getDebugStart() const {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual MEMOFFSET_64 getDebugEnd() const {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual size_t getSize() const {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual std::wstring getName() const {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual TypedVarPtr getElement( const std::wstring& fieldName ) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual TypedVarPtr getElement( size_t index ) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual VarStorage getElementStorage(const std::wstring& fieldName) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual VarStorage getElementStorage(size_t index ) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual MEMOFFSET_REL getElementOffset(const std::wstring& fieldName ) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual MEMOFFSET_REL getElementOffset(size_t index ) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual RELREG_ID getElementOffsetRelativeReg(const std::wstring& fieldName ) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual RELREG_ID getElementOffsetRelativeReg(size_t index ) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual unsigned long getElementReg(const std::wstring& fieldName) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual unsigned long getElementReg(size_t index) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual size_t getElementCount() {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual std::wstring getElementName( size_t index ) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual size_t getElementIndex(const std::wstring&  elementName) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual TypedVarPtr getMethod( const std::wstring &name, const std::wstring&  prototype = L"") {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual TypedVarPtr getMethod( const std::wstring &name, TypeInfoPtr prototype) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual TypeInfoPtr getType() const {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual NumVariant getValue() const {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual void setValue(const NumVariant& value) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual TypedVarPtr deref() {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual TypedVarPtr castTo(const std::wstring& typeName) const {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual TypedVarPtr castTo(const TypeInfoPtr &typeInfo) const {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual void writeBytes(DataAccessorPtr& stream, size_t pos = 0) const {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual TypedValue call(const TypedValueList& arglst) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual void setElement( const std::wstring& fieldName, const TypedValue& value) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//    virtual void setElement( size_t index, const TypedValue& value ) {
//        throw TypeException(L"Not applicable for managed objects");
//    }
//
//protected:
//
//    CComPtr<IXCLRDataValue>  m_clrDataValue;
//};
//
/////////////////////////////////////////////////////////////////////////////////
//
//}
//
/////////////////////////////////////////////////////////////////////////////////
