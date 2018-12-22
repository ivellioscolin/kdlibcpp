#include <stdafx.h>

#include <memory>
#include <regex>

#include "clang/Basic/MemoryBufferCache.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"

#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/HeaderSearchOptions.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"

#include "kdlib/typedvar.h"
#include "kdlib/exceptions.h"

#include "evalexpr.h"
#include "strconvert.h"
#include "exprparser.h"

namespace kdlib {

TypedValue getNumericConst(const clang::Token& token);

TypedValue getCharConst(const clang::Token& token);

std::string  getIdentifier(const clang::Token& token);

kdlib::TypeInfoPtr getStandardIntType(const std::string& name);

bool isBinOperation(const clang::Token& token);

bool isInt64KeyWord(const clang::Token& token);

///////////////////////////////////////////////////////////////////////////////

TypedValue evalExpr(const std::wstring& expr, const ScopePtr& scope, const TypeInfoProviderPtr& typeInfoProvider)
{
    return evalExpr(wstrToStr(expr), scope, typeInfoProvider);
}

///////////////////////////////////////////////////////////////////////////////

TypedValue evalExpr(const std::string& expr, const ScopePtr& scope, const TypeInfoProviderPtr& typeInfoProvider)
{
    auto  preprocessorOptions = std::make_shared<clang::PreprocessorOptions>();

    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diagnosticIDs(new clang::DiagnosticIDs());

    auto diagnosticOptions = new clang::DiagnosticOptions();

    auto textDiagnosticPrinter = new clang::TextDiagnosticPrinter(llvm::outs(), diagnosticOptions);

    clang::DiagnosticsEngine  diagnosticEngine(diagnosticIDs, diagnosticOptions, textDiagnosticPrinter);

    clang::LangOptions  langOptions;

    llvm::IntrusiveRefCntPtr<clang::vfs::InMemoryFileSystem>  memoryFileSystem(new clang::vfs::InMemoryFileSystem());

    memoryFileSystem->addFile("<input>", 0, llvm::MemoryBuffer::getMemBuffer(expr));

    clang::FileSystemOptions  fileSystemOptions;
    clang::FileManager  fileManager(fileSystemOptions, memoryFileSystem);
    clang::SourceManager  sourceManager(diagnosticEngine, fileManager);

    const clang::FileEntry *pFile = fileManager.getFile("<input>");
    clang::FileID  fileID = sourceManager.getOrCreateFileID(pFile, clang::SrcMgr::C_User);
    sourceManager.setMainFileID(fileID);

    clang::MemoryBufferCache  memoryBufferCache;

    auto headerSearchOptions = std::make_shared<clang::HeaderSearchOptions>();
    auto targetOptions = std::make_shared<clang::TargetOptions>();
    targetOptions->Triple = llvm::sys::getDefaultTargetTriple();
    clang::TargetInfo*  targetInfo = clang::TargetInfo::CreateTargetInfo(diagnosticEngine, targetOptions);
    clang::HeaderSearch  headerSearch(headerSearchOptions, sourceManager, diagnosticEngine, langOptions, targetInfo);

    clang::CompilerInstance  compilerInstance;

    clang::Preprocessor   preprocessor(
        preprocessorOptions,
        diagnosticEngine,
        langOptions,
        sourceManager,
        memoryBufferCache,
        headerSearch,
        compilerInstance
    );
    preprocessor.Initialize(*targetInfo);

    preprocessor.EnterMainSourceFile();
    textDiagnosticPrinter->BeginSourceFile(langOptions, &preprocessor);

    clang::Token token;

    std::list<clang::Token>  tokens;

    do {

        preprocessor.Lex(token);

        if (diagnosticEngine.hasErrorOccurred())
        {
            NOT_IMPLEMENTED();
        }

        tokens.push_back(token);

    } while (!token.is(clang::tok::eof));

    textDiagnosticPrinter->EndSourceFile();

    ExprEval  exprEval(scope, typeInfoProvider, &tokens);

    return exprEval.getResult();
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr evalType(const std::wstring& expr, const TypeInfoProviderPtr typeInfoProvider)
{
    return evalType(wstrToStr(expr), typeInfoProvider);
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr evalType(const std::string& expr, const TypeInfoProviderPtr typeInfoProvider)
{
    auto  preprocessorOptions = std::make_shared<clang::PreprocessorOptions>();

    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diagnosticIDs(new clang::DiagnosticIDs());

    auto diagnosticOptions = new clang::DiagnosticOptions();

    auto textDiagnosticPrinter = new clang::TextDiagnosticPrinter(llvm::outs(), diagnosticOptions);

    clang::DiagnosticsEngine  diagnosticEngine(diagnosticIDs, diagnosticOptions, textDiagnosticPrinter);

    clang::LangOptions  langOptions;

    llvm::IntrusiveRefCntPtr<clang::vfs::InMemoryFileSystem>  memoryFileSystem(new clang::vfs::InMemoryFileSystem());
    memoryFileSystem->addFile("<input>", 0, llvm::MemoryBuffer::getMemBuffer(expr.c_str()));

    clang::FileSystemOptions  fileSystemOptions;
    clang::FileManager  fileManager(fileSystemOptions, memoryFileSystem);
    clang::SourceManager  sourceManager(diagnosticEngine, fileManager);

    const clang::FileEntry *pFile = fileManager.getFile("<input>");
    clang::FileID  fileID = sourceManager.getOrCreateFileID(pFile, clang::SrcMgr::C_User);
    sourceManager.setMainFileID(fileID);

    clang::MemoryBufferCache  memoryBufferCache;

    auto headerSearchOptions = std::make_shared<clang::HeaderSearchOptions>();
    auto targetOptions = std::make_shared<clang::TargetOptions>();
    targetOptions->Triple = llvm::sys::getDefaultTargetTriple();
    clang::TargetInfo*  targetInfo = clang::TargetInfo::CreateTargetInfo(diagnosticEngine, targetOptions);
    clang::HeaderSearch  headerSearch(headerSearchOptions, sourceManager, diagnosticEngine, langOptions, targetInfo);

    clang::CompilerInstance  compilerInstance;

    clang::Preprocessor   preprocessor(
        preprocessorOptions,
        diagnosticEngine,
        langOptions,
        sourceManager,
        memoryBufferCache,
        headerSearch,
        compilerInstance
    );
    preprocessor.Initialize(*targetInfo);

    preprocessor.EnterMainSourceFile();
    textDiagnosticPrinter->BeginSourceFile(langOptions, &preprocessor);

    clang::Token token;

    std::list<clang::Token>  tokens;

    do {

        preprocessor.Lex(token);

        if (diagnosticEngine.hasErrorOccurred())
        {
            NOT_IMPLEMENTED();
        }

        tokens.push_back(token);

    } while (!token.is(clang::tok::eof));

    textDiagnosticPrinter->EndSourceFile();

    TypeEval  exprEval(ScopePtr(new ScopeList()), typeInfoProvider, &tokens);

    return exprEval.getResult();
}

///////////////////////////////////////////////////////////////////////////////

ExprEval::ExprEval(const ScopePtr& scope,
    const TypeInfoProviderPtr& typeInfoProvider,
    std::list<clang::Token>* tokens,
    clang::tok::TokenKind  endToken)
{
    m_scope = scope;
    m_typeInfoProvider = typeInfoProvider;
    m_tokens = tokens;
    m_endToken = endToken;
}

///////////////////////////////////////////////////////////////////////////////

TypedValue  ExprEval::getResult()
{
    std::list<TypedValue>  operands;
    std::list<BinOperation*> operations;

    std::unique_ptr<UnaryOperation>  ternaryOp;

    while ( true )
    {
        UnaryOperation*  preOp = 0;
        std::list<UnaryOperation*>  preOperations;
        while (0 != (preOp = getPreUnaryOperation()))
        {
            preOperations.push_front(preOp);
        }

        auto  val = getOperand();

        UnaryOperation*  postOp = 0;
        std::list<UnaryOperation*>  postOperations;
        while (0 != (postOp = getPostUnaryOperation()))
        {
            postOperations.push_back(postOp);
        }

        for (auto op : postOperations)
        {
            val = op->getResult(val);
            delete op;
        }

        for (auto op : preOperations)
        {
            val = op->getResult(val);
            delete op;
        }

        operands.push_back(val);

        auto token = m_tokens->front();

        if (token.is(clang::tok::question))
        {
            ternaryOp.reset(getTernaryOperation());
            break;
        }

        if (token.is(m_endToken))
        {
            m_tokens->pop_front();
            break;
        }

        operations.push_back(getOperation());
    }

    while (!operations.empty())
    {
        auto  op = std::max_element(operations.begin(), operations.end(),
            [](const BinOperation* op1, const BinOperation* op2)
            {
                return op1->getPriority() > op2->getPriority();
            });

        auto op1 = operands.begin();
        std::advance(op1, std::distance(operations.begin(), op));

        auto op2 = operands.begin();
        std::advance(op2, std::distance(operations.begin(), op) + 1);

        auto res = (*op)->getResult(*op1, *op2);

        delete *op;
        operations.erase(op);

        *op1 = res;
        operands.erase(op2);
    }

    if (ternaryOp)
        return ternaryOp->getResult(operands.front());

    return operands.front();

}

///////////////////////////////////////////////////////////////////////////////

TypedValue ExprEval::getOperand()
{
    if (m_tokens->empty())
        throw ExprException(L"error syntax");

    auto token = m_tokens->front();

    if (token.is(clang::tok::identifier))
    {
        return getIdentifierValue();
    }

    m_tokens->pop_front();

    if (token.is(clang::tok::numeric_constant))
    {
        return getNumericConst(token);
    }

    if (token.isOneOf(clang::tok::char_constant, clang::tok::wide_char_constant))
    {
        return getCharConst(token);
    }

    if (token.is(clang::tok::l_paren))
    {
        return ExprEval(m_scope, m_typeInfoProvider, m_tokens, clang::tok::r_paren).getResult();
    }

    if (token.is(clang::tok::kw_sizeof))
    {
        return getSizeof();
    }
    
    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

BinOperation* ExprEval::getOperation()
{
    if (m_tokens->empty())
        throw ExprException(L"error syntax");

    auto  token = m_tokens->front();

    if (!isBinOperation(token))
        throw ExprException(L"error syntax");

    m_tokens->pop_front();

    if (token.is(clang::tok::plus))
        return new AddExprOperation();

    if (token.is(clang::tok::star))
        return new MultExprOperation();

    if (token.is(clang::tok::minus))
        return new SubExprOperation();

    if (token.is(clang::tok::slash))
        return new DivExprOperation();

    if (token.is(clang::tok::percent))
        return new ModExprOperation();

    if (token.is(clang::tok::equalequal))
        return new EqualOperation();

    if (token.is(clang::tok::exclaimequal))
        return new NotEqualOperation();

    if (token.is(clang::tok::less))
        return new LessOperation();

    if (token.is(clang::tok::lessequal))
        return new LessEqualOperation();

    if (token.is(clang::tok::greater))
        return new GreaterOperation();

    if (token.is(clang::tok::greaterequal))
        return new GreaterEqualOperation();

    if (token.is(clang::tok::lessless))
        return new LeftShiftOperation();

    if (token.is(clang::tok::greatergreater))
        return new RightShiftOperation();

    if (token.is(clang::tok::amp))
        return new BitwiseAndOperation();

    if (token.is(clang::tok::pipe))
        return new BitwiseOrOperation();

    if (token.is(clang::tok::caret))
        return new BitwiseXorOperation();

    if (token.is(clang::tok::pipepipe))
        return new BoolOrOperation();

    if (token.is(clang::tok::ampamp))
        return new BoolAndOperation();

    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

UnaryOperation* ExprEval::getPreUnaryOperation()
{
    if (m_tokens->empty())
        throw ExprException(L"error syntax");

    auto  token = m_tokens->front();

    if (token.isOneOf(clang::tok::numeric_constant,
        clang::tok::identifier,
        clang::tok::char_constant,
        clang::tok::wide_char_constant) )
            return nullptr;

    if (token.is(clang::tok::l_paren))
        return getTypeCastOperation();

    if (token.is(clang::tok::kw_sizeof))
        return getSizeofOperation();

    if (isBaseTypeKeyWord(token))
        return nullptr;

    m_tokens->pop_front();

    if (token.is(clang::tok::minus))
        return new UnMinusOperation();

    if (token.is(clang::tok::plus))
        return new UnPlusOperation();

    if (token.is(clang::tok::star))
        return new DerefOperation();

    if (token.is(clang::tok::tilde))
        return new BitwiseNotOperation();

    if (token.is(clang::tok::exclaim))
        return new BooleanNotOperation();

    if (token.is(clang::tok::amp))
        return new RefOperation();

    throw ExprException(L"error syntax");

}

///////////////////////////////////////////////////////////////////////////////

UnaryOperation* ExprEval::getPostUnaryOperation()
{
    assert(!m_tokens->empty());

    auto  token = m_tokens->front();

    if (token.isOneOf(m_endToken, clang::tok::question))
        return nullptr;

    if (isBinOperation(token))
        return nullptr;

    if (token.is(clang::tok::l_square))
        return getArrayOperation();

    if (token.is(clang::tok::period))
        return getAttributeOperation();

    if (token.is(clang::tok::arrow))
        return getAttributeOperationPtr();

    throw ExprException(L"error syntax");
}

/////////////////////////////////////////////////////////////////////////////////

UnaryOperation* ExprEval::getArrayOperation()
{
    auto  token = m_tokens->front();

    if (!token.is(clang::tok::l_square))
        throw ExprException(L"error syntax");

    m_tokens->pop_front();

    auto indexVal = ExprEval(m_scope, m_typeInfoProvider, m_tokens, clang::tok::r_square).getResult();

    return new ArrayValueOperation(indexVal);
}

/////////////////////////////////////////////////////////////////////////////////

UnaryOperation* ExprEval::getAttributeOperation()
{
    auto  token = m_tokens->front();

    if (!token.is(clang::tok::period))
        throw ExprException(L"error syntax");

    m_tokens->pop_front();

    if (m_tokens->front().is(m_endToken))
        throw ExprException(L"error syntax");

    token = m_tokens->front();
    
    if (!token.is(clang::tok::identifier))
        throw ExprException(L"error syntax");

    std::string  name(token.getIdentifierInfo()->getNameStart(), token.getLength());

    m_tokens->pop_front();

    return new AttributeOperation(name);
}

/////////////////////////////////////////////////////////////////////////////////

UnaryOperation* ExprEval::getAttributeOperationPtr()
{
    auto  token = m_tokens->front();

    if (!token.is(clang::tok::arrow))
        throw ExprException(L"error syntax");

    m_tokens->pop_front();

    if (m_tokens->front().is(m_endToken))
        throw ExprException(L"error syntax");

    token = m_tokens->front();

    if (!token.is(clang::tok::identifier))
        throw ExprException(L"error syntax");

    std::string  name(token.getIdentifierInfo()->getNameStart(), token.getLength());

    m_tokens->pop_front();

    return new AttributeOperation(name);
}

/////////////////////////////////////////////////////////////////////////////////

UnaryOperation* ExprEval::getSizeofOperation()
{
    auto token = m_tokens->front();

    if ( !token.is( clang::tok::kw_sizeof ) )
        throw ExprException(L"error syntax");

    if ( std::next(m_tokens->begin())->is( m_endToken) )
        throw ExprException(L"error syntax");

    if (std::next(m_tokens->begin())->is(clang::tok::l_paren))
        return nullptr;

    m_tokens->pop_front();
        
    return new SizeofOperation();
}

///////////////////////////////////////////////////////////////////////////////

UnaryOperation* ExprEval::getTypeCastOperation()
{
    auto token = m_tokens->front();

    if (!token.is(clang::tok::l_paren))
        throw ExprException(L"error syntax");

    try {

        std::list<clang::Token>   exprCopy(m_tokens->begin(), m_tokens->end());

        exprCopy.pop_front();

        TypeInfoPtr  typeCast = TypeEval(m_scope, m_typeInfoProvider, &exprCopy, clang::tok::r_paren).getResult();

        *m_tokens = exprCopy;

        return new TypeCastOperation(typeCast);
    }
    catch (DbgException&)
    {
    }

    return nullptr;
}


///////////////////////////////////////////////////////////////////////////////

UnaryOperation* ExprEval::getTernaryOperation()
{
    auto token = m_tokens->front();

    if (!token.is(clang::tok::question))
        throw ExprException(L"error syntax");

    m_tokens->pop_front();

    TypedValue  val1 = ExprEval(m_scope, m_typeInfoProvider, m_tokens, clang::tok::colon).getResult();

    TypedValue  val2 = ExprEval(m_scope, m_typeInfoProvider, m_tokens, m_endToken).getResult();

    return new TrenaryOperation(val1, val2);
}

///////////////////////////////////////////////////////////////////////////////

size_t ExprEval::getSizeof()
{
    auto token = m_tokens->front();

    if (!token.is(clang::tok::l_paren))
        throw  ExprException(L"error syntax");

    m_tokens->pop_front();

    if (m_tokens->front().is(m_endToken))
        throw ExprException(L"error syntax");

    try {
        std::list<clang::Token>   exprCopy(m_tokens->begin(), m_tokens->end());
        TypedValue  exprResult = ExprEval(m_scope, m_typeInfoProvider, &exprCopy, clang::tok::r_paren).getResult();
        *m_tokens = exprCopy;
        return exprResult.getType()->getSize();
    }
    catch (DbgException&)
    {  }

    TypeInfoPtr  typeInfo = TypeEval(m_scope, m_typeInfoProvider, m_tokens, clang::tok::r_paren).getResult();
    if ( !typeInfo )
        throw ExprException(L"error syntax");

    return typeInfo->getSize();
}

///////////////////////////////////////////////////////////////////////////////

TypedValue ExprEval::getIdentifierValue()
{
    std::string  fullName;

    do {

        if (m_tokens->front().is(m_endToken))
            break;

        auto token = m_tokens->front();
 
        if (token.is(clang::tok::identifier))
        {
            m_tokens->pop_front();
            std::string  name(token.getIdentifierInfo()->getNameStart(), token.getLength());
            fullName += name;             
        }
        else if ( token.is(clang::tok::colon) )
        {
            m_tokens->pop_front();
            fullName += ":";
        }
        else
        {
            break;
        }

    } while (true);


    if (fullName == "true")
        return TypedValue(true);

    if (fullName == "false")
        return TypedValue(false);

    TypedValue   result;
    std::wstring  wname = strToWStr(fullName);
    if (m_scope->find(wname, result))
        return result;

    try {
        return evalType(fullName, m_typeInfoProvider)->getValue();
    }
    catch (DbgException&)
    {
    }

    throw  ExprException(L"error syntax");

}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr applyComplexModifierRecursive(const parser::ComplexMatcher& matcher, TypeInfoPtr& typeInfo_)
{
    TypeInfoPtr  typeInfo = typeInfo_;

    if (matcher.isPointer())
    {
        for ( auto p : matcher.getPointerMatcher().getPointerMatchers() )
            typeInfo = typeInfo->ptrTo();
    }

    if (matcher.isArray())
    {
        auto  indices = matcher.getArrayMatcher().getArrayIndices();
        for ( auto it = indices.rbegin(); it != indices.rend(); ++it)
        {
            auto i = getNumericConst(*it->getMatchResult().begin()).getValue().asLongLong();
            typeInfo = typeInfo->arrayOf(i);
        }
    }
  
    if (matcher.isNestedMatcher())
    {
        typeInfo = applyComplexModifierRecursive(matcher.getNestedMatcher().getInnerMatcher(), typeInfo);
    }

    return typeInfo;
}

///////////////////////////////////////////////////////////////////////////////

std::string getTypeModifierRecursive(const parser::ComplexMatcher& matcher)
{
    std::stringstream  sstr;

    if (matcher.isPointer())
    {
        for (auto p : matcher.getPointerMatcher().getPointerMatchers())
            sstr << '*';
    }

    if (matcher.isNestedMatcher())
    {
        sstr << '(' << getTypeModifierRecursive(matcher.getNestedMatcher().getInnerMatcher()) << ')';
    }
    
    if (matcher.isArray())
    {
        auto  indices = matcher.getArrayMatcher().getArrayIndices();
        for (auto it = indices.rbegin(); it != indices.rend(); ++it)
        {
            auto i = getNumericConst(*it->getMatchResult().begin()).getValue().asLongLong();
            sstr << '[' << std::dec << i << ']';
        }
    }

    return sstr.str();
}

//////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeEval::getResult()
{
    using namespace parser;

    auto endIt = 
        std::find_if(m_tokens->cbegin(), m_tokens->cend(), [this](auto& token)
        {   
             return token.is(this->m_endToken); 
    });

    std::list<clang::Token>  tokens;
    std::copy_if(m_tokens->cbegin(), endIt, std::back_inserter(tokens), [](auto& token){
        return !token.is(clang::tok::kw_const);
    });

    for (auto it = tokens.begin(); it != tokens.end(); ++it)
    {
        if (it->is(clang::tok::greatergreater))
        {
            auto &t = *it;
            t.setKind(clang::tok::greater);
            it = tokens.insert(it, t);
        }
    }

    TypeMatcher  typeMatcher;

    auto matchResult = typeMatcher.match(std::make_pair(tokens.cbegin(), tokens.cend()));

    if (!matchResult.isMatched() || matchResult.end() != tokens.cend())
        throw  ExprException(L"error syntax");

    TypeInfoPtr  typeInfo;

    if (typeMatcher.isBasedType())
    {
        typeInfo = getBaseType();
    }
    else if (typeMatcher.isStandardIntType())
    {
        typeInfo = getStandardIntType();
    }
    else if (typeMatcher.isCustomType())
    {
        typeInfo = getCustomType(typeMatcher.getCustomMatcher());
    }
    else
    {
        NOT_IMPLEMENTED();
    }

    if (!typeInfo)
        throw  ExprException(L"error syntax");

    if (typeMatcher.isComplexType() )
    {
        typeInfo = applyComplexModifierRecursive(typeMatcher.getComplexMather(), typeInfo);
    }  

    m_tokens->erase(m_tokens->begin(), ++endIt);

    return typeInfo;
}

///////////////////////////////////////////////////////////////////////////////

std::string TypeEval::getTemplateName(const parser::TemplateMatcher& templateMatcher)
{
    std::string  templateName;

    for (auto& arg : templateMatcher.getTemplateArgs())
    {
        if (!templateName.empty())
            templateName += ',';

        if (arg.isType())
        {
            templateName += getTypeName(arg.getTypeMatcher());
        }
        else if (arg.isNumeric())
        {
            auto  value = ExprEval2(m_scope, m_typeInfoProvider, arg.getNumericMatcher().getMatchResult().getMatchedRange()).getResult();
            templateName += std::to_string(value.getValue().asLongLong());
        }
        else
        {
            throw  ExprException(L"error syntax");
        }
    }

    templateName.insert(templateName.begin(), '<');
    if (templateName.back() == '>')
        templateName.insert(templateName.end(), ' ');
    templateName.insert(templateName.end(), '>');

    return templateName;
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeEval::getCustomType(const parser::CustomTypeMatcher&  customMatcher)
{
    TypeInfoPtr  typeInfo;

    std::string   typeName = customMatcher.getTypeNameMatcher().getName();

    if (customMatcher.isTemplate())
    {
        for (auto namespaceMatcher : customMatcher.getTypeNameMatcher().getNamespaces())
        {
            typeName += "::";
            typeName += namespaceMatcher.getName();
        }

        typeName += getTemplateName(customMatcher.getTemplateMatcher());

        typeInfo = m_typeInfoProvider->getTypeByName(strToWStr(typeName));

        for (auto fieldMatcher : customMatcher.getFields())
        {
            auto fieldName = fieldMatcher.getName();
            typeInfo = typeInfo->getElement(strToWStr(fieldName));
        }
    }
    else
    {
        try 
        {
            typeInfo = m_typeInfoProvider->getTypeByName(strToWStr(typeName));
        }
        catch (SymbolException&)
        {}

        for (auto namespaceMatcher : customMatcher.getTypeNameMatcher().getNamespaces())
        {

            if (typeInfo)
            {
                auto fieldName = namespaceMatcher.getName();
                typeInfo = typeInfo->getElement(strToWStr(fieldName));
            }
            else
            {
                typeName += "::";
                typeName += namespaceMatcher.getName();

                try 
                {
                    typeInfo = m_typeInfoProvider->getTypeByName(strToWStr(typeName));
                }
                catch (SymbolException&)
                { }
            }
        }
    }
    
    return typeInfo;
}

///////////////////////////////////////////////////////////////////////////////

std::string TypeEval::getTypeName(const parser::TypeMatcher& typeMatcher)
{
    std::string  typeName;

    if (typeMatcher.isBasedType())
    {
        typeName = getBaseTypeName(typeMatcher.getBaseTypeMatcher());
    }
    else
    if (typeMatcher.isCustomType())
    {
        typeName = getCustomTypeName(typeMatcher.getCustomMatcher());
    }

    if (typeMatcher.isComplexType())
    {
        typeName += ' ';
        typeName += getTypeModifierRecursive(typeMatcher.getComplexMather());
    }

    return typeName;
}

///////////////////////////////////////////////////////////////////////////////

std::string TypeEval::getCustomTypeName(const parser::CustomTypeMatcher& customMatcher)
{
    std::string   typeName = customMatcher.getTypeNameMatcher().getName();

    if (customMatcher.isTemplate())
    {
        for (auto namespaceMatcher : customMatcher.getTypeNameMatcher().getNamespaces())
        {
            typeName += "::";
            typeName += namespaceMatcher.getName();
        }

        typeName += getTemplateName(customMatcher.getTemplateMatcher());

        for (auto fieldMatcher : customMatcher.getFields())
        {
            typeName += "::";
            typeName += fieldMatcher.getName();
        }
    }
    else
    {
        for (auto namespaceMatcher : customMatcher.getTypeNameMatcher().getNamespaces())
        {
            typeName += "::";
            typeName += namespaceMatcher.getName();
        }
    }

    return typeName;
}

///////////////////////////////////////////////////////////////////////////////

std::string  TypeEval::getBaseTypeName(const parser::BaseTypeMatcher& baseTypeMatcher)
{
    std::unique_ptr<BaseTypeBuilder>  baseTypeBuilder(new EmptyBaseTypeBuilder);

    for (auto& token : baseTypeMatcher.getMatchResult())
    {
        if (token.is(clang::tok::kw_int))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addInt());
        }
        else
        if (token.is(clang::tok::kw_char))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addChar());
        }
        else
        if (token.is(clang::tok::kw_short))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addShort());
        }
        else
        if (token.is(clang::tok::kw_unsigned))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addUnsigned());
        }
        else
        if (token.is(clang::tok::kw_signed))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addSigned());
        }
        else
        if (token.is(clang::tok::kw_long))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addLong());
        }
        else
        if (isInt64KeyWord(token))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addLong());
            baseTypeBuilder.reset(baseTypeBuilder->addLong());
        }
        else
        if (token.is(clang::tok::kw_float))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addFloat());
        }
        else
        if (token.is(clang::tok::kw_double))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addDouble());
        }
        else
        {
            throw ExprException(L"error syntax");
        }
    }

    return baseTypeBuilder->getTypeName();
}

///////////////////////////////////////////////////////////////////////////////


TypeInfoPtr TypeEval::getBaseType()
{
    std::unique_ptr<BaseTypeBuilder>  baseTypeBuilder(new EmptyBaseTypeBuilder);

    while (true)
    {
        auto  token = m_tokens->front();

        if (token.is(m_endToken))
            break;
   
        if (token.is(clang::tok::kw_int))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addInt());
            m_tokens->pop_front();
            continue;
        }

        if (token.is(clang::tok::kw_char))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addChar());
            m_tokens->pop_front();
            continue;
        }

        if (token.is(clang::tok::kw_short))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addShort());
            m_tokens->pop_front();
            continue;
        }

        if (token.is(clang::tok::kw_unsigned))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addUnsigned());
            m_tokens->pop_front();
            continue;
        }

        if (token.is(clang::tok::kw_signed))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addSigned());
            m_tokens->pop_front();
            continue;
        }

        if (token.is(clang::tok::kw_long))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addLong());
            m_tokens->pop_front();
            continue;
        }

        if (isInt64KeyWord(token))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addLong());
            baseTypeBuilder.reset(baseTypeBuilder->addLong());
            m_tokens->pop_front();
            continue;
        }

        if (token.is(clang::tok::kw_float))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addFloat());
            m_tokens->pop_front();
            continue;
        }

        if (token.is(clang::tok::kw_double))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addDouble());
            m_tokens->pop_front();
            continue;
        }

        if (token.is(clang::tok::kw_const))
        {
            m_tokens->pop_front();
            continue;
        }

        if (token.isOneOf(clang::tok::star, clang::tok::l_paren, clang::tok::l_square, 
            clang::tok::amp, clang::tok::ampamp))
            break;

        throw ExprException(L"error syntax");
    }

    return baseTypeBuilder->getResult();
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeEval::getStandardIntType()
{
    auto  token = m_tokens->front();
    m_tokens->pop_front();

    assert(token.is(clang::tok::identifier));
 
    std::string  name(token.getIdentifierInfo()->getNameStart(), token.getLength());

    return kdlib::getStandardIntType(name);
}

///////////////////////////////////////////////////////////////////////////////

static const std::regex integralLiteralRe("(0x[0-9a-f]+|0[0-7]+|[1-9][0-9]*|0)((l)|(u)|(ul|lu)|(ll)|(ull|llu))?", std::regex::icase);
static const std::regex floatLiteralRe("\\d*((\\.(\\d+(e[+\\-]?\\d+)?)?)|(e[+\\-]?\\d+))((f)|(l))?", std::regex::icase);

TypedValue  getNumericConst(const clang::Token& token)
{
    std::string  strVal(token.getLiteralData(), token.getLength());

    std::smatch  smatch;

    if (std::regex_match(strVal, smatch, integralLiteralRe))
    {
        bool  isSuffix = smatch[2].matched;
        bool  isLong = smatch[3].matched;
        bool  isUnsigned = smatch[4].matched;
        bool  isULong = smatch[5].matched;
        bool  isLongLong = smatch[6].matched;
        bool  isULongLong = smatch[7].matched;

        if (!isSuffix || isLong )
        {
            try {
                return std::stol(strVal, nullptr, 0);
            } 
            catch (std::out_of_range&)
            {}

            try {
                return std::stoul(strVal, nullptr, 0);
            }
            catch (std::out_of_range&)
            {}

            try {
                return std::stoll(strVal, nullptr, 0);
            }
            catch (std::out_of_range&)
            {}

            try {
                return std::stoull(strVal, nullptr, 0);
            }
            catch (std::out_of_range&)
            {}

            throw ExprException(L"error syntax");
        }
        else
        {
            if (isUnsigned || isULong )
            {

                try {
                    return std::stoul(strVal, nullptr, 0);
                }
                catch (std::out_of_range&)
                {}

                try {
                    return std::stoull(strVal, nullptr, 0);
                }
                catch (std::out_of_range&)
                {}

                throw ExprException(L"error syntax");
            }
            else if (isLongLong)
            {
                try {
                    return std::stoll(strVal, nullptr, 0);
                }
                catch (std::out_of_range&)
                {
                }

                throw ExprException(L"error syntax");
            }
            else if (isULongLong)
            {
                try {
                    return std::stoull(strVal, nullptr, 0);
                }
                catch (std::out_of_range&)
                {
                }

                throw ExprException(L"error syntax");
            }

            assert(0);
        }
    }
    else if (std::regex_match(strVal, smatch, floatLiteralRe))
    {
        bool isLong = !smatch[7].matched;

        try {
            if (isLong)
                return std::stod(strVal);
            else
                return std::stof(strVal);
        }
        catch (std::exception&)
        {
        }

        throw ExprException(L"error syntax");
    }

    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

static const std::regex charLiteralRe("(L'(.).*')|('(.{1,4})')", std::regex::icase);

TypedValue getCharConst(const clang::Token& token)
{
    std::string  strVal(token.getLiteralData(), token.getLength());

    std::smatch  smatch;

    const int  a = 'ff';

    if (std::regex_match(strVal, smatch, charLiteralRe))
    {
        if (smatch[1].matched)
        {
            return strToWStr(smatch[2].str())[0];
        }

        auto  bytesSeq = smatch[4].str();
        if (bytesSeq.size() == 1)
            return bytesSeq[0];

        int  intRes = 0;
        for (auto byte : bytesSeq)
            intRes = ( intRes << 8 ) | byte;

        return intRes;
    }

    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

std::string  tokenToStr(const clang::Token& token)
{
    switch (token.getKind())
    {
    case clang::tok::identifier:
        return std::string(token.getIdentifierInfo()->getNameStart(), token.getLength());
    case clang::tok::numeric_constant:
        return std::string(token.getLiteralData(), token.getLength());       
    case clang::tok::less:
        return "<";
    case clang::tok::greater:
        return ">";
    case clang::tok::kw_bool:
        return "bool";
    case clang::tok::kw_char:
        return "char";
    case clang::tok::kw_char16_t:
        return "char16_t";
    case clang::tok::kw_char32_t:
        return "char32_t";
    case clang::tok::kw_double:
        return "double";
    case clang::tok::kw_float:
        return "float";
    case clang::tok::kw_int:
        return "int";
    case clang::tok::kw_long:
        return "long";
    case clang::tok::kw_short:
        return "short";
    case clang::tok::kw_signed:
        return "signed";
    case clang::tok::kw_unsigned:
        return "unsigned";
    case clang::tok::kw_void:
        return "void";
    case clang::tok::kw_wchar_t:
        return "wchar_t";
    case clang::tok::kw_struct:
        return "struct";
    case clang::tok::minus:
        return "-";
    case clang::tok::plus:
        return "+";
    case clang::tok::colon:
        return ":";
    case clang::tok::star:
        return "*";
    case clang::tok::l_square:
        return "[";
    case clang::tok::r_square:
        return "]";
    case clang::tok::l_paren:
        return "(";
    case clang::tok::r_paren:
        return ")";
    case clang::tok::amp:
        return "&";
    case clang::tok::ampamp:
        return "&&";
    case clang::tok::comma:
        return ",";
    }

    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////

bool isBaseTypeKeyWord(const clang::Token& token)
{
    if (token.isOneOf(
        clang::tok::kw_bool,
        clang::tok::kw_char,
        clang::tok::kw_char16_t,
        clang::tok::kw_char32_t,
        clang::tok::kw_double,
        clang::tok::kw_float,
        clang::tok::kw_int,
        clang::tok::kw_long,
        clang::tok::kw_short,
        clang::tok::kw_signed,
        clang::tok::kw_unsigned,
        clang::tok::kw_void,
        clang::tok::kw_wchar_t
    )) return true;

    return isInt64KeyWord(token);
}

///////////////////////////////////////////////////////////////////////////////

static const std::map< std::string, std::function<kdlib::TypeInfoPtr(void)>  >  standardIntTypes = {
    { "wchar_t", []() { return kdlib::loadType(L"Int2B"); }  },
    { "int8_t", []() { return kdlib::loadType(L"Int1B"); } },
    { "uint8_t", []() { return kdlib::loadType(L"UInt1B"); } },
    { "int16_t", []() { return kdlib::loadType(L"Int2B"); } },
    { "uint16_t",[]() { return kdlib::loadType(L"UInt2B"); } },
    { "int32_t", []() { return kdlib::loadType(L"Int4B"); } },
    { "uint32_t", []() { return kdlib::loadType(L"UInt4B"); } },
    { "int64_t", []() { return kdlib::loadType(L"Int8B"); } },
    { "uint64_t", []() { return kdlib::loadType(L"UInt8B"); } },
    { "size_t", []() { return kdlib::ptrSize() == 4 ? kdlib::loadType(L"UInt4B") : kdlib::loadType(L"UInt8B");  } },
    { "intptr_t", []() { return kdlib::ptrSize() == 4 ? kdlib::loadType(L"Int4B") : kdlib::loadType(L"Int8B");  } },
    { "uintptr_t", []() { return kdlib::ptrSize() == 4 ? kdlib::loadType(L"UInt4B") : kdlib::loadType(L"UInt8B");  } },
    { "bool", []() { return kdlib::loadType(L"Bool");  } }
};

bool isStandardIntType(const clang::Token& token)
{
    if (token.is(clang::tok::identifier))
    {
        std::string  name(token.getIdentifierInfo()->getNameStart(), token.getLength());

        return standardIntTypes.find(name) != standardIntTypes.end();
    }

    return false;
}

kdlib::TypeInfoPtr getStandardIntType(const std::string& name)
{
    return standardIntTypes.at(name)();
}

///////////////////////////////////////////////////////////////////////////////

bool isBinOperation(const clang::Token& token)
{
    return token.isOneOf(
        clang::tok::plus,
        clang::tok::star,
        clang::tok::minus,
        clang::tok::slash,
        clang::tok::percent,
        clang::tok::equalequal,
        clang::tok::exclaimequal,
        clang::tok::less,
        clang::tok::lessequal,
        clang::tok::greater,
        clang::tok::greaterequal,
        clang::tok::lessless,
        clang::tok::greatergreater,
        clang::tok::amp,
        clang::tok::pipe,
        clang::tok::caret,
        clang::tok::pipepipe,
        clang::tok::ampamp
    );
}



///////////////////////////////////////////////////////////////////////////////

bool isInt64KeyWord(const clang::Token& token)
{
    if (token.is(clang::tok::identifier))
    {
        std::string  name(token.getIdentifierInfo()->getNameStart(), token.getLength());
        return name == "__int64";
    }

    return false;
}


///////////////////////////////////////////////////////////////////////////////

std::string  getIdentifier(const clang::Token& token)
{
    return std::string(token.getIdentifierInfo()->getNameStart(), token.getLength());
}

///////////////////////////////////////////////////////////////////////////////

bool isOperationToken(const clang::Token& token)
{
    return (token.isOneOf(
        clang::tok::minus,
        clang::tok::plus,
        clang::tok::exclaim,
        clang::tok::plusplus,
        clang::tok::minusminus,
        clang::tok::tilde
    ));
}

///////////////////////////////////////////////////////////////////////////////

TypedValue ExprEval2::getResult()
{
    using namespace parser;

    ConstExpressionMatcher  exprMatcher;

    auto matchResult = exprMatcher.match(m_tokenRange);

    if (!matchResult.isMatched() || matchResult.end() != m_tokenRange.second)
        throw  ExprException(L"error syntax");

    TypedValue  value = getNumericConst(*exprMatcher.getOperand().getNumeric().getMatchResult().begin());

    for (auto op : exprMatcher.getLeftOps())
    {
        auto opToken = *op.getMatchResult().begin();
        std::unique_ptr< UnaryOperation>  op;

        if (opToken.is(clang::tok::minus))
            op.reset(new UnMinusOperation());
        else if (opToken.is(clang::tok::plus))
            op.reset(new UnPlusOperation());
        else if (opToken.is(clang::tok::exclaim))
            op.reset(new BooleanNotOperation());
        else if (opToken.is(clang::tok::plusplus))
            op.reset(new PreIncrementOperation());
        else if (opToken.is(clang::tok::tilde))
            op.reset(new BitwiseNotOperation());
        else
            throw ExprException(L"error syntax");

        value = op->getResult(value);
    }
        
    return value;
}


///////////////////////////////////////////////////////////////////////////////

TypedValue TypeCastOperation::getResult(const TypedValue& val)
{
    if (m_type->isBase())
    {
        return castToBase(val);
    }
    else if (m_type->isPointer())
    {
        return castToPtr(val);
    }
    else if (m_type->isArray())
    {
        throw ExprException(L"error syntax");
    }
    else if (m_type->isUserDefined())
    {
        throw ExprException(L"error syntax");
    }

    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////

TypedValue TypeCastOperation::castToBase(const TypedValue& val)
{
    NumVariant  srcValue;
    if (val.getType()->isBase())
        srcValue = val.getValue();
    else if (val.getType()->isPointer())
        srcValue = val.getValue();
    else if (val.getType()->isArray())
        srcValue = val.getAddress();
    else
        throw ExprException(L"error syntax");

    if (m_type->getName() == L"Char")
        return srcValue.asChar();

    if (m_type->getName() == L"WChar")
        return srcValue.asShort();

    if (m_type->getName() == L"Long")
        return srcValue.asLong();

    if (m_type->getName() == L"ULong")
        return srcValue.asULong();

    if (m_type->getName() == L"Int1B")
        return srcValue.asChar();

    if (m_type->getName() == L"UInt1B")
        return srcValue.asUChar();

    if (m_type->getName() == L"Int2B")
        return srcValue.asShort();

    if (m_type->getName() == L"UInt2B")
        return srcValue.asUShort();

    if (m_type->getName() == L"Int4B")
        return srcValue.asLong();

    if (m_type->getName() == L"UInt4B")
        return srcValue.asULong();

    if (m_type->getName() == L"Int8B")
        return srcValue.asLongLong();

    if (m_type->getName() == L"UInt8B")
        return srcValue.asULongLong();

    if (m_type->getName() == L"Float")
        return srcValue.asFloat();

    if (m_type->getName() == L"Double")
        return srcValue.asDouble();

    if (m_type->getName() == L"Bool")
        return srcValue.asBool();

    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////

TypedValue TypeCastOperation::castToPtr(const TypedValue& val)
{
    NumVariant  srcValue;
    if (val.getType()->isBase())
    {
        return TypedValue(val.getValue().asULongLong()).castTo(m_type);
    }

    if (val.getType()->isPointer())
    {
        return val.castTo(m_type);
    }

    if (val.getType()->isArray())
    {
        return TypedValue(val.getAddress()).castTo(m_type);
    }

    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////

TypedValue AddExprOperation::getResult(const TypedValue& val1, const TypedValue& val2)
{
    if (val1.getType()->isBase() && val2.getType()->isBase())
        return val1 + val2;

    if (val1.getType()->isPointer() || val1.getType()->isArray())
    {
        if (val2.getType()->isBase())
            return TypedValue(val1 + val1.getType()->deref()->getSize() * val2).castTo(val1.getType()->deref()->ptrTo());
        throw ExprException(L"error syntax");
    }

    if (val2.getType()->isPointer() || val2.getType()->isArray())
    {
        if (val1.getType()->isBase())
            return TypedValue(val2 + val2.getType()->deref()->getSize() * val1).castTo(val2.getType()->deref()->ptrTo());
        throw ExprException(L"error syntax");
    }

    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

TypedValue SubExprOperation::getResult(const TypedValue& val1, const TypedValue& val2)
{
    if (val1.getType()->isBase() && val2.getType()->isBase())
        return val1 - val2;

    if (val1.getType()->isPointer() || val1.getType()->isArray())
    {
        if (val2.getType()->isBase())
            return TypedValue(val1 - val1.getType()->deref()->getSize() * val2).castTo(val1.getType()->deref()->ptrTo());

        if (val2.getType()->isPointer() || val2.getType()->isArray())
            return (val1 - val2) / (val1.getType()->deref()->getSize());

        throw ExprException(L"error syntax");
    }

    throw ExprException(L"error syntax");

}

///////////////////////////////////////////////////////////////////////////////

TypedValue MultExprOperation::getResult(const TypedValue& val1, const TypedValue& val2) 
{
    if (val1.getType()->isBase() && val2.getType()->isBase())
        return val1 * val2;

    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

TypedValue DivExprOperation::getResult(const TypedValue& val1, const TypedValue& val2)
{
    if (val1.getType()->isBase() && val2.getType()->isBase())
        return val1 / val2;

    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

TypedValue ModExprOperation::getResult(const TypedValue& val1, const TypedValue& val2)
{
    if (val1.getType()->isBase() && val2.getType()->isBase())
        return val1 % val2;

    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

TypedValue LeftShiftOperation::getResult(const TypedValue& val1, const TypedValue& val2)
{
    if (val1.getType()->isBase() && val2.getType()->isBase())
        return val1 << val2;

    throw ExprException(L"error syntax");
}


///////////////////////////////////////////////////////////////////////////////

TypedValue RightShiftOperation::getResult(const TypedValue& val1, const TypedValue& val2)
{
    if (val1.getType()->isBase() && val2.getType()->isBase())
        return val1 >> val2;

    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

TypedValue BitwiseOrOperation::getResult(const TypedValue& val1, const TypedValue& val2)
{
    if (val1.getType()->isBase() && val2.getType()->isBase())
        return val1 | val2;

    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

TypedValue BitwiseAndOperation::getResult(const TypedValue& val1, const TypedValue& val2)
{
    if (val1.getType()->isBase() && val2.getType()->isBase())
        return val1 & val2;

    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

TypedValue BitwiseXorOperation::getResult(const TypedValue& val1, const TypedValue& val2)
{
    if (val1.getType()->isBase() && val2.getType()->isBase())
        return val1 ^ val2;

    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

TypedValue UnMinusOperation::getResult(const TypedValue& val)
{
    if (val.getType()->isBase())
        return -val;

    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

TypedValue BitwiseNotOperation::getResult(const TypedValue& val)
{
    if (val.getType()->isBase())
        return ~val;

    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

TypedValue BooleanNotOperation::getResult(const TypedValue& val)
{
    if (val.getType()->isBase())
        return !val.getValue();

    throw ExprException(L"error syntax");
}

///////////////////////////////////////////////////////////////////////////////

}