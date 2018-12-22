#pragma once

#include "parser.h"

namespace kdlib {


bool isBaseTypeKeyWord(const clang::Token& token);
bool isStandardIntType(const clang::Token& token);
std::string  getIdentifier(const clang::Token& token);
bool isOperationToken(const clang::Token& token);

namespace parser {

class BaseTypeMatcher : public Matcher
{
public:
    auto  match(const TokenRange& matchRange)
    {
        auto& beg = matchRange.first;
        auto cur = beg;
        for (; cur != matchRange.second; cur++)
        {
            if (!isBaseTypeKeyWord(*cur))
                break;
        }

        return matchResult = MatchResult(std::make_pair(beg, cur));
    }
};

class StandardIntMatcher : public Matcher
{
public:

    auto  match(const TokenRange& matchRange)
    {
        auto& beg = matchRange.first;
        auto& end = matchRange.second;

        if (beg != end && isStandardIntType(*beg))
        {
            matchResult = MatchResult(std::make_pair(beg, std::next(beg, 1)));
        }
       
        return matchResult;
    }
};

class PointerMatcher : public Matcher
{
public:


    auto  match(const TokenRange& matchRange)
    {
        return matchResult = rep(pointersMatcher).match(matchRange);
    }

    const auto&  getPointerMatchers() const
    {
        return pointersMatcher;
    }

private:

    ListMatcher<Is<clang::tok::star>>  pointersMatcher;

};

class ArrayMatcher : public Matcher
{
public:

    auto match(const TokenRange& matchRange)
    {
        return matchResult = rep(all_of(Is<clang::tok::l_square>(), numericMatcher, Is<clang::tok::r_square>())).match(matchRange);
    }

    const auto& getArrayIndices() const {
        return numericMatcher;
    }

private:

    ListMatcher<NumericMatcher> numericMatcher;
};

class ComplexMatcher;

class NestedComplexMatcher : public Matcher
{
public: 

    MatchResult  match(const TokenRange& matchRange);
    
    const ComplexMatcher& getInnerMatcher() const;

private:

    std::unique_ptr<ComplexMatcher>  innerMatcher;

};

class RefMatcher : public Matcher
{
public:
    auto  match(const TokenRange& matchRange)
    {
        return matchResult = any_of(Is<clang::tok::amp>(), Is<clang::tok::ampamp>()).match(matchRange);
    }
};

class ComplexMatcher : public Matcher
{

public:

    auto  match(const TokenRange& matchRange)
    {
        return matchResult = all_of(opt(pointerMatcher), opt(any_of(refMatcher, all_of(opt(nestedComplexMatcher), opt(arrayMatcher))))).match(matchRange);
    }

    bool isPointer() const
    {
        return pointerMatcher.getMatchResult().isMatched();
    }

    const PointerMatcher& getPointerMatcher() const
    {
        assert(isPointer());
        return pointerMatcher;
    }

    bool isNestedMatcher() const
    {
        return nestedComplexMatcher.getMatchResult().isMatched();
    }

    const NestedComplexMatcher& getNestedMatcher() const
    {
        assert(isNestedMatcher());
        return nestedComplexMatcher;
    }

    bool isArray() const
    {
        return arrayMatcher.getMatchResult().isMatched();
    }

    const ArrayMatcher& getArrayMatcher() const
    {
        assert(isArray());
        return arrayMatcher;
    }


private:

    PointerMatcher   pointerMatcher;
    ArrayMatcher  arrayMatcher;
    RefMatcher   refMatcher;
    NestedComplexMatcher  nestedComplexMatcher;
};


class FieldMatcher : public Matcher
{
public:

    auto  match(const TokenRange& matchRange)
    {
        return matchResult = all_of(Is<clang::tok::colon>(), Is<clang::tok::colon>(), name).match(matchRange);
    }

    std::string  getName() const
    {
        return getIdentifier(*name.getMatchResult().begin());
    }

private:

    IdentifierMatcher  name;
};

using NamespaceMatcher = FieldMatcher;


class CustomNameMatcher : public Matcher
{
public:

    auto  match(const TokenRange& matchRange)
    {
        if (matchRange.first == matchRange.second)
            return MatchResult();

        if (isStandardIntType(*matchRange.first) || isBaseTypeKeyWord(*matchRange.first))
            return MatchResult();

        return matchResult = all_of(nameMatcher, opt(rep(namespacesMatchers))).match(matchRange);
    }

    std::string  getName() const
    {
        return getIdentifier(*nameMatcher.getMatchResult().begin());
    }

    const auto& getNamespaces() const
    {
        return namespacesMatchers;
    }

private:

    IdentifierMatcher  nameMatcher;

    ListMatcher<NamespaceMatcher>  namespacesMatchers;
};




class TemplateNumericArgMatcher : public Matcher
{
public:

    auto  match(const TokenRange& matchRange)
    {
        auto& beg = matchRange.first;
        auto cur = beg;
        for (; cur != matchRange.second; cur++)
        {
            if ((*cur).is(clang::tok::numeric_constant))
                continue;

            if (isOperationToken(*cur))
                continue;

            break;
        }

        return matchResult = MatchResult(std::make_pair(beg, cur));
    }
};

class TypeMatcher;

class TemplateArgMatcher : public Matcher
{
public:

    MatchResult  match(const TokenRange& matchRange);

    bool isType() const;

    const TypeMatcher& getTypeMatcher() const;

    bool isNumeric() const
    {
        return numericMatcher.getMatchResult().isMatched();
    }

    const auto& getNumericMatcher() const
    {
        return numericMatcher;
    }

private:

    std::unique_ptr<TypeMatcher>  typeMatcher;

    TemplateNumericArgMatcher  numericMatcher;

};

class TemplateArgsMatcher : public Matcher
{
public:
    auto  match(const TokenRange& matchRange)
    {
        return matchResult = all_of(argsMatchers, opt(rep(all_of(Is<clang::tok::comma>(), argsMatchers)))).match(matchRange);
    }

    const auto& getArgs() const
    {
        return argsMatchers;
    }

private:

    ListMatcher<TemplateArgMatcher>  argsMatchers;
};

class TemplateMatcher : public Matcher
{
public:
    auto  match(const TokenRange& matchRange)
    {
        return matchResult = all_of(Is<clang::tok::less>(), templateArgsMatcher, Is<clang::tok::greater>()).match(matchRange);
    }

    const auto& getTemplateArgs() const
    {
        assert(templateArgsMatcher.getMatchResult().isMatched());
        return templateArgsMatcher.getArgs();
    }

private:

    TemplateArgsMatcher  templateArgsMatcher;
};



class CustomTypeMatcher : public Matcher
{
public:

    auto  match(const TokenRange& matchRange)
    {
        return matchResult = all_of( typeNameMatcher, opt(templateMatcher), opt(rep(fieldsMatchers))).match(matchRange);
    }

    const auto& getFields() const
    {
        return fieldsMatchers;
    }

    const auto& getTypeNameMatcher() const
    {
        assert(typeNameMatcher.getMatchResult().isMatched());
        return typeNameMatcher;
    }

    bool isTemplate() const
    {
        return templateMatcher.getMatchResult().isMatched();
    }

    const auto& getTemplateMatcher() const
    {
        assert(isTemplate());
        return templateMatcher;
    }

private:

    ListMatcher<FieldMatcher>  fieldsMatchers;
    CustomNameMatcher  typeNameMatcher;
    TemplateMatcher  templateMatcher;
};


class TypeMatcher : public Matcher
{
public:
    auto  match(const TokenRange& matchRange)
    {    
        auto matcher = all_of(any_of(baseTypeMatcher, stdIntMatcher, customMatcher), opt(complexMatcher));
        matchResult = matcher.match(matchRange);
        return matchResult;
    }

    bool isBasedType() const
    {
        return baseTypeMatcher.getMatchResult().isMatched();
    }

    const BaseTypeMatcher& getBaseTypeMatcher() const
    {
        assert(isBasedType());
        return baseTypeMatcher;
    }

    bool isStandardIntType() const
    {
        return stdIntMatcher.getMatchResult().isMatched();
    }

    bool isComplexType() const
    {
        return complexMatcher.getMatchResult().isMatched();
    }

    const ComplexMatcher& getComplexMather() const
    {
        assert(isComplexType());
        return complexMatcher;
    }

    bool isCustomType() const
    {
        return customMatcher.getMatchResult().isMatched();
    }

    const CustomTypeMatcher& getCustomMatcher() const
    {
        assert(isCustomType());
        return customMatcher;
    }


private:

    CustomTypeMatcher  customMatcher;
    BaseTypeMatcher  baseTypeMatcher;
    StandardIntMatcher  stdIntMatcher;
    ComplexMatcher complexMatcher;
};


inline
MatchResult NestedComplexMatcher::match(const TokenRange& matchRange)
{
    innerMatcher = std::make_unique<ComplexMatcher>();
    return matchResult = all_of(Is<clang::tok::l_paren>(), *innerMatcher.get(), Is<clang::tok::r_paren>()).match(matchRange);
}

inline
const ComplexMatcher& NestedComplexMatcher::getInnerMatcher() const
{
    return *innerMatcher.get();
} 


inline
MatchResult TemplateArgMatcher::match(const TokenRange& matchRange)
{
    typeMatcher = std::make_unique<TypeMatcher>();
    return matchResult = any_of(*typeMatcher.get(), numericMatcher).match(matchRange);
}

inline
bool TemplateArgMatcher::isType() const
{
    return typeMatcher->getMatchResult().isMatched();
}

inline
const TypeMatcher& TemplateArgMatcher::getTypeMatcher() const 
{
    assert(isType());
    return *typeMatcher.get();
}


} }

