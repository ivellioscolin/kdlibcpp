#pragma once

#include "clang/Lex/Token.h"

#include "kdlib/typedvar.h"
#include "kdlib/exceptions.h"

#include "strconvert.h"
#include "typeparser.h"

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
    TypedValue getResult(const TypedValue& val) override;
};

class UnPlusOperation : public UnaryOperation
{
    TypedValue getResult(const TypedValue& val) override
    {
        return val;
    }
};

class DerefOperation : public UnaryOperation
{
    TypedValue getResult(const TypedValue& val) override
    {
        TypedValue  v = val;
        return v.deref();
    }
};


class RefOperation : public UnaryOperation
{
    TypedValue getResult(const TypedValue& val) override
    {
        return makePointer(val.getType(), val.getAddress());
    }
};

class BitwiseNotOperation : public UnaryOperation
{
    TypedValue getResult(const TypedValue& val) override;
};

class BooleanNotOperation : public UnaryOperation
{
    TypedValue getResult(const TypedValue& val) override;
};

class PreIncrementOperation : public UnaryOperation
{
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
        const TypeInfoProviderPtr typeInfoProvider,
        std::list<clang::Token>* tokens,
        clang::tok::TokenKind  endToken = clang::tok::eof
    ) : m_scope(scope), m_typeInfoProvider(typeInfoProvider), m_tokens(tokens), m_endToken(endToken)
    {}

    TypeInfoPtr   getResult();

private:


    TypeInfoPtr getBaseType();

    TypeInfoPtr getStandardIntType();

    //TypeInfoPtr getCustomType();

    //TypeInfoPtr getComplexType(TypeInfoPtr& origType);

    //void getTypeModifiers(std::list<TypeModifier*>& typeModifiers, clang::tok::TokenKind endToken);

    std::string getTemplateArgs(const parser::ListMatcher<parser::TemplateArgMatcher>& argList);

    std::string getTemplateName(const parser::TemplateMatcher& templateMatcher);

    std::string getNestedTemplateName(const parser::DoubleTemplateMatcher& templateMatcher);

    std::string getTypeName(const parser::TypeMatcher& typeMatcher);

    std::string getBaseTypeName(const parser::BaseTypeMatcher& baseTypeMatcher);

    std::string getCustomTypeName(const parser::CustomTypeMatcher& customMatcher);


    TypeInfoPtr getCustomType(const parser::CustomTypeMatcher& customMatcher);

    ScopePtr   m_scope;
    
    TypeInfoProviderPtr  m_typeInfoProvider;

    std::list<clang::Token>*  m_tokens;
    clang::tok::TokenKind  m_endToken;

};

///////////////////////////////////////////////////////////////////////////////

class BaseTypeBuilder {
public:
    virtual TypeInfoPtr  getResult() = 0;
    virtual std::string getTypeName() = 0;

    virtual BaseTypeBuilder* addChar() = 0;
    virtual BaseTypeBuilder* addShort() = 0;
    virtual BaseTypeBuilder* addInt() = 0;
    virtual BaseTypeBuilder* addUnsigned() = 0;
    virtual BaseTypeBuilder* addSigned() = 0;
    virtual BaseTypeBuilder* addLong() = 0;
    virtual BaseTypeBuilder* addFloat() = 0;
    virtual BaseTypeBuilder* addDouble() = 0;   
};




class DeclBaseTypeBuilder : public BaseTypeBuilder
{
    BaseTypeBuilder* addChar() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addShort() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addInt() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addFloat() {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addDouble() {
        throw ExprException(L"bad syntax");
    }
};

class UnsignedCharBaseTypeBuilder : public DeclBaseTypeBuilder
{

    TypeInfoPtr  getResult() override {
        return loadType(L"UInt1B");
    }

    std::string getTypeName() override
    {
        return "unsigned char";
    }

    BaseTypeBuilder* addUnsigned() override {
        return this;
    }

    BaseTypeBuilder* addSigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }
};

class SignedCharBaseTypeBuilder : public DeclBaseTypeBuilder
{

    TypeInfoPtr  getResult() override {
        return loadType(L"Int1B");
    }

    std::string getTypeName() override
    {
        return "char";
    }

    BaseTypeBuilder* addUnsigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addSigned() override {
        return this;
    }

    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }
};


class CharBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Int1B");
    }

    std::string getTypeName() override
    {
        return "char";
    }

    BaseTypeBuilder* addUnsigned() override {
        return new UnsignedCharBaseTypeBuilder;
    }

    BaseTypeBuilder* addSigned() override {
        throw new SignedCharBaseTypeBuilder;
    }

    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }
};


class UnsignedShortBaseTypeBuilder : public DeclBaseTypeBuilder
{

    TypeInfoPtr  getResult() override {
        return loadType(L"UInt2B");
    }

    std::string getTypeName() override
    {
        return "unsigned short";
    }

    BaseTypeBuilder* addUnsigned() override {
        return this;
    }

    BaseTypeBuilder* addSigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }
};


class SignedShortBaseTypeBuilder : public DeclBaseTypeBuilder
{

    TypeInfoPtr  getResult() override {
        return loadType(L"Int2B");
    }

    std::string getTypeName() override
    {
        return "short";
    }

    BaseTypeBuilder* addUnsigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addSigned() override {
        return this;
    }

    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }
};


class ShortBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Int2B");
    }

    std::string getTypeName() override
    {
        return "short";
    }

    BaseTypeBuilder* addUnsigned() override {
        return new UnsignedShortBaseTypeBuilder;
    }

    BaseTypeBuilder* addSigned() override {
        return new SignedShortBaseTypeBuilder;
    }

    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }
};

class UnsignedLongLongIntBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"UInt8B");
    }

    std::string getTypeName() override
    {
        return "unsigned long long";
    }

    BaseTypeBuilder* addUnsigned() override {
        return this;
    }

    BaseTypeBuilder* addSigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }
};

class SignedLongLongIntBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Int8B");
    }

    std::string getTypeName() override
    {
        return "long long";
    }

    BaseTypeBuilder* addUnsigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addSigned() override {
        return this;
    }

    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }
};

class UnsignedLongIntBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"UInt4B");
    }

    std::string getTypeName() override
    {
        return "unsigned long";
    }

    virtual BaseTypeBuilder* addUnsigned() override {
        return this;
    }

    BaseTypeBuilder* addSigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addLong() override {
        return new UnsignedLongLongIntBaseTypeBuilder;
    }
};


class SignedLongIntBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Int4B");
    }

    std::string getTypeName() override
    {
        return "long";
    }

    virtual BaseTypeBuilder* addUnsigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addSigned() override {
        return this;
    }

    BaseTypeBuilder* addLong() override {
        return new UnsignedLongLongIntBaseTypeBuilder;
    }
};

class UnsignedIntBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"UInt4B");
    }

    std::string getTypeName() override
    {
        return "unsigned int";
    }

    virtual BaseTypeBuilder* addUnsigned() override {
        return this;
    }

    BaseTypeBuilder* addSigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addLong() override {
        return new UnsignedLongIntBaseTypeBuilder;
    }
};

class SignedIntBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Int4B");
    }

    std::string getTypeName() override
    {
        return "int";
    }

    BaseTypeBuilder* addUnsigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addSigned() override {
        return this;
    }

    BaseTypeBuilder* addLong() override {
        return new UnsignedLongIntBaseTypeBuilder;
    }
};

class LongLongIntBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Int8B");
    }

    std::string getTypeName() override
    {
        return "long long";
    }

    BaseTypeBuilder* addUnsigned() override {
        return new UnsignedLongLongIntBaseTypeBuilder;
    }

    BaseTypeBuilder* addSigned() override {
        return new SignedLongLongIntBaseTypeBuilder;
    }

    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }
};

class LongIntBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Int4B");
    }

    std::string getTypeName() override
    {
        return "long";
    }

    BaseTypeBuilder* addUnsigned() override {
        return new UnsignedLongIntBaseTypeBuilder;
    }

    BaseTypeBuilder* addSigned() override {
        return new SignedLongIntBaseTypeBuilder;
    }

    BaseTypeBuilder* addLong() override {
        return new LongLongIntBaseTypeBuilder;
    }
};

class IntBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Int4B");
    }

    std::string getTypeName() override
    {
        return "int";
    }

    BaseTypeBuilder* addUnsigned() override {
        return new UnsignedIntBaseTypeBuilder;
    }

    BaseTypeBuilder* addSigned() override {
        return new SignedIntBaseTypeBuilder;
    }

    BaseTypeBuilder* addLong() override {
        return new LongIntBaseTypeBuilder;
    }
};

class UnsignedLongLongBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"UInt8B");
    }

    std::string getTypeName() override
    {
        return "unsigned long long";
    }

    virtual BaseTypeBuilder* addUnsigned() override {
        return this;
    }

    BaseTypeBuilder* addSigned() override {
        throw ExprException(L"bad syntax");
    }
    
    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addInt() override {
        return new UnsignedLongLongIntBaseTypeBuilder;
    }


};


class SignedLongLongBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Int8B");
    }

    std::string getTypeName() override
    {
        return "long long";
    }

    BaseTypeBuilder* addUnsigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addSigned() override {
        return this;
    }

    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addInt() override {
        return new UnsignedLongLongIntBaseTypeBuilder;
    }
};

class UnsignedLongBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"UInt4B");
    }

    std::string getTypeName() override
    {
        return "unsigned long";
    }

    BaseTypeBuilder* addUnsigned() override {
        return this;
    }

    BaseTypeBuilder* addSigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addLong() override {
        return new UnsignedLongLongBaseTypeBuilder;
    }

    BaseTypeBuilder* addInt() override {
        return new UnsignedLongIntBaseTypeBuilder;
    }
};

class SignedLongBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Int4B");
    }

    std::string getTypeName() override
    {
        return "long";
    }

    BaseTypeBuilder* addUnsigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addSigned() override {
        return this;
    }

    BaseTypeBuilder* addLong() override {
        return new SignedLongLongBaseTypeBuilder;
    }

    BaseTypeBuilder* addInt() override {
        return new UnsignedLongIntBaseTypeBuilder;
    }
};


class LongLongBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Int8B");
    }

    std::string getTypeName() override
    {
        return "long long";
    }

    BaseTypeBuilder* addUnsigned() override {
        return new UnsignedLongLongBaseTypeBuilder;
    }

    BaseTypeBuilder* addSigned() override {
        return new SignedLongLongBaseTypeBuilder;
    }

    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addInt() override {
        return new LongLongIntBaseTypeBuilder;
    }
};

class UnsignedBaseTypeBuilder : public DeclBaseTypeBuilder {

    virtual TypeInfoPtr  getResult() override {
        return loadType(L"UInt4B");
    }

    std::string getTypeName() override
    {
        return "unsigned int";
    }

    virtual BaseTypeBuilder* addChar() override {
        return new UnsignedCharBaseTypeBuilder;
    }

    virtual BaseTypeBuilder* addShort() override {
        return new UnsignedShortBaseTypeBuilder;
    }

    virtual BaseTypeBuilder* addInt() override {
        return new UnsignedIntBaseTypeBuilder;
    }

    BaseTypeBuilder* addLong() override {
        return new UnsignedLongBaseTypeBuilder;
    }

    BaseTypeBuilder* addUnsigned()  override {
        return this;
    }

    BaseTypeBuilder* addSigned()  override {
        throw ExprException(L"bad syntax");
    }
};

class SignedBaseTypeBuilder : public DeclBaseTypeBuilder {

    virtual TypeInfoPtr  getResult() override {
        return loadType(L"Int4B");
    }

    std::string getTypeName() override
    {
        return "int";
    }

    virtual BaseTypeBuilder* addChar() override {
        return new SignedCharBaseTypeBuilder;
    }

    virtual BaseTypeBuilder* addShort() override {
        return new SignedShortBaseTypeBuilder;
    }

    virtual BaseTypeBuilder* addInt() override {
        return new SignedIntBaseTypeBuilder;
    }

    BaseTypeBuilder* addLong() override {
        return new SignedLongBaseTypeBuilder;
    }

    BaseTypeBuilder* addUnsigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addSigned() override {
        return this;
    }
};

class LongDoubleBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Double");
    }

    std::string getTypeName() override
    {
        return "double";
    }

    BaseTypeBuilder* addUnsigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addSigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }
};


class LongFloatBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Double");
    }

    std::string getTypeName() override
    {
        return "double";
    }

    BaseTypeBuilder* addUnsigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addSigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }
};


class LongBaseTypeBuilder : public DeclBaseTypeBuilder 
{

    virtual TypeInfoPtr  getResult() {
        return loadType(L"Int4B");
    }

    std::string getTypeName() override
    {
        return "long";
    }

    virtual BaseTypeBuilder* addInt() override {
        return new LongIntBaseTypeBuilder;
    }

    BaseTypeBuilder* addLong() override {
        return new LongLongBaseTypeBuilder;
    }

    BaseTypeBuilder* addUnsigned()override {
        return new UnsignedLongBaseTypeBuilder;
    }

    BaseTypeBuilder* addSigned() override {
        return new SignedLongBaseTypeBuilder;
    }

    BaseTypeBuilder* addFloat() override {
        return new LongFloatBaseTypeBuilder();
    }

    BaseTypeBuilder* addDouble() override {
        return new LongDoubleBaseTypeBuilder();
    }
};


class FloatBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Float");
    }

    std::string getTypeName() override
    {
        return "float";
    }

    BaseTypeBuilder* addUnsigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addSigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addLong() override {
        return new LongFloatBaseTypeBuilder();
    }
};


class DoubleBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Double");
    }

    std::string getTypeName() override
    {
        return "double";
    }

    BaseTypeBuilder* addUnsigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addSigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addLong() override {
        return new LongDoubleBaseTypeBuilder();
    }
};

class EmptyBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        throw ExprException(L"bad syntax");
    }

    std::string getTypeName() override  {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addInt() override {
        return new IntBaseTypeBuilder;
    }

    BaseTypeBuilder* addChar() override {
        return new CharBaseTypeBuilder;
    }

    BaseTypeBuilder* addShort() override {
        return new ShortBaseTypeBuilder;
    }

    BaseTypeBuilder* addUnsigned() override {
        return new UnsignedBaseTypeBuilder;
    }

    BaseTypeBuilder* addSigned() override {
        return new SignedBaseTypeBuilder;
    }

    BaseTypeBuilder* addLong() override {
        return new LongBaseTypeBuilder;
    }

    BaseTypeBuilder* addFloat() override {
        return new FloatBaseTypeBuilder();
    }

    BaseTypeBuilder* addDouble() override {
        return new DoubleBaseTypeBuilder();
    }
};

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

    ScopePtr   m_scope;
    TypeInfoProviderPtr  m_typeInfoProvider;
    TokenRange  m_tokenRange;
};

///////////////////////////////////////////////////////////////////////////////


}