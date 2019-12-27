#pragma once

#include <boost/shared_ptr.hpp>

#include <clang/Frontend/ASTUnit.h>
#include <clang/AST/Type.h>


#include "typeinfoimp.h"


namespace kdlib {


class ClangASTSession;
typedef boost::shared_ptr<ClangASTSession>  ClangASTSessionPtr;

class TypeInfoProviderClang;


class ClangASTSession : public boost::enable_shared_from_this<ClangASTSession>
{
public:

    static ClangASTSessionPtr getASTSession(std::unique_ptr<clang::ASTUnit>&  astUnit) {
        return ClangASTSessionPtr( new ClangASTSession(astUnit) );
    }

    //TypeInfoPtr getTypeInfo(const std::wstring& name);

    clang::ASTContext&  getASTContext() {
          return m_astUnit->getASTContext();
    }

    size_t getPtrSize() ;


private:

    ClangASTSession(std::unique_ptr<clang::ASTUnit>& astUnit) 
    {
        astUnit.swap(m_astUnit);
    }

    std::unique_ptr<clang::ASTUnit>  m_astUnit;

};


class TypeFieldClangField : public TypeField
{
public:

    static TypeFieldPtr getField(ClangASTSessionPtr& astSession, clang::RecordDecl* structDecl, clang::FieldDecl* fieldDecl, MEMOFFSET_32 startOffset);
    static TypeFieldPtr getStaticField(ClangASTSessionPtr& astSession, clang::RecordDecl* structDecl, clang::VarDecl* varDecl);

private:

    TypeFieldClangField(const std::wstring  &name) :
        TypeField(name)
        {}

    virtual TypeInfoPtr getTypeInfo();

    ClangASTSessionPtr  m_astSession;
    clang::DeclaratorDecl*  m_fieldDecl;
    clang::RecordDecl*  m_recordDecl;
};


class TypeInfoClangStruct : public TypeInfoFields
{
public:

    TypeInfoClangStruct(const std::wstring & name, ClangASTSessionPtr& session, clang::RecordDecl*  decl);

protected:

    virtual bool isUserDefined() {
        return true;
    }

    virtual std::wstring str() {
        TypeInfoPtr  selfPtr = shared_from_this();
        return printStructType(selfPtr);
    }

    virtual size_t getBaseClassesCount();

    virtual TypeInfoPtr getBaseClass(const std::wstring& className);
    
    virtual TypeInfoPtr getBaseClass(size_t index);

    virtual MEMOFFSET_REL getBaseClassOffset(const std::wstring &name);

    virtual MEMOFFSET_REL getBaseClassOffset(size_t index);

    virtual size_t getSize();

protected:

    virtual void getFields();

    void getRecursiveFields(clang::RecordDecl* recordDecl, MEMOFFSET_32 startOffset);

    void getFieldFromBaseClasses(clang::RecordDecl* recordDecl, MEMOFFSET_32 startOffset);

    ClangASTSessionPtr  m_astSession;

    clang::RecordDecl*  m_decl;
};

class TypeInfoClangStructNoDef : public TypeInfoImp
{
public:

    TypeInfoClangStructNoDef(const std::wstring& name, ClangASTSessionPtr& session, clang::RecordDecl*  decl) :
        m_name(name),
        m_astSession(session),
        m_decl(decl)
        {}

protected:

    bool isUserDefined() override {
        return true;
    }

    std::wstring getName() override  {
        return  m_name;
    }

    std::wstring str() override;

    size_t getSize() override {
        throw TypeException(L"not complete declaration");
    }

    TypeInfoPtr getElement( const std::wstring &name ) override  {
        throw TypeException(L"not complete declaration");
    }

    TypeInfoPtr getElement( size_t index ) override {
        throw TypeException(L"not complete declaration");
    }

    MEMOFFSET_REL getElementOffset( const std::wstring &name ) override  {
        throw TypeException(L"not complete declaration");
    }

    MEMOFFSET_REL getElementOffset( size_t index ) override {
        throw TypeException(L"not complete declaration");
    }

    std::wstring getElementName( size_t index ) override  {
        throw TypeException(L"not complete declaration");
    }

    size_t getElementIndex( const std::wstring &name )  override {
        throw TypeException(L"not complete declaration");
    }

    size_t getElementCount() override {
        throw TypeException(L"not complete declaration");
    }

    MEMOFFSET_64 getElementVa( const std::wstring &name ) override {
        throw TypeException(L"not complete declaration");
    }

    MEMOFFSET_64 getElementVa( size_t index ) override  {
        throw TypeException(L"not complete declaration");
    }

    bool isStaticMember( const std::wstring &name ) override {
        throw TypeException(L"not complete declaration");
    }

    bool isStaticMember( size_t index ) override {
        throw TypeException(L"not complete declaration");
    }

    bool isVirtualMember( const std::wstring &name ) override {
        throw TypeException(L"not complete declaration");
    }

    bool isVirtualMember( size_t index ) override  {
        throw TypeException(L"not complete declaration");
    }

    bool isIncomplete() override {
        return true;
    }

protected:

    std::wstring  m_name;

    ClangASTSessionPtr  m_astSession;

    clang::RecordDecl*  m_decl;

};


class TypeInfoClangPointer : public TypeInfoPointer
{
public:

    TypeInfoClangPointer( ClangASTSessionPtr& session, const clang::PointerType* ptrType) :
        TypeInfoPointer( TypeInfoClangPointer::getDerefType(session, ptrType), session->getPtrSize() )
        {}

protected:

    static TypeInfoPtr  getDerefType( ClangASTSessionPtr& session, const clang::PointerType* refType);
};


class TypeInfoClangArray: public TypeInfoArray
{
public:
    TypeInfoClangArray(ClangASTSessionPtr& session, const clang::ArrayType* arrayType) :
        TypeInfoArray(TypeInfoClangArray::getDerefType(session, arrayType), getElementCount(arrayType))
        {}

protected:

    static TypeInfoPtr getDerefType( ClangASTSessionPtr& session, const clang::ArrayType* arrayType);

    size_t getElementCount(const clang::ArrayType* arrayType);

};


class TypeInfoIncompleteClangArray : public TypeInfoClangArray
{
public:
    TypeInfoIncompleteClangArray(ClangASTSessionPtr& session, const clang::ArrayType* arrayType) :
        TypeInfoClangArray(session, arrayType)
    {}

protected:

    bool isIncomplete() override
    {
        return true;
    }

    size_t getSize() override
    {
        throw TypeException(L"incomplete array");
    }

};

class TypeInfoClangRef : public TypeInfoImp
{
public:
    TypeInfoClangRef(ClangASTSessionPtr& session, const clang::ReferenceType* refType) :
        m_astSession(session),
        m_refType(refType)
        {}

protected:

    virtual TypeInfoPtr deref();

    virtual std::wstring getName() {
        return deref()->getName() + L"&";
    }

    virtual std::wstring str() {
        return L"ref to " + deref()->getName();
    }

protected:


    ClangASTSessionPtr  m_astSession;

    const clang::ReferenceType*  m_refType;
};

class TypeInfoClangFuncPrototype : public TypeInfoFunctionPrototype
{
public:
    TypeInfoClangFuncPrototype(ClangASTSessionPtr& session, const clang::FunctionProtoType* functionProtoType);
};

class TypeInfoClangFunc : public TypeInfoClangFuncPrototype
{

public:

    TypeInfoClangFunc(ClangASTSessionPtr& session, clang::FunctionDecl*  funcDecl);

private:

    TypeInfoPtr getClassParent() override
    {
        return m_classParent;
    }

    TypeInfoPtr  m_classParent;
};


class TypeFieldClangEnumField : public TypeField
{
public:

    static TypeFieldPtr getField(ClangASTSessionPtr& astSession, clang::EnumConstantDecl*  EnumDecl);

private:

    TypeFieldClangEnumField(const std::wstring  &name) :
        TypeField(name)
        {}

    virtual TypeInfoPtr getTypeInfo();

    NumVariant getValue() const;

    clang::EnumConstantDecl*   m_decl;
  
    ClangASTSessionPtr  m_astSession;
};


class TypeInfoClangEnum : public TypeInfoFields
{
public:

     TypeInfoClangEnum(ClangASTSessionPtr& session, clang::EnumDecl* decl) :
        TypeInfoFields(strToWStr(decl->getNameAsString())),
        m_astSession(session),
        m_decl(decl)
     {}

protected:
    
    virtual bool isEnum() {
        return true;
    }

    virtual std::wstring str() {
        TypeInfoPtr  selfPtr = shared_from_this();
        return printEnumType(selfPtr);
    }

protected:

    virtual void getFields();

    ClangASTSessionPtr  m_astSession;

    clang::EnumDecl*  m_decl;
};



class TypeInfoProviderClangEnum  : public TypeInfoEnumerator {

public:
    
    virtual TypeInfoPtr Next();

    TypeInfoProviderClangEnum(const std::wstring& mask, const boost::shared_ptr<TypeInfoProviderClang>& clangProvider );


private:

    size_t   m_index;

    std::vector<TypeInfoPtr>  m_typeList;
};


class TypeInfoProviderClang : public TypeInfoProvider, public boost::enable_shared_from_this<TypeInfoProviderClang>
{

    friend TypeInfoProviderClangEnum;

public:

    TypeInfoProviderClang( const std::string&  sourceCode, const std::string&  compileOptions);

private:

    TypeInfoPtr getTypeByName(const std::wstring& name) override;

    TypeInfoEnumeratorPtr getTypeEnumerator(const std::wstring& mask) override;

    std::wstring makeTypeName(const std::wstring& typeName, const std::wstring& typeQualifier, bool isConst) override;

private:

    ClangASTSessionPtr  m_astSession;

    std::map< std::string, TypeInfoPtr>  m_typeCache;
};


class SymbolEnumeratorClang;

using SymbolList = std::vector<std::pair<std::string, clang::FunctionDecl*> >;

class SymbolProviderClang : public SymbolProvider, public boost::enable_shared_from_this< SymbolProviderClang>
{

public:

    friend SymbolEnumeratorClang;

    SymbolProviderClang(const std::string&  sourceCode, const std::string&  compileOptions);

private:

    SymbolEnumeratorPtr getSymbolEnumerator(const std::wstring& mask = L"") override;

    ClangASTSessionPtr  m_astSession;

    SymbolList  m_symbols;
};


class SymbolEnumeratorClang : public SymbolEnumerator
{

public:

    SymbolEnumeratorClang(const std::string& mask, const boost::shared_ptr<SymbolProviderClang>& clangProvider) :
        m_symbolProvider(clangProvider),
        m_index(-1),
        m_mask(mask)
    {}

private:

    virtual bool Next() override;
    virtual std::wstring getName() override;
    virtual MEMOFFSET_64 getOffset() override;
    virtual TypeInfoPtr getType() override;

private:

    size_t   m_index;

    std::string  m_mask;

    boost::shared_ptr<SymbolProviderClang> m_symbolProvider;
};

}
