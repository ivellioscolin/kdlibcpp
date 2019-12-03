#include <stdafx.h>

#include "basetypematcher.h"
#include "kdlib/exceptions.h"
#include "evalexpr.h"

namespace kdlib {
namespace parser {

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
    virtual BaseTypeBuilder* addVoid() = 0;
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

    BaseTypeBuilder* addUnsigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addSigned() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addLong() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addFloat() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addDouble() override {
        throw ExprException(L"bad syntax");
    }

    BaseTypeBuilder* addVoid() override {
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

class VoidTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        return loadType(L"Void");
    }

    std::string getTypeName() override
    {
        return "void";
    }
};

class EmptyBaseTypeBuilder : public DeclBaseTypeBuilder
{
    TypeInfoPtr  getResult() override {
        throw ExprException(L"bad syntax");
    }

    std::string getTypeName() override {
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

    BaseTypeBuilder* addVoid() override {
        return new VoidTypeBuilder();
    }
};


MatchResult  BaseTypeMatcher::match(const TokenRange& matchRange)
{
    const auto& beg = matchRange.first;
    const auto& end = matchRange.second;

    std::unique_ptr<BaseTypeBuilder>  baseTypeBuilder(new EmptyBaseTypeBuilder);

    for (auto cur = beg; cur != end; cur = std::next(cur) )
    {
        if (cur->is(clang::tok::kw_const))
        {
            m_const = true;
            continue;
        }

        if (cur->is(clang::tok::kw_void))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addVoid());
            continue;
        }

        if (cur->is(clang::tok::kw_int))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addInt());
            continue;
        }
        
        if (cur->is(clang::tok::kw_char))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addChar());
            continue;
        }
        
        if (cur->is(clang::tok::kw_short))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addShort());
            continue;
        }
        
        if (cur->is(clang::tok::kw_unsigned))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addUnsigned());
            continue;
        }
        
        if (cur->is(clang::tok::kw_signed))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addSigned());
            continue;
        }
        
        if (cur->is(clang::tok::kw_long))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addLong());
            continue;
        }
        
        if (isInt64KeyWord(*cur))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addLong());
            baseTypeBuilder.reset(baseTypeBuilder->addLong());
            continue;
        }
        
        if (cur->is(clang::tok::kw_float))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addFloat());
            continue;
        }
        
        if (cur->is(clang::tok::kw_double))
        {
            baseTypeBuilder.reset(baseTypeBuilder->addDouble());
            continue;
        }

        if (cur != beg)
        {
            m_typeInfo = baseTypeBuilder->getResult();
            m_typeName = baseTypeBuilder->getTypeName();
            return matchResult = MatchResult(std::make_pair(beg, cur));
        }

        break;
    }

    return matchResult = MatchResult();
}


std::string  BaseTypeMatcher::getTypeName() const
{
    std::string  name;
    if (m_const)
        name += "const ";
    name += m_typeName;
    return name;
}

TypeInfoPtr BaseTypeMatcher::getBaseType() const
{
    return m_typeInfo;
}


//TypeInfoPtr TypeEval::getBaseType()
//{
//    if (m_tokens->front().is(clang::tok::kw_void))
//    {
//        m_tokens->pop_front();
//        return loadType(L"Void");
//    }
//
//    std::unique_ptr<BaseTypeBuilder>  baseTypeBuilder(new EmptyBaseTypeBuilder);
//
//    while (true)
//    {
//        auto  token = m_tokens->front();
//
//        if (token.is(m_endToken))
//            break;
//   
//        if (token.is(clang::tok::kw_int))
//        {
//            baseTypeBuilder.reset(baseTypeBuilder->addInt());
//            m_tokens->pop_front();
//            continue;
//        }
//
//        if (token.is(clang::tok::kw_char))
//        {
//            baseTypeBuilder.reset(baseTypeBuilder->addChar());
//            m_tokens->pop_front();
//            continue;
//        }
//
//        if (token.is(clang::tok::kw_short))
//        {
//            baseTypeBuilder.reset(baseTypeBuilder->addShort());
//            m_tokens->pop_front();
//            continue;
//        }
//
//        if (token.is(clang::tok::kw_unsigned))
//        {
//            baseTypeBuilder.reset(baseTypeBuilder->addUnsigned());
//            m_tokens->pop_front();
//            continue;
//        }
//
//        if (token.is(clang::tok::kw_signed))
//        {
//            baseTypeBuilder.reset(baseTypeBuilder->addSigned());
//            m_tokens->pop_front();
//            continue;
//        }
//
//        if (token.is(clang::tok::kw_long))
//        {
//            baseTypeBuilder.reset(baseTypeBuilder->addLong());
//            m_tokens->pop_front();
//            continue;
//        }
//
//        if (isInt64KeyWord(token))
//        {
//            baseTypeBuilder.reset(baseTypeBuilder->addLong());
//            baseTypeBuilder.reset(baseTypeBuilder->addLong());
//            m_tokens->pop_front();
//            continue;
//        }
//
//        if (token.is(clang::tok::kw_float))
//        {
//            baseTypeBuilder.reset(baseTypeBuilder->addFloat());
//            m_tokens->pop_front();
//            continue;
//        }
//
//        if (token.is(clang::tok::kw_double))
//        {
//            baseTypeBuilder.reset(baseTypeBuilder->addDouble());
//            m_tokens->pop_front();
//            continue;
//        }
//
//        if (token.is(clang::tok::kw_const))
//        {
//            m_tokens->pop_front();
//            continue;
//        }
//
//        if (token.isOneOf(clang::tok::star, clang::tok::l_paren, clang::tok::l_square, 
//            clang::tok::amp, clang::tok::ampamp))
//            break;
//
//        throw ExprException(L"error syntax");
//    }
//
//    return baseTypeBuilder->getResult();
//}

}
}