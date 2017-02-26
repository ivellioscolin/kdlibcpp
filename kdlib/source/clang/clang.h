#pragma once

#include <boost/shared_ptr.hpp>

#include <clang/Frontend/ASTUnit.h>
#include <clang/AST/Type.h>


#include "typeinfoimp.h"


namespace kdlib {


class ClangASTSession;
typedef boost::shared_ptr<ClangASTSession>  ClangASTSessionPtr;



class ClangASTSession : public boost::enable_shared_from_this<ClangASTSession>
{
public:

    static ClangASTSessionPtr getASTSession(std::unique_ptr<clang::ASTUnit>&  astUnit) {
        return ClangASTSessionPtr( new ClangASTSession(astUnit) );
    }

    TypeInfoPtr getTypeInfo(const std::wstring& name);

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
    TypeInfoClangStruct(const std::wstring & name, ClangASTSessionPtr& session, clang::CXXRecordDecl*  decl) :
        TypeInfoFields(name),
        m_astSession(session),
        m_decl(decl)
    {}

protected:

    virtual bool isUserDefined() {
        return true;
    }

    virtual std::wstring str() {
        TypeInfoPtr  selfPtr = shared_from_this();
        return printStructType(selfPtr);
    }

    virtual size_t getSize();

protected:

    virtual void getFields();

    void TypeInfoClangStruct::getRecursiveFields( clang::CXXRecordDecl* recordDecl, MEMOFFSET_32 startOffset);

    ClangASTSessionPtr  m_astSession;

    clang::CXXRecordDecl*  m_decl;
};


class TypeInfoClangPointer : public TypeInfoPointer
{
public:

    TypeInfoClangPointer( ClangASTSessionPtr& session, clang::PointerType* ptrType) :
        TypeInfoPointer( TypeInfoClangPointer::getDerefType(session, ptrType), session->getPtrSize() )
        {}

protected:

    static TypeInfoPtr  getDerefType( ClangASTSessionPtr& session, clang::PointerType* refType);
};


class TypeInfoClangArray: public TypeInfoArray
{
public:
    TypeInfoClangArray(ClangASTSessionPtr& session, clang::ArrayType* arrayType) :
        TypeInfoArray(TypeInfoClangArray::getDerefType(session, arrayType), getElementCount(arrayType))
        {}

protected:

    static TypeInfoPtr getDerefType( ClangASTSessionPtr& session, clang::ArrayType* arrayType);

    size_t getElementCount(clang::ArrayType* arrayType);

};

class TypeInfoClangRef : public TypeInfoImp
{
public:
    TypeInfoClangRef(ClangASTSessionPtr& session, clang::ReferenceType* refType) :
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

    clang::ReferenceType*  m_refType;
};

class TypeInfoClangFunc : public TypeInfoImp
{

protected:

    virtual bool isFunction() {
        return true;
    }

    virtual std::wstring getName() {
        return L"function";
    }

    virtual std::wstring str() {
        return L"function";
    }
};

class TypeInfoProviderClang : public TypeInfoProvider
{
public:

    TypeInfoProviderClang( const std::wstring&  sourceCode, const std::wstring&  compileOptions);

private:

    virtual TypeInfoPtr getTypeByName(const std::wstring& name);

private:

    ClangASTSessionPtr  m_astSession;

};



}