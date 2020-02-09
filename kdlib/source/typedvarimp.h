#pragma once

#include <kdlib/typedvar.h>
#include <kdlib/exceptions.h>
#include <kdlib/memaccess.h>

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class TypedVarImp : public TypedVar
{

protected:


    std::wstring str() override
    {
        NOT_IMPLEMENTED();
    }


    NumVariant getValue() const override
    {
        NOT_IMPLEMENTED();
    }

    void setValue(const NumVariant& value) override
    {
        throw TypeException( m_typeInfo->getName(), L" failed to set value");
    }

    std::wstring  getStrValue() const override
    {
        NOT_IMPLEMENTED();
    }

    void setStrValue(const std::wstring& value) override
    {
        NOT_IMPLEMENTED();
    }

    std::wstring printValue() const override
    {
        NOT_IMPLEMENTED();
    }

    VarStorage getStorage() const override
    {
        return m_varData->getStorageType();
    }

    std::wstring getRegisterName() const override
    {
        return m_varData->getRegisterName();
    }

    MEMOFFSET_64 getAddress() const override
    {
        return m_varData->getAddress();
    }

    MEMOFFSET_64 getDebugStart() const override
    {
        throw TypeException( L"is not a function");
    }

    MEMOFFSET_64 getDebugEnd() const override
    {
        throw TypeException( L"is not a function");
    }

    size_t getSize() const override
    {
        return m_typeInfo->getSize();
    }

    std::wstring getName() const override
    {
        if (m_name.empty() )
            throw TypeException( m_typeInfo->getName(), L" can not get variable name");
        return m_name;
    }

    TypeInfoPtr getType() const override
    {
       return m_typeInfo;
    }

    TypedVarPtr deref() override
    {
        throw TypeException( m_typeInfo->getName(), L"object can not be derefernced" );
    }

    TypedVarPtr getElement( const std::wstring& fieldName ) override
    {
        throw TypeException( m_typeInfo->getName(), L" type has no named fields");
    }

    TypedVarPtr getElement( size_t index ) override
    {
        throw TypeException( m_typeInfo->getName(), L" type has no fields or array elements");
    }

    MEMOFFSET_REL getElementOffset( const std::wstring& fieldName ) override
    {
        throw TypeException( m_typeInfo->getName(), L" type has no named fields");
    }

    MEMOFFSET_REL getElementOffset( size_t index ) override
    {
        throw TypeException( m_typeInfo->getName(), L" type has no fields or array elements");
    }

    RELREG_ID getElementOffsetRelativeReg(const std::wstring& fieldName ) override
    {
        throw TypeException( m_typeInfo->getName(), L" type has no named fields");
    }

    RELREG_ID getElementOffsetRelativeReg(size_t index ) override
    {
        throw TypeException( m_typeInfo->getName(), L" type has no fields or array elements");
    }

    VarStorage getElementStorage(const std::wstring& fieldName) override
    {
        throw TypeException(m_typeInfo->getName(), L" not implemented for this type");
    }

    VarStorage getElementStorage(size_t index) override
    {
        throw TypeException(m_typeInfo->getName(), L" not implemented for this type");
    }

    size_t getElementCount() override
    {
        throw TypeException( m_typeInfo->getName(), L" type has no fields or array elements");
    }

    std::wstring getElementName( size_t index ) override
    {
        throw TypeException( m_typeInfo->getName(), L" type has no named fields");
    }

    size_t getElementIndex( const std::wstring&  elementName ) override
    {
        throw TypeException( m_typeInfo->getName(), L" type has no named fields");
    }

    TypedVarPtr getMethod( const std::wstring &name, const std::wstring&  prototype = L"") override
    {
        throw TypeException( m_typeInfo->getName(), L" type has no methods");
    }
    
    TypedVarPtr getMethod( const std::wstring &name, TypeInfoPtr prototype) override
    {
        throw TypeException( m_typeInfo->getName(), L" type has no methods");
    }

    unsigned long getElementReg(const std::wstring& fieldName) override
    {
        NOT_IMPLEMENTED();
    }

    unsigned long getElementReg(size_t index) override
    {
        NOT_IMPLEMENTED();
    }

    TypedVarPtr castTo(const std::wstring& typeName) const override;

    TypedVarPtr castTo(const TypeInfoPtr &typeInfo) const override;

    void writeBytes(const DataAccessorPtr& stream, size_t pos = 0) const override
    {
        std::vector<unsigned char>  buffer( getSize() );
        m_varData->readBytes(buffer, getSize() );
        stream->writeBytes(buffer, pos);
    }

    TypedValue call(const TypedValueList& arglst) override
    {
        throw TypeException( L" is not a function");
    }
    
    void setElement( const std::wstring& fieldName, const TypedValue& value) override
    {
        throw TypeException( m_typeInfo->getName(), L" type has no fields or array elements");
    }

    void setElement( size_t index, const TypedValue& value ) override
    {
        throw TypeException( m_typeInfo->getName(), L" type has no fields or array elements");
    }

    TypedVarList getInlineFunctions(MEMOFFSET_64 offset) override
    {
        throw TypeException(m_typeInfo->getName(), L" type has no inlined functions");
    }

    void getSourceLine(MEMOFFSET_64 offset, std::wstring& fileName, unsigned long& lineno) override
    {
        throw TypeException(m_typeInfo->getName(), L" failed to get source line");
    }

protected:

    TypedVarImp( const TypeInfoPtr& typeInfo, const DataAccessorPtr &dataSource, const std::wstring& name = L"" ) :
        m_typeInfo( typeInfo ),
        m_varData( dataSource ),
        m_name( name )
        {}

    TypeInfoPtr  m_typeInfo;

    DataAccessorPtr  m_varData;

    std::wstring  m_name;
};

///////////////////////////////////////////////////////////////////////////////

class TypedVarBase : public TypedVarImp
{
public:

    TypedVarBase( const TypeInfoPtr& typeInfo, const DataAccessorPtr &dataSource, const std::wstring& name = L"" ) :
        TypedVarImp( typeInfo, dataSource, name )
        {}

    std::wstring printValue() const;

protected:

    virtual NumVariant getValue() const;

    virtual void setValue(const NumVariant& value);


    virtual std::wstring str();
};

///////////////////////////////////////////////////////////////////////////////

class TypedVarUdt :  public TypedVarImp
{
public:

       TypedVarUdt( const TypeInfoPtr& typeInfo, const DataAccessorPtr &dataSource, const std::wstring& name = L"" ) :
        TypedVarImp( typeInfo, dataSource, name )
        {}

protected:

    virtual NumVariant getValue() const {
        return NumVariant( m_varData->getAddress() );
    }

    virtual TypedVarPtr getElement( const std::wstring& fieldName );

    virtual void setElement( const std::wstring& fieldName, const TypedValue& value);

    virtual  TypedVarPtr getElement( size_t index );

    virtual void setElement( size_t index, const TypedValue& value );

    virtual MEMOFFSET_REL getElementOffset( const std::wstring& fieldName ) {
        return m_typeInfo->getElementOffset( fieldName );
    }

    virtual MEMOFFSET_REL getElementOffset( size_t index ) {
        return m_typeInfo->getElementOffset( index );
    }

    virtual size_t getElementCount() {
        return m_typeInfo->getElementCount();
    }

    virtual std::wstring getElementName( size_t index ) {
        return m_typeInfo->getElementName( index );
    }

    size_t getElementIndex(const std::wstring&  elementName) {
        return m_typeInfo->getElementIndex(elementName);
    }

    MEMDISPLACEMENT getVirtualBaseDisplacement( const std::wstring &fieldName );
    MEMDISPLACEMENT getVirtualBaseDisplacement( size_t index );

    virtual std::wstring str();

    virtual TypedVarPtr getMethod( const std::wstring &name, const std::wstring&  prototype = L"");
    
    virtual TypedVarPtr getMethod( const std::wstring &name, TypeInfoPtr prototype)
    {
        NOT_IMPLEMENTED();
    }


protected:

    TypedVarPtr getMethodRecursive( 
        const std::wstring& methodName,
        const std::wstring& methodPrototype,
        TypeInfoPtr&  baseClass,
        MEMOFFSET_REL startOffset
        );

    TypedVarPtr getVirtualMethodRecursive( 
        const std::wstring& methodName, 
        const std::wstring& methodPrototype,
        TypeInfoPtr&  baseClass,
        MEMOFFSET_REL startOffset
        );

    
};


///////////////////////////////////////////////////////////////////////////////

class TypedVarPointer : public TypedVarImp
{
public:

    TypedVarPointer( const TypeInfoPtr& typeInfo, const DataAccessorPtr &dataSource, const std::wstring& name = L"" ) :
        TypedVarImp( typeInfo, dataSource, name )
        {}


public:

    virtual NumVariant getValue() const {
        return NumVariant(getPtrValue());
    }

    virtual void setValue(const NumVariant& value) {
        getSize() == 4 ? m_varData->writeDWord(value.asULong()) : m_varData->writeQWord(value.asULongLong());
    }

    virtual TypedVarPtr deref();

    virtual TypedVarPtr getElement( const std::wstring& fieldName ) {
        TypedVarPtr  derefPtr = deref();
        return derefPtr->getElement( fieldName );
    }

    virtual  TypedVarPtr getElement(size_t index);

    virtual std::wstring str();


    std::wstring printValue() const;

private:

    MEMOFFSET_64  getPtrValue() const {
        return addr64(getSize() == 4 ? m_varData->readDWord() : m_varData->readQWord());
    }
};

///////////////////////////////////////////////////////////////////////////////

class TypedVarArray : public TypedVarImp
{
public:

    TypedVarArray( const TypeInfoPtr& typeInfo, const DataAccessorPtr &dataSource, const std::wstring& name = L"") :
        TypedVarImp( typeInfo, dataSource, name )
        {}

    virtual NumVariant getValue() const {
        return NumVariant(m_varData->getAddress() );
    }

    virtual void setElement( size_t index, const TypedValue& value ) {
        getElement(index)->setValue(value);
    }

    virtual size_t getElementCount() {
        return m_typeInfo->getElementCount();
    }

    virtual  TypedVarPtr getElement( size_t index );

    virtual std::wstring str();
};

///////////////////////////////////////////////////////////////////////////////

class TypedVarBitField : public TypedVarImp
{
public:
    TypedVarBitField( const TypeInfoPtr& typeInfo, const DataAccessorPtr &dataSource, const std::wstring& name = L"" ) :
      TypedVarImp( typeInfo, dataSource, name)
      {}

    virtual NumVariant getValue() const;

    virtual void setValue(const NumVariant& value);

    virtual std::wstring str();

    std::wstring printValue() const;
};

///////////////////////////////////////////////////////////////////////////////

class TypedVarEnum : public TypedVarImp
{
public:
    TypedVarEnum( const TypeInfoPtr& typeInfo, const DataAccessorPtr &dataSource, const std::wstring& name = L"" ) :
      TypedVarImp( typeInfo, dataSource, name )
      {}

    virtual NumVariant getValue() const {
        return NumVariant( m_varData->readDWord() );
    }

    virtual std::wstring str();

    std::wstring printValue() const;
};

///////////////////////////////////////////////////////////////////////////////

class TypedVarFunction : public TypedVarImp
{
public:

    TypedVarFunction( const TypeInfoPtr& typeInfo, const DataAccessorPtr &dataSource, const std::wstring& name = L"" ) :
        TypedVarImp( typeInfo, dataSource, name )
    {}

    virtual NumVariant getValue() const {
        return NumVariant( getAddress() );
    }

    virtual size_t getElementCount() {
        return m_typeInfo->getElementCount();
    }

    virtual MEMOFFSET_64 getDebugStart() const  {
        return 0;
    }

    virtual MEMOFFSET_64 getDebugEnd() const  {
        return 0;
    }

    virtual size_t getSize() const  {
        return 0;
    }

    virtual std::wstring str();

    virtual TypedValue call(const TypedValueList& arglst);

protected:

    TypedValueList castArgs(const TypedValueList& arglst);

    TypedValue callCdecl(const TypedValueList& arglst);
    TypedValue callStd(const TypedValueList& arglst);
    TypedValue callFast(const TypedValueList& arglst);
    TypedValue callThis(const TypedValueList& arglst);
    TypedValue callX64(const TypedValueList& arglst);

    void makeCallx86();
    void makeCallx64();
};

///////////////////////////////////////////////////////////////////////////////

class TypedVarVtbl : public TypedVarImp
{
public:

    TypedVarVtbl(const TypeInfoPtr& typeInfo, const DataAccessorPtr &dataSource, const std::wstring& name = L"") :
        TypedVarImp(typeInfo, dataSource, name)
    {}

    virtual NumVariant getValue() const 
    {
        return NumVariant(getAddress());
    }

    virtual size_t getElementCount()
    {
        return m_typeInfo->getElementCount();
    }

    virtual TypedVarPtr getElement(size_t index);

    virtual std::wstring str(); 
};

//////////////////////////////////////////////////////////////////////////////

class TypedVarVoid : public TypedVar
{
public:

    TypedVarVoid()
    {}

    std::wstring str() override
    {
       throw TypeException(L"Not applicable for Void");
    }

    NumVariant getValue() const override
    {
       throw TypeException(L"Not applicable for Void");
    } 

    void setValue(const NumVariant& value) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    std::wstring  getStrValue() const override
    {
        throw TypeException(L"Not applicable for Void");
    }

    void setStrValue(const std::wstring& value) 
    {
        throw TypeException(L"Not applicable for Void");
    }

    std::wstring printValue() const override
    {
        throw TypeException(L"Not applicable for Void");
    }

    VarStorage getStorage() const override
    {
       throw TypeException(L"Not applicable for Void");
    }

    std::wstring getRegisterName() const override
    {
       throw TypeException(L"Not applicable for Void");
    }

    MEMOFFSET_64 getAddress() const override
    {
       throw TypeException(L"Not applicable for Void");
    }

    MEMOFFSET_64 getDebugStart() const override
    {
       throw TypeException(L"Not applicable for Void");
    }

    MEMOFFSET_64 getDebugEnd() const override
    {
       throw TypeException(L"Not applicable for Void");
    }

    size_t getSize() const override
    {
       throw TypeException(L"Not applicable for Void");
    }

    std::wstring getName() const override
    {
       throw TypeException(L"Not applicable for Void");
    }

    TypeInfoPtr getType() const override;

    TypedVarPtr deref() override
    {
       throw TypeException(L"Not applicable for Void");
    }

    TypedVarPtr getElement( const std::wstring& fieldName ) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    TypedVarPtr getElement( size_t index ) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    MEMOFFSET_REL getElementOffset( const std::wstring& fieldName )  override
    {
       throw TypeException(L"Not applicable for Void");
    }

    MEMOFFSET_REL getElementOffset( size_t index ) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    RELREG_ID getElementOffsetRelativeReg(const std::wstring& fieldName ) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    RELREG_ID getElementOffsetRelativeReg(size_t index ) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    VarStorage getElementStorage(const std::wstring& fieldName) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    VarStorage getElementStorage(size_t index) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    size_t getElementCount() override
     {
       throw TypeException(L"Not applicable for Void");
    }

    std::wstring getElementName( size_t index ) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    size_t getElementIndex( const std::wstring&  elementName ) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    TypedVarPtr getMethod( const std::wstring &name, const std::wstring&  prototype = L"") override
    {
       throw TypeException(L"Not applicable for Void");
    }
    
    TypedVarPtr getMethod( const std::wstring &name, TypeInfoPtr prototype) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    unsigned long getElementReg(const std::wstring& fieldName) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    unsigned long getElementReg(size_t index) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    TypedVarPtr castTo(const std::wstring& typeName) const override
    {
       throw TypeException(L"Not applicable for Void");
    }

    TypedVarPtr castTo(const TypeInfoPtr &typeInfo) const override
    {
       throw TypeException(L"Not applicable for Void");
    }

    void writeBytes(const DataAccessorPtr& stream, size_t bytes = 0) const override
    {
       throw TypeException(L"Not applicable for Void");
    }

    TypedValue call(const TypedValueList& arglst) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    void setElement( const std::wstring& fieldName, const TypedValue& value) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    void setElement( size_t index, const TypedValue& value ) override
    {
       throw TypeException(L"Not applicable for Void");
    }

    TypedVarList getInlineFunctions(MEMOFFSET_64 offset) override
    {
        throw TypeException(L"Not applicable for Void");
    }

    void getSourceLine(MEMOFFSET_64 offset, std::wstring& fileName, unsigned long& lineno) override
    {
        throw TypeException(L"Not applicable for Void");
    }
};

//////////////////////////////////////////////////////////////////////////////

class SymbolFunction : public TypedVarFunction
{
public:

    SymbolFunction( const SymbolPtr& symbol );

    MEMOFFSET_64 getDebugStart() const override {
        return getDebugXImpl(SymTagFuncDebugStart);
    }

    MEMOFFSET_64 getDebugEnd() const override {
        return getDebugXImpl(SymTagFuncDebugEnd);
    }

    MEMOFFSET_REL getElementOffset( size_t index ) override;

    RELREG_ID getElementOffsetRelativeReg(size_t index ) override;

    MEMOFFSET_REL getElementOffset( const std::wstring& paramName ) override;
 
    RELREG_ID getElementOffsetRelativeReg(const std::wstring& paramName ) override;

    unsigned long getElementReg(const std::wstring& fieldName) override;

    unsigned long getElementReg(size_t index) override;

    size_t getElementIndex(const std::wstring& paramName ) override;

    std::wstring getElementName( size_t index ) override;

    VarStorage getElementStorage(const std::wstring& fieldName) override;

    VarStorage getElementStorage(size_t index) override;

    size_t getSize() const override {
        return m_symbol->getSize();
    }

    TypedVarList getInlineFunctions(MEMOFFSET_64 offset) override;

    void getSourceLine(MEMOFFSET_64 offset, std::wstring& fileName, unsigned long& lineno) override;

protected:

    SymbolPtr getChildSymbol(size_t index);

    MEMOFFSET_64 getDebugXImpl(SymTags symTag) const;

    SymbolPtr  m_symbol;
};


///////////////////////////////////////////////////////////////////////////////


class TypedVarMethodBound : public TypedVarFunction
{

public:

    TypedVarMethodBound(
        const TypeInfoPtr& prototype,
        const DataAccessorPtr &dataSource, 
        const std::wstring& name, 
        MEMOFFSET_64 thisValue);

     virtual TypedValue call(const TypedValueList& arglst);

private:

    MEMOFFSET_64   m_this;

};

///////////////////////////////////////////////////////////////////////////////

class TypedVarMethodUnbound : public TypedVarFunction
{
public:

    TypedVarMethodUnbound(const TypeInfoPtr& typeInfo, const DataAccessorPtr &dataSource, const std::wstring& name):
       TypedVarFunction(typeInfo, dataSource, name )
    {}
};


///////////////////////////////////////////////////////////////////////////////

class SymbolInlineFunction : public TypedVarImp
{
public:

    SymbolInlineFunction(const SymbolPtr& symbol);

    void getSourceLine(MEMOFFSET_64 offset, std::wstring& fileName, unsigned long& lineno) override
    {
        m_symbol->getInlineSourceLine(offset, fileName, lineno);
    }

    size_t getSize() const override
    {
        NOT_IMPLEMENTED();
    }

private:
    SymbolPtr  m_symbol;

};

///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib
