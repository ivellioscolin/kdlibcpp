#pragma once

#include "clang/Lex/Token.h"

#include "kdlib/typedvar.h"
#include "kdlib/exceptions.h"

#include "strconvert.h"
#include "typeparser.h"
#include "exprparser.h"

namespace kdlib {


class BinOperation {
public:
    BinOperation() {}
    BinOperation(BinOperation&) = delete;
    virtual TypedValue getResult(const TypedValue& val1, const TypedValue& val2) = 0;
    virtual int getPriority() const = 0;
};

class UnaryOperation {
public:
    UnaryOperation()
    {}
    UnaryOperation(UnaryOperation&) = delete;
    virtual TypedValue getResult(const TypedValue& val) = 0;
};
 

class ExprException : public TypeException
{
public:
    ExprException(const std::wstring& desc) : TypeException(desc)
    {}
};

class UnMinusOperation : public UnaryOperation
{
public:
    TypedValue getResult(const TypedValue& val) override;
};

class UnPlusOperation : public UnaryOperation
{
public:
    TypedValue getResult(const TypedValue& val) override
    {
        return val;
    }
};

class DerefOperation : public UnaryOperation
{
public:
    TypedValue getResult(const TypedValue& val) override
    {
        TypedValue  v = val;
        return v.deref();
    }
};


class RefOperation : public UnaryOperation
{
public:
    TypedValue getResult(const TypedValue& val) override
    {
        return makePointer(val.getType(), val.getAddress());
    }
};

class BitwiseNotOperation : public UnaryOperation
{
public:
    TypedValue getResult(const TypedValue& val) override;
};

class BooleanNotOperation : public UnaryOperation
{
public:
    TypedValue getResult(const TypedValue& val) override;
};

class PreIncrementOperation : public UnaryOperation
{
public:
    TypedValue getResult(const TypedValue& val) override
    {
        return val.getValue() + 1;
    }
};

class ArrayValueOperation : public UnaryOperation
{
public:

    ArrayValueOperation(const NumVariant&  value) :
        m_index(value)
    {}

private:

    TypedValue getResult(const TypedValue& val) override
    {
        TypedValue  v = val;
        return v.getElement(m_index);
    }

    size_t  m_index;
};

class AttributeOperation : public UnaryOperation
{
public:

    AttributeOperation(const std::string  attrName) :
        m_attributeName(attrName)
    {}

protected:

    TypedValue getResult(const TypedValue& val) override
    {
        TypedValue  v = val;
        return v.getElement(strToWStr(m_attributeName));
    }

    std::string  m_attributeName;
};

class AttributeOperationPtr : public AttributeOperation
{

private:

    TypedValue getResult(const TypedValue& val) override
    {
        TypedValue  v = val;
        return v.deref()->getElement(strToWStr(m_attributeName));
    }
};

class SizeofOperation : public UnaryOperation
{

private:

    TypedValue getResult(const TypedValue& val) override
    {
        return val.getType()->getSize();
    }
};


class TypeCastOperation : public UnaryOperation
{
public:

    TypeCastOperation(const TypeInfoPtr& ptr) :
        m_type(ptr)
    {}

    TypedValue getResult(const TypedValue& val);

private:

    TypedValue castToBase(const TypedValue& val);
    TypedValue castToPtr(const TypedValue& val);

    TypeInfoPtr   m_type;
};

class TrenaryOperation : public UnaryOperation
{
public:
    TrenaryOperation(const TypedValue& v1, const TypedValue& v2) :
        m_value1(v1), m_value2(v2)
    {}

    TypedValue getResult(const TypedValue& val) override
    {
        return val ? m_value1 : m_value2;
    }

private:

    TypedValue  m_value1;

    TypedValue  m_value2;

};


enum BinOpearationPriority
{
    Mul = 0,
    Div = 0,
    Mod = 0,
    Add = 1,
    Sub = 1,
    LeftShift = 2,
    RightShift = 2,
    Less = 4,
    LessEqual = 4,
    Greater = 4,
    GreaterEqual = 4,
    Equal = 5,
    NotEqual = 5,
    BitwiseAnd = 6,
    BitwiseOr = 7,
    BitwiseXor = 8,
    BooleanAnd = 9,
    BooleanOr = 10
};

class AddExprOperation : public BinOperation
{
    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override;

    int getPriority() const override
    {
        return BinOpearationPriority::Add;
    }
};


class SubExprOperation : public BinOperation 
{

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override;

    int getPriority() const override
    {
        return BinOpearationPriority::Sub;
    }
};

class MultExprOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override;

    int getPriority() const override
    {
        return BinOpearationPriority::Mul;
    }
};

class DivExprOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override;

    int getPriority() const override
    {
        return BinOpearationPriority::Div;
    }
};

class ModExprOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override;

    int getPriority() const override
    {
        return BinOpearationPriority::Mod;
    }
};

class EqualOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override
    {
        return val1 == val2;
    }

    int getPriority() const override
    {
        return BinOpearationPriority::Equal;
    }
};

class NotEqualOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override
    {
        return val1 != val2;
    }

    int getPriority() const override
    {
        return BinOpearationPriority::NotEqual;
    }
};

class LessOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override
    {
        return val1 < val2;
    }

    int getPriority() const override
    {
        return BinOpearationPriority::NotEqual;
    }
};

class LessEqualOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override
    {
        return val1 <= val2;
    }

    int getPriority() const override
    {
        return BinOpearationPriority::LessEqual;
    }
};

class GreaterOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override
    {
        return val1 > val2;
    }

    int getPriority() const override
    {
        return BinOpearationPriority::Greater;
    }
};

class GreaterEqualOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override
    {
        return val1 >= val2;
    }

    int getPriority() const override
    {
        return BinOpearationPriority::GreaterEqual;
    }
};

class LeftShiftOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override;

    int getPriority() const override
    {
        return BinOpearationPriority::LeftShift;
    }
};

class RightShiftOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override;

    int getPriority() const override
    {
        return BinOpearationPriority::RightShift;
    }
};

class BitwiseAndOperation: public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override;

    int getPriority() const override
    {
        return BinOpearationPriority::BitwiseAnd;
    }

};

class BitwiseOrOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2);

    int getPriority() const override
    {
        return BinOpearationPriority::BitwiseOr;
    }

};

class BitwiseXorOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override;

    int getPriority() const override
    {
        return BinOpearationPriority::BitwiseXor;
    }

};


class BoolOrOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override
    {
        return val1.getValue() || val2.getValue();
    }

    int getPriority() const override
    {
        return BinOpearationPriority::BooleanOr;
    }

};

class BoolAndOperation : public BinOperation {

    TypedValue getResult(const TypedValue& val1, const TypedValue& val2) override
    {
        return val1.getValue() && val2.getValue();
    }

    int getPriority() const override
    {
        return BinOpearationPriority::BooleanAnd;
    }
};


///////////////////////////////////////////////////////////////////////////////

class ExprEval {

public:

    ExprEval(
        const ScopePtr& scope,
        const TypeInfoProviderPtr& typeInfoProvider,
        std::list<clang::Token>* tokens,
        clang::tok::TokenKind  endToken = clang::tok::eof
        );

    TypedValue  getResult();

private:

    ScopePtr   m_scope;
    TypeInfoProviderPtr  m_typeInfoProvider;
    std::list<clang::Token>*  m_tokens;
    clang::tok::TokenKind  m_endToken;

    TypedValue getOperand();
    BinOperation *getOperation();
    UnaryOperation* getPreUnaryOperation();
    UnaryOperation* getPostUnaryOperation();
    UnaryOperation* getArrayOperation();
    UnaryOperation* getAttributeOperation();
    UnaryOperation* getAttributeOperationPtr();
    UnaryOperation* getSizeofOperation();
    UnaryOperation* getTypeCastOperation();
    UnaryOperation* getTernaryOperation();

    size_t getSizeof();

    TypedValue getIdentifierValue();

    TypeInfoPtr getTypeDeclaration();
    TypeInfoPtr getBasedTypeDeclaration();
 };

///////////////////////////////////////////////////////////////////////////////

class TypeModifier
{
public:

    virtual TypeInfoPtr getResult(TypeInfoPtr& origType) = 0;
};

class TypePtrModifier : public TypeModifier
{
    TypeInfoPtr getResult(TypeInfoPtr& origType) {
        return origType->ptrTo();
    }
};

class TypeArrayModifier : public TypeModifier
{

public:

    explicit TypeArrayModifier(size_t  arraySize) : m_arraySize(arraySize)
    {}


private:

    TypeInfoPtr getResult(TypeInfoPtr& origType) {
        return origType->arrayOf(m_arraySize);
    }

    size_t  m_arraySize;
};

class TypeEval {

public:
       
    TypeEval(const ScopePtr& scope,
        const TypeInfoProviderPtr& typeInfoProvider,
        std::list<clang::Token>* tokens,
        clang::tok::TokenKind  endToken = clang::tok::eof
    ) : m_scope(scope), m_typeInfoProvider(typeInfoProvider), m_tokens(tokens), m_endToken(endToken)
    {}

    TypeInfoPtr   getResult();

    std::list<std::string>  getTemplateArgList();

private:

    TypeInfoPtr getStandardIntType(const parser::StandardIntMatcher& stdIntMatcher);

    std::string getTemplateArgs(const parser::ListMatcher<parser::TemplateArgMatcher>& argList);

    std::list<std::string>  getTemplateArgList(const parser::ListMatcher<parser::TemplateArgMatcher>& argList);

    std::string getTemplateName(const parser::TemplateMatcher& templateMatcher);

    std::string getNestedTemplateName(const parser::DoubleTemplateMatcher& templateMatcher);

    std::string getTypeName(const parser::QualifiedTypeMatcher& typeMatcher);

    //std::string getBaseTypeName(const parser::BaseTypeMatcher& baseTypeMatcher);

    std::string getCustomTypeName(const parser::CustomTypeMatcher& customMatcher);


    TypeInfoPtr getCustomType(const parser::CustomTypeMatcher& customMatcher);

    ScopePtr   m_scope;
    
    TypeInfoProviderPtr  m_typeInfoProvider;

    std::list<clang::Token>*  m_tokens;
    clang::tok::TokenKind  m_endToken;

};

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

class ExprEval2 {

public:

    ExprEval2(
        const ScopePtr& scope,
        const TypeInfoProviderPtr& typeInfoProvider,
        const TokenRange& tokenRange
    ) :
        m_scope(scope),
        m_typeInfoProvider(typeInfoProvider),
        m_tokenRange(tokenRange)
    {}

    TypedValue  getResult();

private:

    static TypedValue getResultForMatcher(const parser::ConstExpressionMatcher& matcher);

    ScopePtr   m_scope;
    TypeInfoProviderPtr  m_typeInfoProvider;
    TokenRange  m_tokenRange;
};

///////////////////////////////////////////////////////////////////////////////


}