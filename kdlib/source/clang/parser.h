#pragma once

#include <list>
#include <vector>

#include "clang/Lex/Token.h"


namespace kdlib {

std::string  tokenToStr(const clang::Token& token);

using TokenIterator = std::list<clang::Token>::const_iterator;

using TokenRange = std::pair<TokenIterator, TokenIterator>;

namespace parser {


using Token = clang::Token;
using TokenKind = clang::tok::TokenKind;

class MatchResult
{
public:

    MatchResult() : matched(false)
    {}

    MatchResult(const TokenRange& m) :
        matchedRange(m)
    {
        matched = matchedRange.first != matchedRange.second;
    }

    ~MatchResult()
    {}

    const TokenRange&  getMatchedRange() const 
    {
        assert(matched);
        return matchedRange;
    }

    TokenIterator begin() const
    {
        assert(matched);
        return matchedRange.first;
    }

    TokenIterator end() const
    {
        assert(matched);
        return matchedRange.second;
    }

    bool isMatched() const 
    {
        return matched;
    }

private:

    bool  matched;
    TokenRange  matchedRange;

};

class Matcher
{
public:

    const MatchResult& getMatchResult() const 
    {
        return matchResult;
    }

protected:

    MatchResult  matchResult;
};


template<TokenKind  tokenKind>
class Is : public Matcher
{
public:

    auto match(const TokenRange& matchRange)
    {
        auto& beg = matchRange.first;
        auto& end = matchRange.second;
        if (beg != end && (*beg).is(tokenKind))
        {
            return matchResult = MatchResult(std::make_pair(beg, std::next(beg, 1)));
        }

        return matchResult = MatchResult();
    }
};


class  TokenIs  : public Matcher
{
public:

    TokenIs(const TokenKind& tokenKind) :
        m_tokenKind(tokenKind)
    {}

    auto match(const TokenRange& matchRange)
    {
        auto& beg = matchRange.first;
        auto& end = matchRange.second;
        if (beg != end && (*beg).is(m_tokenKind))
        {
            return matchResult = MatchResult(std::make_pair(beg, std::next(beg, 1)));
        }

        return matchResult = MatchResult();
    }

private:

    TokenKind  m_tokenKind;
};

inline
auto token_is(const TokenKind& tokenKind)
{
    return TokenIs(tokenKind);
}

class BaseOpt
{};

template<typename T>
class Opt : public BaseOpt
{
public:

    Opt(T& m) : matcher(m)
    {}

    auto match(const TokenRange& matchRange)
    {
        return matcher.match(matchRange);
    }

private:

    T&  matcher;
};


template<typename T>
inline
auto opt(T& m )
{
    return Opt<T>(m);
}

template <typename...> class AllOf;

template<typename T>
class AllOf<T>
{
public:

    AllOf(T& m) : matcher(m)
    {}

    auto match(const TokenRange& matchRange)
    {
        return matcher.match(matchRange);
    }

    bool is_optional()
    {
        return std::is_base_of<BaseOpt, T>::value;
    }

private:

    T&  matcher;
};


template <typename T, typename... Ts>
class AllOf<T, Ts...> : public AllOf<Ts...>
{
public:

    AllOf(T& m, Ts&... ts) : AllOf<Ts...>(ts...), matcher(m)
    {}

    auto match(const TokenRange& matchRange)
    {
        auto result1 = matcher.match(matchRange);

        if (!result1.isMatched() )
        {
            if (std::is_base_of<BaseOpt, T>::value)
            {
                return AllOf<Ts...>::match(matchRange);
            }
            else
            {
                return MatchResult();
            }
        }

        auto result2 = AllOf<Ts...>::match(std::make_pair(result1.end(), matchRange.second));

        if (!result2.isMatched())
        {
            if (AllOf<Ts...>::is_optional())
            {
                return result1;
            }
            else
            {
                return MatchResult();
            }
        }

        return MatchResult(std::make_pair(result1.begin(), result2.end()));        
    }

    bool is_optional()
    {
        return std::is_base_of<BaseOpt, T>::value && AllOf<Ts...>::is_optional();
    }

private:
 
    T&  matcher;
};

template <typename... Ts>
inline
auto all_of(Ts&... ts)
{
    return AllOf<Ts...>(ts...);
}


template<typename...> class AnyOf;

template<typename T>
class AnyOf<T>
{
public:

    AnyOf(T& m) : matcher(m)
    {}

    auto match(const TokenRange& matchRange)
    {
        return matcher.match(matchRange);
    }

private:

    T&  matcher;
};


template <typename T, typename... Ts>
class AnyOf<T, Ts...> : public AnyOf<Ts...>
{
public:

    AnyOf(T& m, Ts&... ts) : AnyOf<Ts...>(ts...), matcher(m)
    {}

    auto match(const TokenRange& matchRange)
    {
        auto result = matcher.match(matchRange);
        if (result.isMatched())
            return result;

        return AnyOf<Ts...>::match(matchRange);
    }

private:

    T& matcher;
};

template <typename... Ts>
inline
auto any_of(Ts&... ts)
{
    return AnyOf<Ts...>(ts...);
}


template<typename T>
class Rep
{
public:

    Rep(T& m) : matcher(m)
    {}
    
    auto match(const TokenRange& matchRange)
    {
        auto& beg = matchRange.first;
        auto& end = matchRange.second;
        auto cur = beg;

        while( cur != end )
        {
            auto result = matcher.match(std::make_pair(cur, end));
            if (!result.isMatched())
                break;

            cur = result.end();
        } 

        if (beg == cur)
            return MatchResult();

        return MatchResult(std::make_pair(beg, cur));
    }

private:

    T&  matcher;
};

template<typename T>
inline
auto rep(T& matcher)
{
    return Rep<T>(matcher);
}

template<typename T>
class ListMatcher 
{
public:

    auto match(const TokenRange& matchRange)
    {
        T matcher;
        auto result = matcher.match(matchRange);
        if (result.isMatched())
            matcherList.push_back(std::move(matcher));
        return result;
    }

    auto begin() const {
        return matcherList.begin();
    }

    auto end() const {
        return matcherList.end();
    }

    auto rbegin() const {
        return matcherList.rbegin();
    }

    auto rend() const {
        return matcherList.rend();
    }

private:

    std::list<T>  matcherList;
};



class NumericMatcher : public Matcher
{
public:
    auto  match(const TokenRange& matchRange)
    {
        return matchResult = Is<clang::tok::numeric_constant>().match(matchRange);
    }
};

class IdentifierMatcher : public Matcher
{
public:

    auto  match(const TokenRange& matchRange)
    {
        return matchResult = Is<clang::tok::identifier>().match(matchRange);
    }
};





} }



