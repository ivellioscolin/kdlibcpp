#pragma once

#include <kdlib/typeinfo.h>
#include <kdlib/exceptions.h>

#include "udtfield.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

size_t getPointerSizeByMachine(const SymbolPtr &symbol);

inline size_t getPtrSizeBySymbol( const SymbolPtr &typeSym )
{
    SymTags symTag = typeSym->getSymTag();
    if ( symTag != SymTagPointerType )
        return getPointerSizeByMachine(typeSym);

    return typeSym->getSize();
}

std::wstring  getMethodPrototype( kdlib::TypeInfoPtr&  methodType );

bool isPrototypeMatch(TypeInfoPtr&  methodType, const std::wstring& methodPrototype);

std::wstring printStructType(TypeInfoPtr& structType);
std::wstring printPointerType(TypeInfoPtr&  ptrType);
std::wstring printEnumType(TypeInfoPtr& enumType);

std::list<std::wstring> getTempalteArgs(const std::wstring& typeName);
std::list<std::string> getTempalteArgs(const std::string& typeName);

///////////////////////////////////////////////////////////////////////////////

class TypeInfoImp : public TypeInfo, public boost::enable_shared_from_this<TypeInfoImp>
{
protected:

    std::wstring str() override
    {
        NOT_IMPLEMENTED();
    }

    std::wstring getName() override
    {
        NOT_IMPLEMENTED();
    }

    std::wstring getScopeName() override
    {
        throw TypeException( getName(), L"type has no scope name" );
    }

    std::pair<std::wstring, std::wstring> splitName() override
    {
        return std::pair<std::wstring, std::wstring>(getName(), std::wstring());
    }

    size_t getSize() override
    {
        NOT_IMPLEMENTED();
    }

    size_t getPtrSize()override
    {
        return ptrSize();
    }

    TypeInfoPtr ptrTo( size_t ptrSize = 0 ) override;

    TypeInfoPtr deref() override
    {
        throw TypeException( getName(), L"type can not be dereffered" );
    }

    TypeInfoPtr arrayOf( size_t size ) override;

    TypeInfoPtr arrayOf() override;

    bool isArray() override
    {
        return false;
    }

    bool isPointer() override
    {
        return false;
    }

    bool isVoid() override
    {
        return false;
    }

    bool isBase() override
    {
        return false;
    }

    bool isUserDefined() override
    {
        return false;
    }

    bool isEnum() override 
    {
        return false;
    }

    bool isBitField() override
    {
        return false;
    }

    bool isFunction() override
    {
        return false;
    }

    bool isVtbl() override
    {
        return false;
    }

    bool isNoType() override
    {
        return false;
    }

    bool isConstant() override
    {
        return m_constant;
    }

    bool isVirtual() override
    {
        throw TypeException( getName(), L"type is not class method" );
    }

    bool isIncomplete() override
    {
        return false;
    }

    bool isTemplate() override;
 
    TypeInfoPtr getElement( const std::wstring &name ) override
    {
         throw TypeException( getName(), L" type has no fields or array elements");
    }

    TypeInfoPtr getElement( size_t index ) override
    {
         throw TypeException( getName(), L" type has no fields or array elements");
    }

    MEMOFFSET_REL getElementOffset( const std::wstring &name ) override
    {
         throw TypeException( getName(), L" type has no fields or array elements");
    }

    MEMOFFSET_REL getElementOffset( size_t index ) override
    {
         throw TypeException( getName(), L" type has no fields or array elements");
    }

    size_t getElementCount() override
    {
        throw TypeException( getName(), L" type has no fields or array elements");
    }

    std::wstring getElementName( size_t index ) override
    {
         NOT_IMPLEMENTED();
    }

    size_t getElementIndex( const std::wstring &name ) override
    {
         NOT_IMPLEMENTED();
    }

    BITOFFSET getBitOffset() override
    {
         throw TypeException( getName(), L" type is not a bit field");
    }

    BITOFFSET getBitWidth() override 
    {
         throw TypeException( getName(), L" type is not a bit field");
    }

    TypeInfoPtr getBitType() override
    {
         throw TypeException( getName(), L" type is not a bit field");
    }
    
    MEMOFFSET_64 getElementVa( const std::wstring &name ) override
    {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    MEMOFFSET_64 getElementVa( size_t index )override
    {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    bool isStaticMember( const std::wstring &name ) override
    {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    bool isStaticMember( size_t index ) override
    {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    bool isVirtualMember( const std::wstring &name ) override
    {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    bool isVirtualMember( size_t index ) override
    {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    bool isConstMember(const std::wstring &name) override
    {
        throw TypeException(getName(), L"type is not a struct");
    }
    
    bool isConstMember(size_t index) override
    {
        throw TypeException(getName(), L"type is not a struct");
    }

    bool isInheritedMember(const std::wstring &name) override
    {
        throw TypeException(getName(), L"type is not a struct");
    }

    bool isInheritedMember(size_t index) override
    {
        throw TypeException(getName(), L"type is not a struct");
    }

    TypeInfoPtr getMethod( const std::wstring &name, const std::wstring&  prototype) override
    {
        throw TypeException( getName(), L"type has no methods" ); 
    }

    TypeInfoPtr getMethod( const std::wstring &name, TypeInfoPtr prototype) override
    {
        throw TypeException( getName(), L"type has no methods" ); 
    }

    TypeInfoPtr getMethod( size_t index ) override
    {
        throw TypeException( getName(), L"type has no methods" ); 
    }

    std::wstring getMethodName(size_t index) override
    {
        throw TypeException(getName(), L"type has no methods");
    }

    size_t getMethodsCount() override
    {
        throw TypeException( getName(), L"type has no methods" ); 
    }

    TypeInfoPtr getBaseClass( const std::wstring& className) override
    {
        throw TypeException( getName(), L"type has no base class" ); 
    }

    TypeInfoPtr getBaseClass( size_t index ) override
    {
        throw TypeException( getName(), L"type has no base class" ); 
    }

    size_t getBaseClassesCount() override
    {
        throw TypeException( getName(), L"type has no base class" ); 
    }

    MEMOFFSET_REL getBaseClassOffset( const std::wstring &name ) override
    {
        throw TypeException( getName(), L"type has no base class" ); 
    }

    MEMOFFSET_REL getBaseClassOffset( size_t index ) override
    {
        throw TypeException( getName(), L"type has no base class" );
    }

    bool isBaseClassVirtual( const std::wstring &name ) override
    {
        throw TypeException( getName(), L"type has no base class" );
    }

    bool isBaseClassVirtual( size_t index ) override
    {
        throw TypeException( getName(), L"type has no base class" );
    }

    void getBaseClassVirtualDisplacement( const std::wstring &name, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize ) override
    {
        throw TypeException( getName(), L"type is not virtual inherited" );
    }

    void getBaseClassVirtualDisplacement( size_t index, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize ) override
    {
        throw TypeException( getName(), L"type is not virtual inherited" );
    }

    void getVirtualDisplacement( const std::wstring& fieldName, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize ) override
    {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    void getVirtualDisplacement( size_t fieldIndex, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize ) override
    {
        throw TypeException( getName(), L"type is not a struct" ); 
    }

    CallingConventionType getCallingConvention() override
    {
        throw TypeException( getName(), L"type is not a function" ); 
    }

    bool hasThis() override
    {
        throw TypeException( getName(), L"type is not a function" ); 
    }

    TypeInfoPtr getReturnType() override
    {
        throw TypeException( getName(), L"type is not a function" ); 
    }

    void appendField(const std::wstring &fieldName, const TypeInfoPtr &fieldType ) override
    {
        throw TypeException( getName(), L"type is not editable" ); 
    }

    TypeInfoPtr getClassParent() override
    {
        throw TypeException( getName(), L"has no class parent" ); 
    }

    size_t getAlignReq() override
    {
        throw TypeException( getName(), L"has no alignment-requirement" );
    }

    TypeInfoPtr getVTBL() override
    {
        throw TypeException( getName(), L"has no VTBL" );
    }

    MEMOFFSET_REL getVtblOffset() override
    {
        throw TypeException( getName(), L"type is not a virtual method" ); 
    }

    TypedVarPtr getVar(const DataAccessorPtr &dataSource) override;

    size_t getTemplateArgsCount() override;

    std::wstring getTemplateArg(size_t index) override;

public:

    void setConstant( const NumVariant& constVal )
    {
        m_constant = true;
        m_constantValue = constVal;
    }

protected:

    TypeInfoImp() :
        m_constant( false )
        {}

    bool   m_constant;

    NumVariant  m_constantValue;

    virtual NumVariant getValue() const 
    {
        if ( !m_constant )
        {
            std::wstring typeName = const_cast<TypeInfoImp&>(*this).getName();
            throw TypeException( typeName, L"this type is not a constant and has not a value" );
        }

        return  m_constantValue;
    }
};


///////////////////////////////////////////////////////////////////////////////

class TypeInfoFields : public TypeInfoImp 
{

protected:
    std::wstring print();

    virtual TypeInfoPtr getElement( const std::wstring &name );

    virtual TypeInfoPtr getElement( size_t index );

    virtual MEMOFFSET_REL getElementOffset( const std::wstring &name );

    virtual MEMOFFSET_REL getElementOffset( size_t index );

    virtual std::wstring getElementName( size_t index );

    virtual size_t getElementIndex( const std::wstring &name );

    virtual std::wstring getName() {
        return m_name;
    }

    virtual size_t getElementCount();

    MEMOFFSET_64 getElementVa( const std::wstring &name ) override;
    MEMOFFSET_64 getElementVa( size_t index ) override;
    bool isStaticMember( const std::wstring &name ) override;
    bool isStaticMember( size_t index ) override;
    bool isVirtualMember( const std::wstring &name ) override;
    bool isVirtualMember( size_t index ) override;
    bool isConstMember(const std::wstring &name) override;
    bool isConstMember(size_t index) override;
    bool isInheritedMember(const std::wstring &name) override;
    bool isInheritedMember(size_t index) override;

    virtual size_t getAlignReq();

protected:

    TypeInfoFields( const std::wstring &name ) :
        m_fields( name ),
        m_name( name ),
        m_fieldsGot(false)
        {}

    FieldCollection  m_fields;

    std::wstring  m_name;

    virtual void getFields() = 0;

private:

    bool m_fieldsGot;

    void checkFields()
    {
        if ( !m_fieldsGot )
        {
            getFields();
            m_fieldsGot = true;
        }
    }

};

///////////////////////////////////////////////////////////////////////////////

class SymbolFields : public TypeInfoFields
{
public:
    SymbolFields(const SymbolPtr &symbol) :
         TypeInfoFields( symbol->getName() ),
         m_symbol(symbol)
    {}

    std::wstring getScopeName() {
        return m_symbol->getScopeName();
    }

protected:

    SymbolPtr  m_symbol;
};


class TypeInfoUdt : public SymbolFields
{
public:

    TypeInfoUdt(const SymbolPtr &symbol) :
        SymbolFields( symbol )
        {}

protected:
    virtual std::wstring str();

    virtual size_t getSize() {
        return  m_symbol->getSize();
    }

    virtual size_t getPtrSize() {
        return getPtrSizeBySymbol( m_symbol );
    }

    virtual bool isUserDefined() {
        return true;
    }

    virtual TypeInfoPtr getClassParent();

    virtual TypeInfoPtr getMethod( const std::wstring &name, const std::wstring&  prototype=L"");
    virtual TypeInfoPtr getMethod( size_t index );
    virtual size_t getMethodsCount();
    virtual std::wstring getMethodName(size_t index);

    virtual TypeInfoPtr getBaseClass( const std::wstring& className);
    virtual TypeInfoPtr getBaseClass( size_t index );
    virtual size_t getBaseClassesCount();

    virtual MEMOFFSET_REL getBaseClassOffset( const std::wstring &name );
    virtual MEMOFFSET_REL getBaseClassOffset( size_t index );

    virtual bool isBaseClassVirtual( const std::wstring &name );
    virtual bool isBaseClassVirtual( size_t index );

    virtual void getBaseClassVirtualDisplacement( const std::wstring &name, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize );
    virtual void getBaseClassVirtualDisplacement( size_t index, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize );

    virtual TypeInfoPtr getVTBL();


protected:

    virtual void getFields();

    virtual void getVirtualDisplacement( const std::wstring& fieldName, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize );

    virtual void getVirtualDisplacement( size_t fieldIndex, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize );

    void getFields( 
        const SymbolPtr &rootSym, 
        const SymbolPtr &baseVirtualSym,
        MEMOFFSET_32 startOffset = 0,
        MEMOFFSET_32 virtualBasePtr = 0,
        size_t virtualDispIndex = 0,
        size_t m_virtualDispSize = 0 );

    void getVirtualFields();

};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoEnum : public SymbolFields 
{
public:

    TypeInfoEnum( const SymbolPtr& symbol ) :
        SymbolFields( symbol )
        {}

    virtual std::wstring str();

    virtual size_t getSize() {
        return  m_symbol->getSize();
    }

    virtual bool isEnum() {
        return true;
    }

    virtual TypeInfoPtr getClassParent();

protected:

    virtual size_t getPtrSize() {
        return getPtrSizeBySymbol( m_symbol );
    }

    virtual void getFields();
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoFunctionPrototype : public TypeInfoImp
{

public:

    TypeInfoFunctionPrototype() : 
        m_hasThis(false)
        {}

protected:

    virtual std::wstring str() {
        return getName();
    }


    virtual bool isFunction() {
        return true;
    }

    virtual size_t getSize() {
        return getPtrSize();
    }

    virtual size_t getElementCount() {
        return m_args.size();
    }

    virtual TypeInfoPtr getElement( size_t index ) {
        if ( index < m_args.size() )
            return m_args[index];
        throw IndexException(index);
    }

    virtual bool hasThis() {
        return m_hasThis;
    }

    virtual CallingConventionType getCallingConvention() {
        return m_callconv;
    }

    virtual TypeInfoPtr getReturnType() {
        return m_returnType;
    }

    virtual TypeInfoPtr getClassParent() {
        return TypeInfoPtr();
    }

    virtual std::wstring getName();

    virtual std::pair<std::wstring, std::wstring> splitName();

protected:

    typedef std::vector< TypeInfoPtr > Args;
    Args m_args;

    TypeInfoPtr  m_returnType;

    CallingConventionType  m_callconv;

    bool m_hasThis;
};

///////////////////////////////////////////////////////////////////////////////


class TypeInfoSymbolFunctionPrototype : public TypeInfoFunctionPrototype
{
public:

    TypeInfoSymbolFunctionPrototype( const SymbolPtr& symbol );

protected:

    virtual size_t getPtrSize() {
        return getPtrSizeBySymbol( m_symbol );
    }

    virtual size_t getSize() {
        return m_symbol->getSize();
    }

    virtual TypeInfoPtr getClassParent();

    virtual size_t getAlignReq() {
        return getPtrSize();
    }

    std::wstring getScopeName() {
        return m_symbol->getScopeName();
    }

private:
    SymbolPtr m_symbol;

};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoSymbolFunction : public TypeInfoSymbolFunctionPrototype
{
public:

    TypeInfoSymbolFunction(const SymbolPtr& symbol) :
        TypeInfoSymbolFunctionPrototype(symbol->getType()),
        m_symbol(symbol)
    {}

    virtual NumVariant getValue() const 
    {
        try 
        {
            return m_symbol->getVa();
        }
        catch(SymbolException&)
        {}

        throw TypeException(L"function has no body");
    }

    virtual bool isVirtual()
    {
        return m_symbol->isVirtual();
    }

    virtual MEMOFFSET_REL getVtblOffset() 
    {
        return m_symbol->getVirtualBaseOffset();
    }

private:

    SymbolPtr m_symbol;

};

///////////////////////////////////////////////////////////////////////////////

template<typename T>
class TypeInfoBaseWrapper : public TypeInfoImp 
{
public:

    TypeInfoBaseWrapper( const std::wstring & name, size_t ptrSize ) :
        m_name( name ),
        m_ptrSize( ptrSize )
        {}

protected:

    virtual std::wstring str() {
        return m_name;
    }

    virtual std::wstring getName() {
        return m_name;
    }

    virtual size_t getSize() {
        return sizeof(T);
    }

    virtual bool isBase() {
        return true;
    }

    virtual size_t getPtrSize() {
        return m_ptrSize != 0 ? m_ptrSize : kdlib::ptrSize();
    }

    virtual size_t getAlignReq() {
        return getSize();
    }

protected:

    std::wstring  m_name;
    size_t  m_ptrSize;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoVoid: public TypeInfoImp 
{
public:

    TypeInfoVoid( size_t ptrSize=0 );

protected:

    virtual std::wstring str() {
        return getName();
    }


    virtual bool isVoid() {
        return true;
    }

    virtual std::wstring getName() {
        return L"Void";
    }

    virtual size_t getSize() {
        throw TypeException( getName(), L"This type has no size" );
    }

    virtual size_t getPtrSize() {
        return m_ptrSize;
    }

private:

    size_t  m_ptrSize;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoNoType : public TypeInfoImp 
{
protected:
    virtual std::wstring str() {
        return getName();
    }

    virtual std::wstring getName() {
        return L"NoType";
    }

    virtual bool isNoType() {
        return true;
    }

    virtual size_t getPtrSize() {
        throw TypeException( getName(), L"Don't exist pointers to this type" );
    }
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoVtbl: public TypeInfoImp
{
public:

    TypeInfoVtbl( const SymbolPtr &symbol ) : m_symbol( symbol ) {}

    virtual std::wstring str() {
        return getName();
    }

    virtual std::wstring getName() {
        return L"VTable";
    }

    std::wstring getScopeName() {
        return m_symbol->getScopeName();
    }

    virtual size_t getAlignReq() {
        return getPtrSizeBySymbol( m_symbol );
    }

    virtual bool isVtbl() {
        return true;
    }

    virtual size_t getElementCount();

    virtual size_t getSize();

protected:
    SymbolPtr   m_symbol;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoBitField : public TypeInfoImp 
{
public:

    TypeInfoBitField( const TypeInfoPtr& fieldType, BITOFFSET bitPos, BITOFFSET bitWidth) :
        m_bitType(fieldType),
        m_bitPos(bitPos),
        m_bitWidth(bitWidth)
        {}

protected:

    virtual bool isBitField() {
        return true;
    }

    virtual std::wstring getName();

    virtual size_t getSize() {
        return m_bitType->getSize();
    }

    virtual BITOFFSET getBitOffset() {
        return m_bitPos;
    }

    virtual BITOFFSET getBitWidth() {
        return m_bitWidth;
    }

    virtual TypeInfoPtr getBitType() {
         return m_bitType;
    }

    virtual size_t getAlignReq() {
        return getSize();
    }

protected:

    TypeInfoPtr  m_bitType;
    BITOFFSET  m_bitWidth;
    BITOFFSET  m_bitPos;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoSymbolBitField : public TypeInfoBitField 
{
public:

    TypeInfoSymbolBitField( const SymbolPtr &symbol );

protected:

    virtual TypeInfoPtr getClassParent();

    std::wstring getScopeName() {
        return m_symbol->getScopeName();
    }

private:

    SymbolPtr  m_symbol;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoReference : public TypeInfoImp
{
protected:

    virtual std::wstring getName();

};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoPointer : public TypeInfoReference 
{
public:

    TypeInfoPointer( const TypeInfoPtr &derefType, size_t ptrSize ) :
        m_derefType( derefType ),
        m_ptrSize( ptrSize )
        {}

    virtual TypeInfoPtr deref() {
        return m_derefType;
    }

    const std::wstring getDerefName() const {
        return m_derefType->getName();
    }

    //virtual TypeInfoPtr getClassParent();

    virtual size_t getAlignReq() {
        return getSize();
    }

protected:

    virtual std::wstring str() {
        return L"ptr to " + m_derefType->getName();
    }

    virtual bool isPointer() {
        return true;
    }

    virtual size_t getSize() {
        return m_ptrSize;
    }

    virtual size_t getPtrSize() {
        return m_ptrSize;
    }

protected:

    TypeInfoPtr  m_derefType;
    size_t  m_ptrSize;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoSymbolPointer : public TypeInfoPointer
{
public:

    TypeInfoSymbolPointer( const SymbolPtr& symbol ) : 
        TypeInfoPointer( loadType( symbol->getType() ), symbol->getSize() )
    {
        m_symbol = symbol;
    }

    std::wstring getScopeName() {
        return m_symbol->getScopeName();
    }

protected:

    SymbolPtr  m_symbol;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoIncompleteArray : public TypeInfoReference
{
public:

    TypeInfoIncompleteArray(const TypeInfoPtr &derefType) : m_derefType(derefType)
    {}

    TypeInfoPtr deref() override
    {
        return m_derefType;
    }

    size_t getAlignReq() override
    {
        return deref()->getAlignReq();
    }

    std::wstring str() override
    {
        return getName();
    }

    bool isArray() override
    {
        return true;
    }

    bool isIncomplete() override
    {
        return true;
    }

    size_t getPtrSize() override
    {
        return m_derefType->getPtrSize();
    }

    TypeInfoPtr getElement(size_t index) override
    {
        return m_derefType;
    }

    size_t getElementCount() override
    {
        throw TypeException(getName(), L" array is incomplete");
    }

    size_t getSize() override
    {
        throw TypeException(getName(), L" array is incomplete");
    }

protected:

    TypeInfoPtr  m_derefType;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoArray : public TypeInfoIncompleteArray
{

public:

    TypeInfoArray(const TypeInfoPtr &derefType, size_t count) : TypeInfoIncompleteArray(derefType)
    {
        size_t maxCount = SIZE_MAX / derefType->getSize();
        if (count > maxCount)
            throw TypeException(L"Failed to create too long array");

        m_count = count;
    }

protected:

    bool isIncomplete() override
    {
        return false;
    }

    size_t getElementCount() override
    {
        return m_count;
    }

    size_t getSize() override
    {
        return m_derefType->getSize() * m_count;
    }

protected:

    size_t  m_count;
};





///////////////////////////////////////////////////////////////////////////////

class TypeInfoSymbolArray : public TypeInfoArray 
{

public:

    TypeInfoSymbolArray( const SymbolPtr& symbol ) : 
        TypeInfoArray( loadType( symbol->getType() ), symbol->getCount() )
    {
        m_symbol = symbol;
    }

    std::wstring getScopeName() {
        return m_symbol->getScopeName();
    }

protected:

    SymbolPtr  m_symbol;

};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoSymbolEnum : public TypeInfoEnumerator {

public:

    TypeInfoSymbolEnum(SymbolSessionPtr& symSession, const std::wstring& mask);

    virtual TypeInfoPtr Next();

private:

    std::vector<TypeInfoPtr>  m_typeList;
    size_t  m_index;
};

///////////////////////////////////////////////////////////////////////////////

class TypeInfoSymbolProvider : public TypeInfoProvider
{
public:

    TypeInfoSymbolProvider(const std::wstring&  pdbFile, MEMOFFSET_64  loadBase);

private:

    virtual TypeInfoPtr getTypeByName(const std::wstring& name);

    virtual TypeInfoEnumeratorPtr getTypeEnumerator(const std::wstring& mask = L"");

private:

    SymbolSessionPtr  m_symbolSession;
};


///////////////////////////////////////////////////////////////////////////////

class TypeInfoDefaultProvider : public TypeInfoProvider
{

    TypeInfoPtr getTypeByName(const std::wstring& name) override
    {
        return loadType(name);
    }

    TypeInfoEnumeratorPtr getTypeEnumerator(const std::wstring& mask = L"") override
    {
        throw TypeException(L"failed to enumrate types");
    }
};


///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib