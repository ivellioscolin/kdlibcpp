#pragma once

#include "parser.h"

namespace kdlib {

namespace parser {


class OperandMatcher : public Matcher
{
public:

    auto  match(const TokenRange& matchRange)
    {
        return matchResult = numericMatcher.match(matchRange);
    }

    const auto& getNumeric() const
    {
        return numericMatcher;
    }

private:

    NumericMatcher  numericMatcher;
};


class UnaryRvalueOperation : public Matcher
{
public:

    auto  match(const TokenRange& matchRange)
    {
        return matchResult = any_of(
            Is<clang::tok::minus>(),
            Is<clang::tok::plus>(),
            Is<clang::tok::exclaim>(),
            Is<clang::tok::tilde>()
        ).match(matchRange);
    }
};

class ConstExpressionMatcher : public Matcher
{
public:
    auto  match(const TokenRange& matchRange)
    {
        auto matcher = all_of(opt(leftOps), operandMatcher);
        matchResult = matcher.match(matchRange);
        return matchResult;
    }

    const auto& getOperand() const {
        return operandMatcher;
    }

    const auto& getLeftOps() const
    {
        return leftOps;
    }

private:

    OperandMatcher   operandMatcher;
    ListMatcher<UnaryRvalueOperation>  leftOps;   
};


}
}
