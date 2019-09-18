#include "stdafx.h"

#pragma warning( disable : 4141 4244 4291 4624 4800 4996 4267)

#include "boost/tokenizer.hpp"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include "clang/AST/RecordLayout.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Driver/Driver.h"

#include "kdlib/typeinfo.h"
#include "kdlib/exceptions.h"

#include "strconvert.h"
#include "clang.h"
#include "fnmatch.h"

using namespace clang;
using namespace clang::tooling;

namespace kdlib{

TypeInfoPtr getTypeForClangBuiltinType(const clang::BuiltinType* builtinType)
{
    switch( builtinType->getKind() )
    {
    case clang::BuiltinType::Void:
        return TypeInfoPtr( new TypeInfoVoid() );

    case clang::BuiltinType::Bool:
        return loadType(L"Bool");
    case clang::BuiltinType::Char_S:
        return loadType(L"Char");
    case clang::BuiltinType::WChar_S:
        return loadType(L"WChar");

    case clang::BuiltinType::UChar:
        return loadType(L"UInt1B");
    case clang::BuiltinType::WChar_U:
    case clang::BuiltinType::UShort:
        return loadType(L"UInt2B");
    case clang::BuiltinType::UInt:
    case clang::BuiltinType::ULong:
        return loadType(L"UInt4B");
    case clang::BuiltinType::ULongLong:
        return loadType(L"UInt8B");

    case clang::BuiltinType::Short:
        return loadType(L"Int2B");
    case clang::BuiltinType::Int:
    case clang::BuiltinType::Long:
        return loadType(L"Int4B");
    case clang::BuiltinType::LongLong:
        return loadType(L"Int8B");

    case clang::BuiltinType::Float:
        return loadType(L"Float");
    case clang::BuiltinType::Double:
    case clang::BuiltinType::LongDouble:
        return loadType(L"Double");
    }

    throw ClangException(L"Unknown base type");
}



TypeInfoPtr getTypeForClangType( ClangASTSessionPtr&  astSession, const clang::QualType& qualType )
{
    if ( qualType->isBuiltinType() )
    {
        const BuiltinType*  builtin = qualType->getAs<BuiltinType>();
        return getTypeForClangBuiltinType(builtin);
    }

    if ( qualType->isPointerType() )
    {
         const PointerType*  ptr = qualType->getAs<PointerType>();
         return TypeInfoPtr( new TypeInfoClangPointer(astSession, ptr));
    }

    if ( qualType->isArrayType() )
    {
        const ArrayType*  arrayType = qualType->getAsArrayTypeUnsafe();
        if (llvm::isa<clang::IncompleteArrayType>(arrayType))
            return TypeInfoPtr( new TypeInfoIncompleteClangArray(astSession, arrayType));

        return TypeInfoPtr( new TypeInfoClangArray(astSession, arrayType ) );
    }

    if ( qualType->isReferenceType() )
    {
        const ReferenceType*  refType = qualType->getAs<ReferenceType>();
        return TypeInfoPtr( new TypeInfoClangRef(astSession, refType ) );
    }

    if ( qualType->isRecordType() )
    {
       RecordDecl*  decl = llvm::dyn_cast<clang::RecordDecl>(qualType->getAsTagDecl());
       if ( decl->getDefinition() )
           return TypeInfoPtr( new TypeInfoClangStruct( strToWStr(decl->getNameAsString()), astSession, decl->getDefinition() ) );
       else
           return TypeInfoPtr( new TypeInfoClangStructNoDef( strToWStr(decl->getNameAsString()), astSession, decl ) );
    }

    if ( qualType->isFunctionProtoType() )
    {
        const FunctionProtoType*   funcType = qualType->getAs<FunctionProtoType>();

        return TypeInfoPtr( new TypeInfoClangFunc(astSession, funcType) );
    }

    if ( qualType->isEnumeralType() )
    {
        EnumDecl*  decl = llvm::dyn_cast<EnumDecl>(qualType->getAsTagDecl());
        return TypeInfoPtr( new TypeInfoClangEnum(astSession, decl) );
    }

    throw TypeException(L"can not parse code");
}

///////////////////////////////////////////////////////////////////////////////

TypeFieldPtr TypeFieldClangField::getField(ClangASTSessionPtr& astSession, clang::RecordDecl* structDecl, clang::FieldDecl* fieldDecl, MEMOFFSET_32 startOffset)
{
    clang::NamedDecl *nameDecl = llvm::dyn_cast<clang::NamedDecl> (fieldDecl);
    if (!nameDecl)
        throw ClangException();

    std::string   name = nameDecl->getNameAsString();

    TypeFieldClangField  *field = new TypeFieldClangField(strToWStr(name));

    field->m_astSession = astSession;
    field->m_fieldDecl = fieldDecl;
    field->m_recordDecl = structDecl;

    const ASTRecordLayout  &typeLayout = fieldDecl->getASTContext().getASTRecordLayout(structDecl);

    field->m_offset = startOffset + typeLayout.getFieldOffset(fieldDecl->getFieldIndex()) / 8;

    return TypeFieldPtr(field);
}

///////////////////////////////////////////////////////////////////////////////

TypeFieldPtr TypeFieldClangField::getStaticField(ClangASTSessionPtr& astSession,  clang::RecordDecl* structDecl, clang::VarDecl* varDecl)
{
    clang::NamedDecl *nameDecl = llvm::dyn_cast<clang::NamedDecl> (varDecl);
    if (!nameDecl)
        throw ClangException();

    std::string   name = nameDecl->getNameAsString();

    TypeFieldClangField  *field = new TypeFieldClangField(strToWStr(name));

    field->m_astSession = astSession;
    field->m_fieldDecl = varDecl;
    field->m_staticMember = true;

    return TypeFieldPtr(field);
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeFieldClangField::getTypeInfo()
{
    const clang::QualType qualType = m_fieldDecl->getType().getLocalUnqualifiedType().getCanonicalType();

    if ( qualType->isRecordType() )
    {
        RecordDecl*  decl = llvm::dyn_cast<clang::RecordDecl>(qualType->getAsTagDecl());

        std::string  name = decl->getNameAsString();
        if ( name.empty() )
        {
            std::stringstream  sstr;
            sstr << m_recordDecl->getNameAsString() << "::<unnamed-type-" << wstrToStr(getName()) << '>';
            name = sstr.str();
        }

       return TypeInfoPtr( new TypeInfoClangStruct( strToWStr(name), m_astSession, decl->getDefinition() ) );
    }

    clang::FieldDecl *fieldDecl = llvm::dyn_cast<clang::FieldDecl>(m_fieldDecl);
    if ( fieldDecl )
    {
        if ( fieldDecl->isBitField() )
        {
            TypeInfoPtr   bitType = getTypeForClangType(m_astSession, qualType);

            unsigned int bitWidth = fieldDecl->getBitWidthValue(m_astSession->getASTContext());

            const ASTRecordLayout  &typeLayout = fieldDecl->getASTContext().getASTRecordLayout(m_recordDecl);

            size_t  bitOffset = typeLayout.getFieldOffset(fieldDecl->getFieldIndex());
            
            bitOffset %= bitType->getSize() * 8;

            return TypeInfoPtr( new TypeInfoBitField(bitType, bitOffset, bitWidth) );
        }
    }

    return getTypeForClangType(m_astSession, qualType);
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoClangStruct::TypeInfoClangStruct(const std::wstring & name, ClangASTSessionPtr& session, clang::RecordDecl*  decl) :
        TypeInfoFields(name),
        m_astSession(session),
        m_decl(decl)
{
   // if ( decl->isInvalidDecl() )
   ///     throw TypeException(L"Invalid declaration");
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoClangStruct::getFields()
{
    getRecursiveFields( m_decl, 0 );
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoClangStruct::getRecursiveFields( clang::RecordDecl* recordDecl, MEMOFFSET_32 startOffset)
{
    getFieldFromBaseClasses(recordDecl, startOffset);

    const ASTRecordLayout  &typeLayout = recordDecl->getASTContext().getASTRecordLayout(recordDecl);

    for(clang::RecordDecl::decl_iterator  declit = recordDecl->decls_begin(); declit != recordDecl->decls_end(); declit++)
    {

        clang::NamedDecl *nameDecl = llvm::dyn_cast<clang::NamedDecl> (*declit);
        if (!nameDecl)
            continue;

        std::string   name = nameDecl->getNameAsString();

        clang::FieldDecl *fieldDecl = llvm::dyn_cast<clang::FieldDecl>(*declit);
        if ( fieldDecl )
        {
            if ( name.empty() )
            {
                const clang::QualType qualType = fieldDecl->getType().getLocalUnqualifiedType().getCanonicalType();
                if ( qualType->isRecordType() )
                {
                    MEMOFFSET_32  fieldOffset = typeLayout.getFieldOffset(fieldDecl->getFieldIndex()) / 8;
                    getRecursiveFields( llvm::dyn_cast<clang::RecordDecl>(qualType->getAsTagDecl()), startOffset + fieldOffset );
                }
            }
            else
            {

                auto field = TypeFieldClangField::getField(m_astSession, recordDecl, fieldDecl, startOffset);
                if (recordDecl != m_decl)
                    field->setMemberInherited();
                m_fields.push_back(field);
                continue;
            }
        }

        clang::VarDecl *varDecl = llvm::dyn_cast<clang::VarDecl>(*declit);
        if ( varDecl )
        {
            auto field = TypeFieldClangField::getStaticField(m_astSession, recordDecl, varDecl);
            if (recordDecl != m_decl)
                field->setMemberInherited();
            m_fields.push_back(field);
            continue;
        }

        clang::EnumDecl  * enumDecl = llvm::dyn_cast<clang::EnumDecl>(*declit);
        if ( enumDecl )
        {
            for (clang::EnumDecl::enumerator_iterator enumIt = enumDecl->enumerator_begin(); enumIt != enumDecl->enumerator_end(); ++enumIt)
            {
                std::string   fieldName = enumIt->getNameAsString();

                m_fields.push_back(TypeFieldClangEnumField::getField(m_astSession, *enumIt));
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoClangStruct::getFieldFromBaseClasses(clang::RecordDecl* recordDecl, MEMOFFSET_32 startOffset)
{
    const CXXRecordDecl  *classDecl = llvm::dyn_cast<clang::CXXRecordDecl>(recordDecl);

    if (!classDecl)
        return;
        
    for (auto baseIt : classDecl->bases())
    {
        auto  baseDecl = baseIt.getType()->getAsCXXRecordDecl();
        auto  baseOffset = recordDecl->getASTContext().getASTRecordLayout(m_decl).getBaseClassOffset( baseIt.getType()->getAsCXXRecordDecl() ).getQuantity();

        getRecursiveFields(baseDecl, startOffset + baseOffset);
    }
}

///////////////////////////////////////////////////////////////////////////////

size_t TypeInfoClangStruct::getSize()
{
    const ASTRecordLayout  &typeLayout = m_decl->getASTContext().getASTRecordLayout(m_decl);
    return typeLayout.getSize().getQuantity();
}

///////////////////////////////////////////////////////////////////////////////

size_t TypeInfoClangStruct::getBaseClassesCount()
{
    const CXXRecordDecl  *classDecl = llvm::dyn_cast<clang::CXXRecordDecl>(m_decl);

    if (!classDecl)
        return 0;
    
    return std::distance(classDecl->bases_begin(), classDecl->bases_end());
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoClangStruct::getBaseClass(const std::wstring& className)
{
    const CXXRecordDecl  *classDecl = llvm::dyn_cast<clang::CXXRecordDecl>(m_decl);

    if (!classDecl)
        throw TypeException(L"Type has no base class");

    for (auto baseIt : classDecl->bases())
    {
        if (baseIt.getType()->getAsCXXRecordDecl()->getNameAsString() == wstrToStr(className))
            return getTypeForClangType(m_astSession, baseIt.getType());
    }

    std::wstringstream  sstr;
    sstr << getName() << " has no this base class : " << className;
    throw TypeException(sstr.str());
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoClangStruct::getBaseClass(size_t index)
{
    const CXXRecordDecl  *classDecl = llvm::dyn_cast<clang::CXXRecordDecl>(m_decl);

    if (!classDecl)
        throw TypeException(L"Type has no base class");

    if (index >= getBaseClassesCount())
        throw IndexException(index);

    return getTypeForClangType(m_astSession, std::next(classDecl->bases_begin(), index)->getType());
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_REL TypeInfoClangStruct::getBaseClassOffset(const std::wstring &className)
{
    const CXXRecordDecl  *classDecl = llvm::dyn_cast<clang::CXXRecordDecl>(m_decl);

    if (!classDecl)
        throw TypeException(L"Type has no base class");

    for (auto baseIt : classDecl->bases())
    {
        if (baseIt.getType()->getAsCXXRecordDecl()->getNameAsString() == wstrToStr(className))
        {
            return m_decl->getASTContext().getASTRecordLayout(m_decl).getBaseClassOffset(
                baseIt.getType()->getAsCXXRecordDecl()
            ).getQuantity();
        }
    }

    std::wstringstream  sstr;
    sstr << getName() << " has no this base class : " << className;
    throw TypeException(sstr.str());
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_REL TypeInfoClangStruct::getBaseClassOffset(size_t index)
{
    const CXXRecordDecl  *classDecl = llvm::dyn_cast<clang::CXXRecordDecl>(m_decl);

    if (!classDecl)
        throw TypeException(L"Type has no base class");

    if (index >= getBaseClassesCount())
        throw IndexException(index);

    return m_decl->getASTContext().getASTRecordLayout(m_decl).getBaseClassOffset(
        std::next(classDecl->bases_begin(), index)->getType()->getAsCXXRecordDecl()
    ).getQuantity();

}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypeInfoClangStructNoDef::str()
{
    return std::wstring(L"forward declaration class/struct : ") +  m_name;
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoClangPointer::getDerefType( ClangASTSessionPtr& astSession, const clang::PointerType* refType)
{
    return getTypeForClangType(astSession, refType->getPointeeType());
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoClangArray::getDerefType( ClangASTSessionPtr& astSession, const clang::ArrayType* arrayType)
{
    return getTypeForClangType(astSession, arrayType->getElementType());
}

///////////////////////////////////////////////////////////////////////////////

size_t TypeInfoClangArray::getElementCount(const clang::ArrayType* arrayType)
{
    const ConstantArrayType*  constArray = llvm::dyn_cast<ConstantArrayType>(arrayType);
    if ( constArray )
        return  constArray->getSize().getSExtValue();

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoClangRef::deref()
{
    return getTypeForClangType(m_astSession, m_refType->getPointeeType());
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoClangFunc::TypeInfoClangFunc(ClangASTSessionPtr& session, const FunctionProtoType* funcProto)
{
    m_returnType = getTypeForClangType(session, funcProto->getReturnType());

    switch ( funcProto->getCallConv() )
    {
    case CC_C:
         m_callconv = CallConv_NearC;
         break;

    case CC_X86StdCall:
        m_callconv = CallConv_NearStd;
        break;

    case CC_X86FastCall:
        m_callconv = CallConv_NearFast;
        break;

    case CC_X86ThisCall:
        m_callconv = CallConv_ThisCall;
        break;

    default:
        throw TypeException(L"unsupported calling conversion");

    }

    for ( clang::FunctionProtoType::param_type_iterator paramIt =  funcProto->param_type_begin(); paramIt != funcProto->param_type_end(); paramIt++)
    {
        m_args.push_back( getTypeForClangType(session, *paramIt ) );
    }
}

///////////////////////////////////////////////////////////////////////////////

TypeFieldPtr TypeFieldClangEnumField::getField(ClangASTSessionPtr& astSession, clang::EnumConstantDecl*  EnumDecl)
{
    std::string   name = EnumDecl->getNameAsString();

    TypeFieldClangEnumField  *field = new TypeFieldClangEnumField(strToWStr(name));

    field->m_decl = EnumDecl;
    field->m_astSession = astSession;
    field->m_constMember = true;    

    return TypeFieldPtr(field);
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeFieldClangEnumField::getTypeInfo()
{
    return makeULongConst( static_cast<unsigned long>(m_decl->getInitVal().getExtValue() ) );
}

///////////////////////////////////////////////////////////////////////////////

NumVariant TypeFieldClangEnumField::getValue() const
{
    return NumVariant( m_decl->getInitVal().getExtValue() );
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoClangEnum::getFields()
{
    for ( clang::EnumDecl::enumerator_iterator  enumIt = m_decl->enumerator_begin(); enumIt != m_decl->enumerator_end(); ++enumIt )
    {
        std::string   fieldName = enumIt->getNameAsString();

        m_fields.push_back( TypeFieldClangEnumField::getField(m_astSession, *enumIt) );
    }
}

///////////////////////////////////////////////////////////////////////////////

class DeclNamedVisitor : public RecursiveASTVisitor<DeclNamedVisitor> 
{
public:

    DeclNamedVisitor(ClangASTSessionPtr&  astSession, const std::string&  typeName) :
        m_session(astSession),
        m_typeName(typeName)
    {}

    bool VisitCXXRecordDecl(CXXRecordDecl *Declaration)
    {
       if ( Declaration->isInvalidDecl() )
            return true;

        const std::string&  name = Declaration->getQualifiedNameAsString();

        if ( name != m_typeName)
            return true;

       if ( Declaration->getDefinition() )
            m_typeInfo = TypeInfoPtr( new TypeInfoClangStruct( strToWStr(Declaration->getNameAsString()), m_session, Declaration->getDefinition() ) );
       else
            m_typeInfo = TypeInfoPtr( new TypeInfoClangStructNoDef( strToWStr(Declaration->getNameAsString()), m_session, Declaration ) );

        return false;
    }

    bool VisitTypedefDecl(TypedefDecl  *Declaration)
    {
       if ( Declaration->isInvalidDecl() )
            return true;

        const std::string&  name = Declaration->getQualifiedNameAsString();

        if ( name != m_typeName)
            return true;

        QualType  decl = Declaration->getUnderlyingType().getCanonicalType();

        m_typeInfo = getTypeForClangType(m_session, decl);

        return false;
    }

    bool VisitFunctionDecl(FunctionDecl *Declaration)
    {
       if ( Declaration->isInvalidDecl() )
            return true;

        if ( Declaration->getTemplatedKind() == FunctionDecl::TemplatedKind:: TK_FunctionTemplate )
            return true;

        if ( CXXRecordDecl  *parentClassDecl = llvm::dyn_cast<CXXRecordDecl>(Declaration->getDeclContext()))
        {
            if ( parentClassDecl->getDescribedClassTemplate() )
                return true;
        }

        const std::string& name = Declaration->getQualifiedNameAsString();

        if ( name != m_typeName)
            return true;

         const FunctionProtoType*  protoType = Declaration->getFunctionType()->getAs<FunctionProtoType>();

        m_typeInfo = TypeInfoPtr( new TypeInfoClangFunc(m_session, protoType ) );

        return false;
    }

    bool VisitEnumDecl (EnumDecl *Declaration)
    {
       if ( Declaration->isInvalidDecl() )
            return true;

        const std::string& name = Declaration->getQualifiedNameAsString();

        if ( name != m_typeName)
            return true;

        m_typeInfo = TypeInfoPtr( new TypeInfoClangEnum(m_session, Declaration) );

        return false;
    }

    TypeInfoPtr  getTypeInfo() {
        return m_typeInfo;
    }

private:

    std::string  m_typeName;

    ClangASTSessionPtr  m_session;

    TypeInfoPtr  m_typeInfo;
};

///////////////////////////////////////////////////////////////////////////////

class DeclNextVisitor : public RecursiveASTVisitor<DeclNextVisitor>
{

public:

    DeclNextVisitor(ClangASTSessionPtr& astSession, std::map<std::string, TypeInfoPtr>* typeMap) :
        m_session(astSession),
        m_typeMap(typeMap)
    {}

    bool VisitCXXRecordDecl(CXXRecordDecl *Declaration)
    {
        try {

            CXXRecordDecl *   definition = Declaration->getDefinition();

            if (definition)
            {
                if (definition->isInvalidDecl())
                    return true;

                auto  templateDecl = definition->getDescribedClassTemplate();

                if (templateDecl)
                {
                    for (auto specIt = templateDecl->spec_begin(); specIt != templateDecl->spec_end(); specIt++)
                    {
                        LangOptions  lo;
                        PrintingPolicy pp(lo);
                        pp.SuppressTagKeyword = true;
                        pp.MSVCFormatting = true;

                        const std::string  &name = (*specIt)->getTypeForDecl()->getCanonicalTypeInternal().getAsString(pp);

                        auto  rr = llvm::dyn_cast<CXXRecordDecl>(*specIt);

                        auto typeInfo = TypeInfoPtr(new TypeInfoClangStruct(strToWStr(name), m_session, rr));
                        (*m_typeMap)[name] = typeInfo;
                    }
                }
                else
                {
                    const std::string  &name = Declaration->getQualifiedNameAsString();
                    (*m_typeMap)[name] = TypeInfoPtr(new TypeInfoClangStruct(strToWStr(name), m_session, definition));
                }
            }
            else
            {
                const std::string  &name = Declaration->getQualifiedNameAsString();
                (*m_typeMap)[name] = TypeInfoPtr(new TypeInfoClangStructNoDef(strToWStr(name), m_session, Declaration));
            }

        }
        catch (TypeException&)
        {
        }

        return true;
    }

    bool VisitTypedefDecl(TypedefDecl  *Declaration)
    {

        try {

            if (Declaration->isInvalidDecl())
                return true;

            const std::string&  name = Declaration->getQualifiedNameAsString();

            QualType  decl = Declaration->getUnderlyingType().getCanonicalType();

            (*m_typeMap)[name] = getTypeForClangType(m_session, decl);

        }
        catch (TypeException&)
        {
        }

        return true;
    }


    bool VisitFunctionDecl(FunctionDecl *Declaration)
    {
        try {

            if (Declaration->isInvalidDecl())
                return true;

            if (Declaration->getTemplatedKind() == FunctionDecl::TemplatedKind::TK_FunctionTemplate)
                return true;

            if (CXXRecordDecl  *parentClassDecl = llvm::dyn_cast<CXXRecordDecl>(Declaration->getDeclContext()))
            {
                if (parentClassDecl->getDescribedClassTemplate())
                    return true;
            }

            std::string  name = Declaration->getQualifiedNameAsString();

            const FunctionProtoType*  protoType = Declaration->getFunctionType()->getAs<FunctionProtoType>();

            TypeInfoPtr  typeInfo = TypeInfoPtr(new TypeInfoClangFunc(m_session, protoType));

            (*m_typeMap)[name] = typeInfo;

        }
        catch (TypeException&)
        {
        }

        return true;
    }

    bool VisitEnumDecl(EnumDecl *Declaration)
    {
        try {
            if (Declaration->isInvalidDecl())
                return true;

            std::string  name = Declaration->getQualifiedNameAsString();

            TypeInfoPtr  typeInfo = TypeInfoPtr(new TypeInfoClangEnum(m_session, Declaration));


            (*m_typeMap)[name] = typeInfo;

        }
        catch (TypeException&)
        {
        }

        return true;
    }

private:


    ClangASTSessionPtr  m_session;

    std::map<std::string, TypeInfoPtr>  *m_typeMap;
};

///////////////////////////////////////////////////////////////////////////////

class FuncVisitor : public RecursiveASTVisitor<FuncVisitor>
{

public:

    FuncVisitor(ClangASTSessionPtr& astSession, std::vector<std::string>&  symbols) :
        m_session(astSession),
        m_symbols(symbols)
   {}


    bool VisitFunctionDecl(FunctionDecl *Declaration)
    {
        try {

            if ( Declaration->isInvalidDecl() )
                return true;

            if ( Declaration->getTemplatedKind() == FunctionDecl::TemplatedKind::TK_FunctionTemplate )
                return true;

            SmallString<64> buf;
            llvm::raw_svector_ostream stream(buf);
            LangOptions  lo;
            PrintingPolicy pp(lo);
            pp.SuppressTagKeyword = true;
            pp.MSVCFormatting = true;

            Declaration->getNameForDiagnostic(stream, pp, true);
      
            m_symbols.push_back(stream.str());

        } catch(TypeException& )
        {}

        return true;
    }
private:


    ClangASTSessionPtr  m_session;

    std::vector<std::string>  &m_symbols;
};

///////////////////////////////////////////////////////////////////////////////

size_t ClangASTSession::getPtrSize()
{
    const Type*  sizeType = getASTContext().getSizeType()->getTypePtr();
    return static_cast<const clang::BuiltinType*>(sizeType)->getKind() ==  clang::BuiltinType::Kind::ULongLong  ? 8 : 4;
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr compileType( const std::wstring& sourceCode, const std::wstring& typeName, const std::wstring& options)
{
    return getTypeInfoProviderFromSource(sourceCode, options)->getTypeByName(typeName);
}

///////////////////////////////////////////////////////////////////////////////

class ASTBuilderAction : public clang::tooling::ToolAction
{
    std::vector<std::unique_ptr<ASTUnit>> &ASTs;

public:
    ASTBuilderAction(std::vector<std::unique_ptr<ASTUnit>> &ASTs) : ASTs(ASTs) {}

    bool runInvocation(std::shared_ptr<CompilerInvocation> Invocation,
        FileManager *Files,
        std::shared_ptr<PCHContainerOperations> PCHContainerOps,
        DiagnosticConsumer *DiagConsumer) override {
        std::unique_ptr<ASTUnit> AST = ASTUnit::LoadFromCompilerInvocation(
            Invocation, std::move(PCHContainerOps),
            CompilerInstance::createDiagnostics(&Invocation->getDiagnosticOpts(),
                DiagConsumer,
                /*ShouldOwnClient=*/false),
            Files);

        if (!AST)
            return false;

        ASTs.push_back(std::move(AST));
        return true;
    }
};

TypeInfoProviderClang::TypeInfoProviderClang( const std::string& sourceCode, const std::string& compileOptions)
{
    std::vector<std::unique_ptr<ASTUnit>> ASTs;
    ASTBuilderAction Action(ASTs);
    llvm::IntrusiveRefCntPtr<vfs::OverlayFileSystem> OverlayFileSystem(
        new vfs::OverlayFileSystem(vfs::getRealFileSystem()));
    llvm::IntrusiveRefCntPtr<vfs::InMemoryFileSystem> InMemoryFileSystem(
        new vfs::InMemoryFileSystem);
    OverlayFileSystem->pushOverlay(InMemoryFileSystem);
    llvm::IntrusiveRefCntPtr<FileManager> Files(
        new FileManager(FileSystemOptions(), OverlayFileSystem));

    std::vector< std::string > args;

    args.push_back("clang-tool");
    args.push_back("-fsyntax-only");

    typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;
    boost::escaped_list_separator<char> Separator('\\', ' ', '\"');

    Tokenizer tok(compileOptions, Separator);

    std::copy(tok.begin(), tok.end(), std::inserter(args, args.end()));

    args.push_back("input.cc");

    ToolInvocation toolInvocation(
        args,
        &Action, 
        Files.get(), 
        std::move(std::make_shared< PCHContainerOperations >())
    );

    InMemoryFileSystem->addFile("input.cc", 0, llvm::MemoryBuffer::getMemBuffer(sourceCode.c_str()));

#ifndef _DEBUG

    IgnoringDiagConsumer   diagnosticConsumer;

    toolInvocation.setDiagnosticConsumer(&diagnosticConsumer);

#endif

    toolInvocation.run();

    std::unique_ptr<ASTUnit>  ast = std::move(ASTs[0]);

    m_astSession = ClangASTSession::getASTSession(ast);

    DeclNextVisitor   visitor(m_astSession, &m_typeCache);

    visitor.TraverseDecl( m_astSession->getASTContext().getTranslationUnitDecl() );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoProviderClang::getTypeByName(const std::wstring& name)
{
    auto  foundType = m_typeCache.find( wstrToStr(name) );

    if ( foundType == m_typeCache.end() )
        throw TypeException(name, L"Failed to get type");

    return foundType->second;
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoEnumeratorPtr TypeInfoProviderClang::getTypeEnumerator(const std::wstring& mask) 
{
    return TypeInfoEnumeratorPtr( new TypeInfoProviderClangEnum(mask, shared_from_this()) );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoProviderClangEnum::TypeInfoProviderClangEnum(const std::wstring& mask, const boost::shared_ptr<TypeInfoProviderClang>& clangProvider )
{
    m_index = 0;

    std::string  ansimask = wstrToStr(mask);

    std::for_each( clangProvider->m_typeCache.begin(), clangProvider->m_typeCache.end(),
        [&]( const std::pair<std::string, TypeInfoPtr> &it ) {
            if (ansimask.empty() || fnmatch(ansimask, it.first) )
                m_typeList.push_back(it.second);
        }
    );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoProviderClangEnum::Next()
{
    if ( m_index < m_typeList.size() )
        return m_typeList[m_index++];
    return TypeInfoPtr();
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoProviderPtr  getTypeInfoProviderFromSource( const std::wstring&  source, const std::wstring&  opts )
{
    return TypeInfoProviderPtr( new TypeInfoProviderClang(wstrToStr(source), wstrToStr(opts) ) );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoProviderPtr  getTypeInfoProviderFromSource(const std::string&  source, const std::string&  opts)
{
    return TypeInfoProviderPtr(new TypeInfoProviderClang(source, opts));
}

///////////////////////////////////////////////////////////////////////////////

SymbolEnumeratorClang::SymbolEnumeratorClang(const std::string&  sourceCode, const std::string&  compileOptions)
{
    std::vector<std::unique_ptr<ASTUnit>> ASTs;
    ASTBuilderAction Action(ASTs);
    llvm::IntrusiveRefCntPtr<vfs::OverlayFileSystem> OverlayFileSystem(
        new vfs::OverlayFileSystem(vfs::getRealFileSystem()));
    llvm::IntrusiveRefCntPtr<vfs::InMemoryFileSystem> InMemoryFileSystem(
        new vfs::InMemoryFileSystem);
    OverlayFileSystem->pushOverlay(InMemoryFileSystem);
    llvm::IntrusiveRefCntPtr<FileManager> Files(
        new FileManager(FileSystemOptions(), OverlayFileSystem));

    std::vector< std::string > args;

    args.push_back("clang-tool");
    args.push_back("-fsyntax-only");

    typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;
    boost::escaped_list_separator<char> Separator('\\', ' ', '\"');

    Tokenizer tok(compileOptions, Separator);

    std::copy(tok.begin(), tok.end(), std::inserter(args, args.end()));

    args.push_back("input.cc");

    ToolInvocation toolInvocation(
        args,
        &Action,
        Files.get(),
        std::move(std::make_shared< PCHContainerOperations >())
    );

    InMemoryFileSystem->addFile("input.cc", 0, llvm::MemoryBuffer::getMemBuffer(sourceCode.c_str()));

#ifndef _DEBUG

    IgnoringDiagConsumer   diagnosticConsumer;

    toolInvocation.setDiagnosticConsumer(&diagnosticConsumer);

#endif

    toolInvocation.run();

    std::unique_ptr<ASTUnit>  ast = std::move(ASTs[0]);

    auto astSession = ClangASTSession::getASTSession(ast);

    FuncVisitor   visitor(astSession, m_symbols);

    visitor.TraverseDecl(astSession->getASTContext().getTranslationUnitDecl());
}

///////////////////////////////////////////////////////////////////////////////

std::wstring SymbolEnumeratorClang::Next()
{
    if (m_index < m_symbols.size())
        return strToWStr(m_symbols[m_index++]);
    
    return std::wstring();
}

///////////////////////////////////////////////////////////////////////////////

SymbolEnumeratorPtr getSymbolEnumeratorFromSource(const std::wstring& source, const std::wstring&  opts)
{
    return SymbolEnumeratorPtr( new SymbolEnumeratorClang(wstrToStr(source), wstrToStr(opts) ) );
}

///////////////////////////////////////////////////////////////////////////////

SymbolEnumeratorPtr getSymbolEnumeratorFromSource(const std::string& source, const std::string&  opts)
{
    return SymbolEnumeratorPtr(new SymbolEnumeratorClang(source, opts));
}

///////////////////////////////////////////////////////////////////////////////
}

