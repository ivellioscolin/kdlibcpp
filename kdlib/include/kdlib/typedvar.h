#pragma once

#include <vector>
#include <map>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "kdlib/typeinfo.h"
#include "kdlib/variant.h"
#include "kdlib/dataaccessor.h"
#include "kdlib/exceptions.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

typedef std::vector<TypedVarPtr> TypedVarList;

class TypedValue;
typedef std::vector<TypedValue>  TypedValueList;

class StackFrame;
typedef boost::shared_ptr<StackFrame>  StackFramePtr;

class Scope;
typedef boost::shared_ptr<Scope>  ScopePtr;

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr loadTypedVar( const std::wstring &varName );

TypedVarPtr loadTypedVar( const std::wstring &typeName, MEMOFFSET_64 addr );

TypedVarPtr loadTypedVar( const TypeInfoPtr &typeInfo, MEMOFFSET_64 addr );

TypedVarPtr loadTypedVar( SymbolPtr &symbol );

TypedVarPtr loadTypedVar( const std::wstring &typeName, const DataAccessorPtr& dataSource);

TypedVarPtr loadTypedVar( const TypeInfoPtr &typeInfo, const DataAccessorPtr& dataSource);

TypedVarPtr loadTypedVar( const std::wstring &funcName, const std::wstring &prototype);

TypedVarPtr containingRecord( MEMOFFSET_64 addr, const std::wstring &typeName, const std::wstring &fieldName );

TypedVarPtr containingRecord( MEMOFFSET_64 addr, TypeInfoPtr &typeInfo, const std::wstring &fieldName );

TypedVarList loadTypedVarList( MEMOFFSET_64 addr, const std::wstring &typeName, const std::wstring &fieldName );

TypedVarList loadTypedVarList( MEMOFFSET_64 addr, TypeInfoPtr &typeInfo, const std::wstring &fieldName );

TypedVarList loadTypedVarArray( MEMOFFSET_64 addr, const std::wstring &typeName, size_t count );

TypedVarList loadTypedVarArray( MEMOFFSET_64 addr, TypeInfoPtr &typeInfo, size_t count );

TypedVarPtr loadCharVar( char var );
TypedVarPtr loadShortVar( short var );
TypedVarPtr loadLongVar( long var );
TypedVarPtr loadLongLongVar( long long var );

TypedVarPtr loadUCharVar( unsigned char var );
TypedVarPtr loadUShortVar( unsigned short var );
TypedVarPtr loadULongVar( unsigned long var );
TypedVarPtr loadULongLongVar( unsigned long long var );

TypedVarPtr loadIntVar( int var );
TypedVarPtr loadUIntVar( unsigned int var );
TypedVarPtr loadBoolVar( bool var );
TypedVarPtr loadFloatVar( float var );
TypedVarPtr loadDoubleVar( double var );
TypedVarPtr loadWCharVar( wchar_t var );

class TypedVar : public NumConvertable, private boost::noncopyable {

    friend TypedVarPtr loadTypedVar( const SymbolPtr &symbol );

    friend TypedVarPtr loadTypedVar( const std::wstring &varName );

    friend TypedVarPtr loadTypedVar( const std::wstring &typeName, MEMOFFSET_64 addr );

    friend TypedVarPtr loadTypedVar( const TypeInfoPtr &typeInfo, MEMOFFSET_64 addr );

    friend TypedVarPtr loadTypedVar( const std::wstring &typeName, const DataAccessorPtr& dataSource );

    friend TypedVarPtr loadTypedVar( const TypeInfoPtr &typeInfo, const     DataAccessorPtr& dataSource );

    friend TypedVarPtr loadTypedVar( const std::wstring &funcName, const std::wstring &prototype);

public:
    
    template <typename T>
    explicit operator T() {
        return static_cast<T>(getValue());
    }

    template <typename T>
    explicit operator T() const {
        return static_cast<T>(getValue());
    }

    virtual std::wstring str() = 0;
    virtual VarStorage getStorage() const = 0;
    virtual std::wstring  getRegisterName() const = 0;
    virtual MEMOFFSET_64 getAddress() const = 0;
    virtual MEMOFFSET_64 getDebugStart() const = 0;
    virtual MEMOFFSET_64 getDebugEnd() const = 0;
    virtual size_t getSize() const = 0;
    virtual std::wstring getName() const = 0;
    virtual TypedVarPtr getElement( const std::wstring& fieldName ) = 0;
    virtual TypedVarPtr getElement( size_t index ) = 0;
    virtual VarStorage getElementStorage(const std::wstring& fieldName) = 0;
    virtual VarStorage getElementStorage(size_t index ) = 0;
    virtual MEMOFFSET_REL getElementOffset(const std::wstring& fieldName ) = 0;
    virtual MEMOFFSET_REL getElementOffset(size_t index ) = 0;
    virtual RELREG_ID getElementOffsetRelativeReg(const std::wstring& fieldName ) = 0;
    virtual RELREG_ID getElementOffsetRelativeReg(size_t index ) = 0;
    virtual unsigned long getElementReg(const std::wstring& fieldName) = 0;
    virtual unsigned long getElementReg(size_t index) = 0;
    virtual size_t getElementCount() = 0;
    virtual std::wstring getElementName( size_t index ) = 0;
    virtual size_t getElementIndex(const std::wstring&  elementName) = 0;
    virtual TypedVarPtr getMethod( const std::wstring &name, const std::wstring&  prototype = L"") = 0;
    virtual TypedVarPtr getMethod( const std::wstring &name, TypeInfoPtr prototype) = 0;
    virtual TypeInfoPtr getType() const = 0;
    virtual void setValue(const NumVariant& value) = 0;
    virtual std::wstring  getStrValue() const = 0;
    virtual void setStrValue(const std::wstring& value) = 0;
    virtual std::wstring printValue() const = 0;
    virtual TypedVarPtr deref() = 0;
    virtual TypedVarPtr castTo(const std::wstring& typeName) const = 0;
    virtual TypedVarPtr castTo(const TypeInfoPtr &typeInfo) const = 0;
    virtual void writeBytes(const DataAccessorPtr& stream, size_t pos = 0) const = 0;
    virtual TypedValue call(const TypedValueList& arglst) = 0;
    virtual void setElement( const std::wstring& fieldName, const TypedValue& value) = 0;
    virtual void setElement( size_t index, const TypedValue& value ) = 0;
    virtual TypedVarList getInlineFunctions(MEMOFFSET_64 offset) = 0;
    virtual void getSourceLine(MEMOFFSET_64 offset, std::wstring& fileName, unsigned long& lineno) = 0;

protected:

    TypedVar() 
    {}

    virtual ~TypedVar() 
    {}
};


///////////////////////////////////////////////////////////////////////////////

class TypedValue  : public NumConvertable
{
public:

    TypedValue();

    TypedValue(const TypedVarPtr& var) : m_value(var){}
    TypedValue(const NumVariant& var );

    TypedValue( char var ) : m_value( loadCharVar(var) ) {}
    TypedValue( short var) : m_value( loadShortVar(var) ) {}
    TypedValue( long var ) : m_value( loadLongVar(var) ) {}
    TypedValue( long long var ) : m_value( loadLongLongVar(var) ) {}

    TypedValue( unsigned char var ) : m_value( loadUCharVar(var) ) {}
    TypedValue( unsigned short var) : m_value( loadUShortVar(var) ) {}
    TypedValue( unsigned long var ) : m_value( loadULongVar(var) ) {}
    TypedValue( unsigned long long var ) : m_value( loadULongLongVar(var) ) {}

    TypedValue( int var ) : m_value( loadIntVar(var) ) {}
    TypedValue( unsigned int var ) : m_value( loadIntVar(var) ) {}

    TypedValue( float var ) : m_value( loadFloatVar(var) ) {}
    TypedValue( double var ) : m_value( loadDoubleVar(var) ) {}
    TypedValue( wchar_t var ) : m_value( loadWCharVar(var) ) {}
    TypedValue( bool var ) : m_value( loadBoolVar(var) ) {}

    template<typename T>
    TypedValue(T* var) : m_value(loadULongLongVar((unsigned long long)var)) {}

    template<typename T, typename std::enable_if < !std::is_arithmetic<T>::value && std::is_trivially_copyable<T>::value, T >::type* = nullptr>
    explicit operator T () const
    {
        auto name = typeid(T).name();

        if ( sizeof(T) > getSize() )
           throw TypeException( L"cannot cast TypedValue");

        DataAccessorPtr dataRange = getCacheAccessor( m_value->getSize() );
        m_value->writeBytes(dataRange);

        std::vector<unsigned char>  buf(m_value->getSize());
        dataRange->readBytes(buf, m_value->getSize());

        return *reinterpret_cast<T*>(&buf.front());
    }

    template<typename T, typename std::enable_if < std::is_arithmetic<T>::value, T >::type* = nullptr>
    explicit operator T() const
    {
        return static_cast<T>(getValue());
    }

    const TypedVarPtr& get()
    {
        return m_value;
    }

public:

    std::wstring str() {
        return m_value->str();
    }

    VarStorage getStorage() const {
        return m_value->getStorage();
    }

    std::wstring  getRegisterName() const {
        return m_value->getRegisterName();
    }

    MEMOFFSET_64 getAddress() const {
        return m_value->getAddress();
    }

    MEMOFFSET_64 getDebugStart() const {
        return m_value->getDebugStart();
    }

    MEMOFFSET_64 getDebugEnd() const {
        return m_value->getDebugEnd();
    }

    size_t getSize() const {
        return m_value->getSize();
    }

    std::wstring getName() const {
        return m_value->getName();
    }

    TypedVarPtr getElement( const std::wstring& fieldName ) {
        return m_value->getElement(fieldName);
    }

    TypedVarPtr getElement( size_t index ) {
        return m_value->getElement(index);
    }

    VarStorage getElementStorage(const std::wstring& fieldName) {
        return m_value->getElementStorage(fieldName);
    }

    VarStorage getElementStorage(size_t index ) {
        return m_value->getElementStorage(index);
    }

    MEMOFFSET_REL getElementOffset(const std::wstring& fieldName ) {
        return m_value->getElementOffset(fieldName);
    }

    MEMOFFSET_REL getElementOffset(size_t index ) {
        return m_value->getElementOffset(index);
    }

    RELREG_ID getElementOffsetRelativeReg(const std::wstring& fieldName ) {
        return m_value->getElementOffsetRelativeReg(fieldName);
    }

    RELREG_ID getElementOffsetRelativeReg(size_t index ) {
        return m_value->getElementOffsetRelativeReg(index);
    }

    unsigned long getElementReg(const std::wstring& fieldName) {
        return m_value->getElementReg(fieldName);
    }

    unsigned long getElementReg(size_t index) {
        return m_value->getElementReg(index);
    }

    size_t getElementCount() {
        return m_value->getElementCount();
    }

    std::wstring getElementName( size_t index ) {
        return m_value->getElementName(index);
    }

    size_t getElementIndex(const std::wstring&  elementName) {
        return m_value->getElementIndex(elementName);
    }

    TypeInfoPtr getType() const {
        return m_value->getType();
    }

    NumVariant getValue() const {
        return m_value->getValue();
    }

    TypedVarPtr deref() {
        return m_value->deref();
    }

    TypedVarPtr castTo(const std::wstring& typeName) const {
        return m_value->castTo(typeName);
    }

    TypedVarPtr castTo(const TypeInfoPtr &typeInfo) const {
        return m_value->castTo(typeInfo);
    }

    void writeBytes(const DataAccessorPtr& stream, size_t pos = 0) const {
        return m_value->writeBytes(stream, pos);
    }

    TypedValue call(const TypedValueList& arglst) {
        return m_value->call(arglst);
    }
    
private:
    TypedVarPtr  m_value;
};


template<typename T>
TypedValue makeArrayValue (const std::vector<T>& data);


template <typename T>
TypedValue makeValue(const std::wstring &typeName, const T& copyVar) {
    return loadTypedVar(typeName, getCacheAccessor(copyVar));
}

template <typename T>
TypedValue makeValue(const TypeInfoPtr &typeInfo, const T& copyVar) {
    return loadTypedVar(typeInfo, getCacheAccessor(copyVar));
}

inline
TypedValue makePointer(const TypeInfoPtr &typeInfo, const MEMOFFSET_64&  addr)
{
    return TypedValue(addr).castTo(typeInfo->ptrTo());
}

inline
TypedValue makePointer(const std::wstring &typeName, const MEMOFFSET_64&  addr)
{
    return TypedValue(addr).castTo(loadType(typeName)->ptrTo());
}

///////////////////////////////////////////////////////////////////////////////

class Scope
{
public:

    virtual TypedValue get(const std::wstring& varName) const = 0;

    virtual bool find(const std::wstring& varName, TypedValue& value) const = 0;

public:

    TypedValue  operator[] (const std::wstring&  varName) {
        return get(varName);
    }
};

class ScopeList : public Scope
{
public:

    ScopeList()
    {}

    explicit ScopeList(const std::list < std::pair<std::wstring, TypedValue> >& varList)
    {
        m_scope.insert(varList.begin(), varList.end());
    }

    virtual TypedValue get(const std::wstring& varName) const override
    {
        auto  var = m_scope.find(varName);
        if (var == m_scope.end())
            throw DbgException("scope doesn't contain varibale");
        return var->second;
    }

    virtual bool find(const std::wstring& varName, TypedValue& value) const override
    {
        auto  var = m_scope.find(varName);
        if (var == m_scope.end())
            return false;
        value = var->second;
        return true;
    }

private:
    std::map<std::wstring, TypedValue>  m_scope;
};

inline ScopePtr makeScope(const std::list < std::pair<std::wstring, TypedValue> >& varList)
{
   return ScopePtr(new ScopeList(varList));
}

ScopePtr getDefaultScope();

///////////////////////////////////////////////////////////////////////////////

TypedValue callRaw(MEMOFFSET_64 addr, CallingConventionType callingConvention, const TypedValueList& arglst);

TypedValue evalExpr(
    const std::wstring& expr,
    const ScopePtr& scope = getDefaultScope(),
    const TypeInfoProviderPtr& typeInfoProvider = getDefaultTypeInfoProvider());

TypedValue evalExpr(
    const std::string& expr,
    const ScopePtr& scope = getDefaultScope(),
    const TypeInfoProviderPtr& typeInfoProvider = getDefaultTypeInfoProvider());

///////////////////////////////////////////////////////////////////////////////

} // end kdlib namespace
