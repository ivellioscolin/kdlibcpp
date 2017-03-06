#include "stdafx.h"

#pragma warning( disable : 4141 4244 4291 4624 4800 4996 4267)

#include "boost/tokenizer.hpp"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include "clang/AST/RecordLayout.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include "kdlib/typeinfo.h"
#include "kdlib/exceptions.h"

#include "strconvert.h"
#include "clang.h"

using namespace clang;
using namespace clang::tooling;

namespace kdlib{

TypeInfoPtr getTypeForClangBuiltinType(const clang::BuiltinType* builtinType)
{
    switch( builtinType->getKind() )
    {
    case clang::BuiltinType::Kind::Void:
        return TypeInfoPtr( new TypeInfoVoid() );

    case clang::BuiltinType::Kind::Bool:
        return loadType(L"Bool");
    case clang::BuiltinType::Kind::Char_S:
        return loadType(L"Char");
    case clang::BuiltinType::Kind::WChar_S:
        return loadType(L"WChar");

    case clang::BuiltinType::Kind::UChar:
        return loadType(L"UInt1B");
    case clang::BuiltinType::Kind::WChar_U:
    case clang::BuiltinType::Kind::UShort:
        return loadType(L"UInt2B");
    case clang::BuiltinType::Kind::UInt:
    case clang::BuiltinType::Kind::ULong:
        return loadType(L"UInt4B");
    case clang::BuiltinType::Kind::ULongLong:
        return loadType(L"UInt8B");

    case clang::BuiltinType::Kind::Short:
        return loadType(L"Int2B");
    case clang::BuiltinType::Kind::Int:
    case clang::BuiltinType::Kind::Long:
        return loadType(L"Int4B");
    case clang::BuiltinType::Kind::LongLong:
        return loadType(L"Int8B");

    case clang::BuiltinType::Kind::Float:
        return loadType(L"Float");
    case clang::BuiltinType::Kind::Double:
    case clang::BuiltinType::Kind::LongDouble:
        return loadType(L"Double");
    }

    throw ClangException(L"Unknown base type");
}



TypeInfoPtr getTypeForClangType( ClangASTSessionPtr&  astSession, const clang::QualType& qualType )
{
    if ( qualType->isBuiltinType() )
    {
        return getTypeForClangBuiltinType(static_cast<const clang::BuiltinType*>(qualType.getTypePtr()));
    }

    if ( qualType->isPointerType() )
    {
         clang::PointerType*  ptr = const_cast<clang::PointerType*>(static_cast<const clang::PointerType*>(qualType.getTypePtr()));
         return TypeInfoPtr( new TypeInfoClangPointer(astSession, ptr));
    }

    if ( qualType->isArrayType() )
    {
        clang::ArrayType*  arrayType = const_cast<clang::ArrayType*>(static_cast<const clang::ArrayType*>(qualType.getTypePtr()));
        return TypeInfoPtr( new TypeInfoClangArray(astSession, arrayType ) );
    }

    if ( qualType->isReferenceType() )
    {
        clang::ReferenceType*  refType = const_cast<clang::ReferenceType*>(static_cast<const clang::ReferenceType*>(qualType.getTypePtr()));
        return TypeInfoPtr( new TypeInfoClangRef(astSession, refType ) );
    }

    if ( qualType->isRecordType() )
    {
       RecordDecl*  decl = llvm::dyn_cast<clang::RecordDecl>(qualType->getAsTagDecl());
       return TypeInfoPtr( new TypeInfoClangStruct( strToWStr(decl->getNameAsString()), astSession, decl->getMostRecentDecl() ) );
    }

    if ( qualType->isFunctionType() )
    {
        return TypeInfoPtr( new TypeInfoClangFunc() );
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

void TypeInfoClangStruct::getFields()
{
    getRecursiveFields( m_decl, 0 );
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoClangStruct::getRecursiveFields( clang::RecordDecl* recordDecl, MEMOFFSET_32 startOffset)
{
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
                m_fields.push_back( TypeFieldClangField::getField(m_astSession, recordDecl, fieldDecl, startOffset) );
                continue;
            }
        }

        clang::VarDecl *varDecl = llvm::dyn_cast<clang::VarDecl>(*declit);
        if ( varDecl )
        {
            m_fields.push_back( TypeFieldClangField::getStaticField(m_astSession, recordDecl, varDecl) );
            continue;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

size_t TypeInfoClangStruct::getSize()
{
    const ASTRecordLayout  &typeLayout = m_decl->getASTContext().getASTRecordLayout(m_decl);
    return typeLayout.getSize().getQuantity();
}


///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoClangPointer::getDerefType( ClangASTSessionPtr& astSession, clang::PointerType* refType)
{
    return getTypeForClangType(astSession, refType->getPointeeType());
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoClangArray::getDerefType( ClangASTSessionPtr& astSession, clang::ArrayType* arrayType)
{
    return getTypeForClangType(astSession, arrayType->getElementType());
}

///////////////////////////////////////////////////////////////////////////////

size_t TypeInfoClangArray::getElementCount(clang::ArrayType* arrayType)
{
    clang::ConstantArrayType*  constArray = llvm::dyn_cast<clang::ConstantArrayType> (arrayType);
    if ( constArray )
    {
        return  constArray->getSize().getSExtValue();
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoClangRef::deref()
{
    return getTypeForClangType(m_astSession, m_refType->getPointeeType());
}

///////////////////////////////////////////////////////////////////////////////

class StructDefVisitor : public RecursiveASTVisitor<StructDefVisitor> 
{
public:

    StructDefVisitor(ClangASTSessionPtr&  astSession, const std::string&  typeName) :
        m_session(astSession),
        m_typeName(typeName)
    {}

    bool VisitCXXRecordDecl(CXXRecordDecl *Declaration)
    {
        std::string&  name = Declaration->getQualifiedNameAsString();

        if ( Declaration->getNameAsString() != m_typeName)
            return true;

        clang::CXXRecordDecl*  decl = Declaration->getDefinition();

        if ( !decl )
            return true;

        m_typeInfo = TypeInfoPtr( new TypeInfoClangStruct(strToWStr(name), m_session, decl ) );

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

TypeInfoPtr ClangASTSession::getTypeInfo(const std::wstring& name)
{
    StructDefVisitor  visitor( shared_from_this(), wstrToStr(name) );
    visitor.TraverseDecl( m_astUnit->getASTContext().getTranslationUnitDecl() );

    TypeInfoPtr  ptr = visitor.getTypeInfo();

    if (!ptr)
        throw TypeException(L"Failed to get type from AST");

    return ptr;
}

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

TypeInfoProviderClang::TypeInfoProviderClang( const std::wstring& sourceCode, const std::wstring& compileOptions)
{
    std::vector< std::string > args;

    typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;
    boost::escaped_list_separator<char> Separator( '\\', ' ', '\"' );

    std::string  optionsStr = wstrToStr(compileOptions);
    Tokenizer tok( optionsStr, Separator );

    std::copy(tok.begin(), tok.end(), std::inserter(args, args.end() ) );

    args.push_back("-w");

    std::vector< std::string >::iterator  it = args.begin();
    for ( ; it != args.end(); ++it )
    {
        if ( 0 == it->find("--target") )
            break;
    }

    if ( it == args.end() )
    {
        if ( getCPUMode() == CPU_AMD64 )
            args.push_back(" --target=i686-pc-windows-msvc");
        if ( getCPUMode() == CPU_I386 )
            args.push_back(" --target=x86_64-pc-windows-msvc");
    }

    std::unique_ptr<ASTUnit>  ast = buildASTFromCodeWithArgs(wstrToStr(sourceCode), args );

    m_astSession = ClangASTSession::getASTSession(ast);
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoProviderClang::getTypeByName(const std::wstring& name)
{
    return m_astSession->getTypeInfo(name);
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoProviderPtr  getTypeInfoProviderFromSource( const std::wstring&  source, const std::wstring&  opts )
{
    return TypeInfoProviderPtr( new TypeInfoProviderClang(source, opts) );
}

///////////////////////////////////////////////////////////////////////////////


}

