#pragma once

#include "parser.h"

namespace kdlib {

namespace parser {

class ConstExpressionMatcher;


class UnaryRvalueOperation : public Matcher
{
public:

    auto  match(const TokenRange& matchRange)
    {
        return matchResult = any_of(
            m_minusMatcher,
            m_plusMatcher,
            m_exclaimMatcher,
            m_tildeMatcher
        ).match(matchRange);
    }

    bool isMinus() const
    {
        return m_minusMatcher.getMatchResult().isMatched();
    }

    bool isPlus() const
    {
        return m_plusMatcher.getMatchResult().isMatched();
    }

    bool isExclaim() const
    {
        return m_exclaimMatcher.getMatchResult().isMatched();
    }

    bool isTilde() const
    {
        return m_tildeMatcher.getMatchResult().isMatched();
    }

private:

    Is<clang::tok::minus>  m_minusMatcher;
    Is<clang::tok::plus>  m_plusMatcher;
    Is<clang::tok::exclaim>  m_exclaimMatcher;
    Is<clang::tok::tilde>  m_tildeMatcher;
};

class NestedExprMatcher : public Matcher
{
public:

    MatchResult  match(const TokenRange& matchRange);

    const auto& getExprMatcher() const
    {
        return *exprMatcher;
    }

private:
    std::unique_ptr<ConstExpressionMatcher>  exprMatcher;
};

class OperandMatcher : public Matcher
{
public:
    auto  match(const TokenRange& matchRange)
    {
        auto mather = any_of(
            all_of(
                opt(rep(unaryOperations, unaryOperations)),
                numericMatcher),
            nestedExprMatcher);
        return matchResult = mather.match(matchRange);
    }

    bool isNumericMatcher() const {
        return numericMatcher.getMatchResult().isMatched();
    }

    const auto& getNumericMatcher() const {
        return numericMatcher;
    }

    bool isNestedExpression() const
    {
        return nestedExprMatcher.getMatchResult().isMatched();
    }

    const auto& getNestedExpressionMatcher() const 
    {
        return nestedExprMatcher;
    }

    const auto& getUnaryOperations() const
    {
        return unaryOperations;
    }

private:
    
    NestedExprMatcher  nestedExprMatcher;
    NumericMatcher  numericMatcher;
    ListMatcher<UnaryRvalueOperation>  unaryOperations;
};

class BinaryOperationMatcher : public Matcher
{
public:
    auto  match(const TokenRange& matchRange)
    {
        auto matcher = any_of(
            m_minusMatcher,
            m_plusMatcher,
            m_starMatcher,
            m_slashMatcher,
            m_percentMatcher,
            m_greatergreaterMatcher,
            m_lesslessMatcher,
            m_ampMatcher,
            m_pipeMatcher,
            m_caretMatcher,
            m_equalequalMatcher,
            m_exclaimequalMatcher,
            m_lessMatcher,
            m_lessequalMatcher,
            m_greaterMatcher,
            m_greaterEqualMatcher,
            m_pipepipeMatcher,
            m_ampampMatcher
        );

        return matchResult = matcher.match(matchRange);
    }

    bool isAdd() const 
    {
        return m_plusMatcher.getMatchResult().isMatched();
    }

    bool isMinus() const 
    {
        return m_minusMatcher.getMatchResult().isMatched();
    }

    bool isStar() const
    {
        return m_starMatcher.getMatchResult().isMatched();
    }

    bool isSlash() const
    {
        return m_slashMatcher.getMatchResult().isMatched();
    }

    bool isPercent() const
    {
        return m_percentMatcher.getMatchResult().isMatched();
    }

    bool isGreaterGreater() const
    {
        return m_greatergreaterMatcher.getMatchResult().isMatched();
    }

    bool isLessLess() const
    {
        return m_lesslessMatcher.getMatchResult().isMatched();
    }

    bool isAmp() const
    {
        return m_ampMatcher.getMatchResult().isMatched();
    }

    bool isPipe() const
    {
        return m_pipeMatcher.getMatchResult().isMatched();
    }

    bool isCaret() const
    {
        return m_caretMatcher.getMatchResult().isMatched();
    }

    bool isEqualequal() const
    {
        return m_equalequalMatcher.getMatchResult().isMatched();
    }

    bool isExclaimequal() const
    {
        return m_exclaimequalMatcher.getMatchResult().isMatched();
    }

    bool isLess() const
    {
        return m_lessMatcher.getMatchResult().isMatched();
    }

    bool isLessEqual() const
    {
        return m_lessequalMatcher.getMatchResult().isMatched();
    }

    bool isGreater() const
    {
        return m_greaterMatcher.getMatchResult().isMatched();
    }

    bool isGreaterEqual() const
    {
        return m_greaterEqualMatcher.getMatchResult().isMatched();
    }

    bool isPipePipe() const
    {
        return m_pipepipeMatcher.getMatchResult().isMatched();
    }

    bool isAmpAmp() const
    {
        return m_ampampMatcher.getMatchResult().isMatched();
    }

private:
    Is<clang::tok::minus>  m_minusMatcher;
    Is<clang::tok::plus>  m_plusMatcher;
    Is<clang::tok::star>  m_starMatcher;
    Is<clang::tok::slash>  m_slashMatcher;
    Is<clang::tok::percent>  m_percentMatcher;
    Is<clang::tok::greatergreater>  m_greatergreaterMatcher;
    Is<clang::tok::lessless>  m_lesslessMatcher;
    Is<clang::tok::amp>  m_ampMatcher;
    Is<clang::tok::pipe>  m_pipeMatcher;
    Is<clang::tok::caret>  m_caretMatcher;
    Is<clang::tok::equalequal>  m_equalequalMatcher;
    Is<clang::tok::exclaimequal>  m_exclaimequalMatcher;
    Is<clang::tok::less>  m_lessMatcher;
    Is<clang::tok::lessequal>  m_lessequalMatcher;
    Is<clang::tok::greater>  m_greaterMatcher;
    Is<clang::tok::greaterequal>  m_greaterEqualMatcher;
    Is<clang::tok::pipepipe>  m_pipepipeMatcher;
    Is<clang::tok::ampamp>  m_ampampMatcher;
};

class ConstExpressionMatcher : public Matcher
{
public:
    auto  match(const TokenRange& matchRange)
    {
        auto matcher = all_of(
            cap(operandMatcher, operandMatcher),
            opt(rep(
                all_of(operationMatcher, operandMatcher), operationMatcher, operandMatcher
            ))
        );

        return matchResult = matcher.match(matchRange);
    }

    const auto& getOperationMatchers() const
    {
        return operationMatcher;
    }

    const auto& getOperandMatchers() const
    {
        return operandMatcher;
    }

private:

    ListMatcher<OperandMatcher>  operandMatcher;
    ListMatcher<BinaryOperationMatcher>  operationMatcher;
};

inline
MatchResult  NestedExprMatcher::match(const TokenRange& matchRange)
{
    exprMatcher = std::make_unique<ConstExpressionMatcher>();
    auto matcher = all_of(
        token_is(clang::tok::l_paren),
        *exprMatcher.get(),
        token_is(clang::tok::r_paren)
    );
    return matchResult = matcher.match(matchRange);
}

}
}
