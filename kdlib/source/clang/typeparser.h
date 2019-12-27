#pragma once

#include "parser.h"
#include "exprparser.h"
#include "basetypematcher.h"

namespace kdlib {

bool isTrueFalse(const clang::Token& token);
std::string  getIdentifier(const clang::Token& token);
bool isOperationToken(const clang::Token& token);

namespace parser {


class StandardIntMatcher : public Matcher
{
public:

    MatchResult  match(const TokenRange& matchRange)
    {
        auto& beg = matchRange.first;
        auto& end = matchRange.second;

        if (beg != end && isStandardIntType(*beg))
        {
            auto r = MatchResult(std::make_pair(beg, std::next(beg)));
            return matchResult = r;
        }

        return matchResult = MatchResult();
    }
};

class PointerMatcher : public Matcher
{
public:

    MatchResult  match(const TokenRange& matchRange)
    {
        auto matcher = all_of(
            token_is(clang::tok::star),
            opt(constMatcher),
            opt(rep(token_is(clang::tok::kw_const)))
        );

        return matchResult = matcher.match(matchRange);
    }

    bool isConst() const
    {
        return constMatcher.getMatchResult().isMatched();
    }

private:

    Is<clang::tok::kw_const>  constMatcher;
};

class PointerPointerMatcher : public Matcher
{
public:

    MatchResult  match(const TokenRange& matchRange)
    {
        auto matcher = rep(pointersMatcher, pointersMatcher);
        return matchResult = matcher.match(matchRange);
    }

    const auto&  getPointerMatchers() const
    {
        return pointersMatcher;
    }

private:

    ListMatcher<PointerMatcher>  pointersMatcher;
};

class ArrayIndexMatcher : public Matcher
{
public:

    MatchResult match(const TokenRange& matchRange)
    {
        auto matcher = all_of(Is<clang::tok::l_square>(), opt(numericMatcher), Is<clang::tok::r_square>());
        return matchResult = matcher.match(matchRange);
    }

    bool isIndexComplete() const
    {
        return numericMatcher.getMatchResult().isMatched();
    }

    const auto& getIndexMatcher() const
    {
        assert(isIndexComplete());
        return numericMatcher;
    }

private:

    NumericMatcher  numericMatcher;

};

class ArrayMatcher : public Matcher
{
public:

    MatchResult match(const TokenRange& matchRange)
    {
        return matchResult = rep(indexMatcher, indexMatcher).match(matchRange);
    }

    const auto& getArrayIndices() const {
        return indexMatcher;
    }

private:

    ListMatcher<ArrayIndexMatcher> indexMatcher;
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

    MatchResult  match(const TokenRange& matchRange)
    {
        auto matcher = all_of(
            any_of(lvalueRefMatcher, rvalueRefMacther),
            opt(rep(token_is(clang::tok::kw_const) ) )
        );

        return matchResult = matcher.match(matchRange);
    }

    bool isLvalue() const 
    {
        return lvalueRefMatcher.getMatchResult().isMatched();
    }

    bool isRvalue() const
    {
        return rvalueRefMacther.getMatchResult().isMatched();
    }

private:

    Is<clang::tok::amp> lvalueRefMatcher;
    Is<clang::tok::ampamp> rvalueRefMacther;
};

class ComplexMatcher : public Matcher
{

public:

    MatchResult  match(const TokenRange& matchRange)
    {
        auto matcher =  all_of(
            opt(pointersMatcher), 
            opt(
                any_of(
                    refMatcher, 
                    all_of(
                        opt(nestedComplexMatcher), 
                        opt(arrayMatcher)
                    )
                )
            )
        );   
               
        matchResult = matcher.match(matchRange);

        if (matchResult.isMatched() && (matchResult.begin() != matchResult.end() ) )
            return matchResult;

        return matchResult = MatchResult();
    }

    bool isPointer() const
    {
        return pointersMatcher.getMatchResult().isMatched();
    }

    const auto& getPointerMatcher() const
    {
        assert(isPointer());
        return pointersMatcher;
    }

    bool isNestedMatcher() const
    {
        return nestedComplexMatcher.getMatchResult().isMatched();
    }

    const auto& getNestedMatcher() const
    {
        assert(isNestedMatcher());
        return nestedComplexMatcher;
    }

    bool isArray() const
    {
        return arrayMatcher.getMatchResult().isMatched();
    }

    const auto& getArrayMatcher() const
    {
        assert(isArray());
        return arrayMatcher;
    }

    bool isReference() const
    {
        return refMatcher.getMatchResult().isMatched();
    }

    const auto& getRefMatcher() const
    {
        assert(isReference());
        return refMatcher;
    }

private:

    PointerPointerMatcher   pointersMatcher;
    ArrayMatcher  arrayMatcher;
    RefMatcher   refMatcher;
    NestedComplexMatcher  nestedComplexMatcher;
};


class FieldMatcher : public Matcher
{
public:

    MatchResult  match(const TokenRange& matchRange)
    {
        auto matcher = all_of(Is<clang::tok::colon>(), Is<clang::tok::colon>(), name);
        return matchResult = matcher.match(matchRange);
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

    MatchResult  match(const TokenRange& matchRange)
    {
        if (matchRange.first == matchRange.second)
            return matchResult = MatchResult();

        if (isStandardIntType(*matchRange.first) 
            || isBaseTypeKeyWord(*matchRange.first) 
            || isTrueFalse(*matchRange.first) )
                return matchResult = MatchResult();

        auto matcher = all_of(nameMatcher, opt(rep(namespacesMatchers, namespacesMatchers)));

        return matchResult = matcher.match(matchRange);
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

class QualifiedTypeMatcher;

class TemplateArgMatcher : public Matcher
{
public:

    MatchResult  match(const TokenRange& matchRange);

    bool isType() const;

    const QualifiedTypeMatcher& getTypeMatcher() const;

    bool isExpression() const
    {
        return exprMatcher.getMatchResult().isMatched();
    }

    const auto& getExpressionMatcher() const
    {
        return exprMatcher;
    }

private:

    std::unique_ptr<QualifiedTypeMatcher>  typeMatcher;

    ConstExpressionMatcher  exprMatcher;
};

class TemplateArgsMatcher : public Matcher
{
public:

    MatchResult  match(const TokenRange& matchRange)
    {
        TemplateArgMatcher  argMatcher;
        auto matcher = all_of(argMatcher, opt(rep(all_of(Is<clang::tok::comma>(), argsMatchers), argsMatchers)));
        matchResult = matcher.match(matchRange);
        argsMatchers.push_front(std::move(argMatcher));
        return matchResult;
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
    MatchResult  match(const TokenRange& matchRange)
    {
        auto matcher = all_of(
            token_is(clang::tok::less),
            templateArgsMatcher,
            token_is(clang::tok::greater)
        );

        return matchResult = matcher.match(matchRange);
    }

    const auto& getTemplateArgs() const
    {
        assert(templateArgsMatcher.getMatchResult().isMatched());
        return templateArgsMatcher.getArgs();
    }

private:

    TemplateArgsMatcher  templateArgsMatcher;
};

class DoubleTemplateMatcher : public Matcher
{
public:
    MatchResult  match(const TokenRange& matchRange)
    {
        auto matcher =
            all_of(
                token_is(clang::tok::less),
                opt(
                    rep(
                        all_of(
                            argsMatchers1,
                            token_is(clang::tok::comma)
                        ),
                        argsMatchers1
                    )
                ),
                nestedTemplateName,
                token_is(clang::tok::less),
                opt(
                    rep(
                        all_of(
                            argsMatchers2,
                            token_is(clang::tok::comma)
                        ),
                        argsMatchers2
                    )
                ),
                cap(argsMatchers2, argsMatchers2),
                Is<clang::tok::greatergreater>()
            );

        return matchResult = matcher.match(matchRange);
    }

    const auto& getTemplateArgs1() const
    {
        return argsMatchers1;
    }

    const auto& getTemplateArgs2() const
    {
        return argsMatchers2;
    }

    std::string getNestedTemplateName() const
    {
        assert(nestedTemplateName.getMatchResult().isMatched());
        return nestedTemplateName.getName();
    }


private:

    ListMatcher<TemplateArgMatcher>  argsMatchers1;
    ListMatcher<TemplateArgMatcher>  argsMatchers2;
    CustomNameMatcher  nestedTemplateName;
};


class CustomTypeMatcher : public Matcher
{
public:

    MatchResult  match(const TokenRange& matchRange)
    {
        auto matcher =
            all_of(
                typeNameMatcher,
                opt(
                    any_of(
                        templateMatcher,
                        templateMatcher2
                    )
                ),
                opt(rep(fieldsMatchers, fieldsMatchers))
            );
            
        return matchResult = matcher.match(matchRange);
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

    bool isNestedTemplate() const
    {
        return templateMatcher2.getMatchResult().isMatched();
    }
    const auto& getNestedTemplateMatcher() const
    {
        assert(isNestedTemplate());
        return templateMatcher2;
    }


private:

    ListMatcher<FieldMatcher>  fieldsMatchers;
    CustomNameMatcher  typeNameMatcher;
    TemplateMatcher  templateMatcher;
    DoubleTemplateMatcher   templateMatcher2;
};


class TypeMatcher : public Matcher
{
public:
    MatchResult  match(const TokenRange& matchRange)
    {    
        auto matcher = any_of(baseTypeMatcher, stdIntMatcher, customMatcher);
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

    const StandardIntMatcher& getStandardIntMatcher() const
    {
        assert(isStandardIntType());
        return stdIntMatcher;
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
};

class QualifiedTypeMatcher : public Matcher
{
public:
    MatchResult  match(const TokenRange& matchRange)
    {
        auto matcher = all_of(
            typeMatcher,
            opt(complexMatcher)
        );
        matchResult = matcher.match(matchRange);
        return matchResult;
    }

    bool isConst() const
    {
       return typeMatcher.isBasedType() && typeMatcher.getBaseTypeMatcher().isConst();
    }

    bool isBasedType() const
    {
        return typeMatcher.isBasedType();
    }

    const BaseTypeMatcher& getBaseTypeMatcher() const
    {
        return typeMatcher.getBaseTypeMatcher();
    }

    bool isStandardIntType() const
    {
        return typeMatcher.isStandardIntType();
    }

    const StandardIntMatcher& getStandardIntMatcher() const
    {
        return typeMatcher.getStandardIntMatcher();
    }

    bool isCustomType() const
    {
        return typeMatcher.isCustomType();
    }

    const CustomTypeMatcher& getCustomMatcher() const
    {
        return typeMatcher.getCustomMatcher();
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

private:

    TypeMatcher  typeMatcher;
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
    typeMatcher = std::make_unique<QualifiedTypeMatcher>();
    auto matcher = any_of(
        exprMatcher,
        *typeMatcher.get()
        );
    return matchResult = matcher.match(matchRange);
}

inline
bool TemplateArgMatcher::isType() const
{
    return typeMatcher->getMatchResult().isMatched();
}

inline
const QualifiedTypeMatcher& TemplateArgMatcher::getTypeMatcher() const
{
    assert(isType());
    return *typeMatcher.get();
}


} }

