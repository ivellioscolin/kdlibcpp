#pragma once

#include <list>
#include <vector>

#include "clang/Lex/Token.h"
#include "clang/Lex/Preprocessor.h"

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

    explicit MatchResult(const TokenRange& m) :
        matchedRange(m),
        matched(true)
    {
    }

    const TokenRange&  getMatchedRange() const 
    {
        assert(matched);
        return matchedRange;
    }

    const TokenIterator& begin() const
    {
        assert(matched);
        return matchedRange.first;
    }

    const TokenIterator& end() const
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

    MatchResult match(const TokenRange& matchRange)
    {
        auto& beg = matchRange.first;
        auto& end = matchRange.second;
        if (beg != end && (*beg).is(tokenKind))
        {
            return matchResult = MatchResult(std::make_pair(beg, std::next(beg)));
        }

        return matchResult = MatchResult();
    }
};


class  TokenIs  : public Matcher
{
public:

    explicit TokenIs(const TokenKind& tokenKind) :
        m_tokenKind(tokenKind)
    {}

    MatchResult match(const TokenRange& matchRange)
    {
        auto& beg = matchRange.first;
        auto& end = matchRange.second;
        if (beg != end && (*beg).is(m_tokenKind))
        {
            matchResult = MatchResult(std::make_pair(beg, std::next(beg)));
            return matchResult;
        }

        matchResult = MatchResult();
        return matchResult;
    }

private:

    TokenKind  m_tokenKind;
};

inline
auto token_is(const TokenKind& tokenKind)
{
    return TokenIs(tokenKind);
}



template<typename T>
class Opt 
{
public:

    Opt(T m) : matcher(m)
    {}

    MatchResult match(const TokenRange& matchRange)
    {
        auto result = matcher.match(matchRange);
        if (result.isMatched())
            return result;

        return MatchResult(std::make_pair(matchRange.first, matchRange.first));
    }

private:

    T  matcher;
};


template<typename T>
inline
auto opt(T&& m )
{
    return Opt<T>(std::forward<T>(m));
}


//template<typename T>
//class Followed
//{
//public:
//
//    auto match(const TokenRange& matchRange)
//    {
//        T  matcher;
//        if ( matcher.match(matchRange).isMatched() )
//            return MatchResult(std::make_pair(matchRange.first, matchRange.first));
//        return MatchResult();
//    }
//};
//
//template<typename T>
//inline
//auto followed(T& m)
//{
//    return Followed<T>();
//}


template <typename...> class AllOf;

template<typename T>
class AllOf<T>
{
public:

    AllOf(T m) : matcher(m)
    {}

    MatchResult match(const TokenRange& matchRange)
    {
        return matcher.match(matchRange);
    }

private:

    T  matcher;
};


template <typename T, typename... Ts>
class AllOf<T, Ts...> : public AllOf<Ts...>
{
public:

    AllOf(T m, const Ts... ts) : AllOf<Ts...>(ts...), matcher(m)
    {}

    MatchResult match(const TokenRange& matchRange)
    {
        MatchResult result1 = matcher.match(matchRange);

        if (!result1.isMatched())
        {
            return MatchResult();
        }

        MatchResult result2 = AllOf<Ts...>::match(std::make_pair(result1.end(), matchRange.second));

        if (!result2.isMatched())
        {
            return MatchResult();
        }

        return MatchResult(std::make_pair(result1.begin(), result2.end()));       
    }

private:
 
    T  matcher;
};

template <typename... Ts>
inline
auto all_of(Ts&&... ts)
{
    return AllOf<Ts...>(std::forward<Ts>(ts)...);
}


template<typename...> class AnyOf;

template<typename T>
class AnyOf<T>
{
public:

    AnyOf(T m) : matcher(m)
    {}

    MatchResult match(const TokenRange& matchRange)
    {
        return matcher.match(matchRange);
    }

private:

    T  matcher;
};


template <typename T, typename... Ts>
class AnyOf<T, Ts...> : public AnyOf<Ts...>
{
public:

    AnyOf(T m, Ts... ts) : AnyOf<Ts...>(ts...), matcher(m)
    {}

    MatchResult match(const TokenRange& matchRange)
    {
        MatchResult result = matcher.match(matchRange);
        if (result.isMatched())
            return result;

        return AnyOf<Ts...>::match(matchRange);
    }

private:

    T matcher;
};

template <typename... Ts>
inline
auto any_of(Ts&&... ts)
{
    return AnyOf<Ts...>(std::forward<Ts>(ts)...);
}

template <typename...> class CaptureList;

template<typename T>
class CaptureList<T>
{
public:

    CaptureList(T& c) : capture(c)
    {}

    void complete()
    {
        capture.complete();
    }

private:

    T&  capture;
};

template<typename T, typename... Ts>
class CaptureList<T, Ts...> : public CaptureList<Ts...>
{
public:

    CaptureList(T& t, Ts& ... ts) : capture(t), CaptureList<Ts...>(ts...)
    {}

    void complete() {
        capture.complete();
        CaptureList<Ts...>::complete();
    }

private:

    T&  capture;
};

template<>
class CaptureList<> 
{
public:

    CaptureList() = default;

    void complete() {
    }
};


template <typename T, typename... Cs>
class Cap
{
public:

    Cap(T& m, Cs& ... cs) : matcher(m), captures(cs...)
    {}

    MatchResult match(const TokenRange& matchRange)
    {

        MatchResult result = matcher.match(std::make_pair(matchRange.first, matchRange.second));
        if (!result.isMatched())
            return MatchResult();

        captures.complete();

        return result;
    }

private:

    T&      matcher;
    CaptureList<Cs...>  captures;
};


template<typename... Ts>
inline
auto cap(Ts & ... matchers)
{
    return Cap<Ts...>(matchers...);
}

template <typename T, typename... Cs>
class Rep
{
public:

    Rep(T m, Cs& ... cs) : matcher(m), captures(cs...)
    {}

    MatchResult match(const TokenRange& matchRange)
    {
        auto& beg = matchRange.first;
        auto& end = matchRange.second;
        auto cur = beg;

        while (cur != end)
        {
            MatchResult result = matcher.match(std::make_pair(cur, end));
            if (!result.isMatched())
                break;

            captures.complete();

            cur = result.end();
        }

        if (beg == cur)
            return MatchResult();      
        
        return MatchResult(std::make_pair(beg, cur));
    }

private:

    T      matcher;
    CaptureList<Cs...>  captures;
};



template<typename... Ts>
inline
auto rep(Ts&& ... matchers)
{
    return Rep<Ts...>(std::forward<Ts>(matchers)...);
}

template<typename T>
class ListMatcher 
{
public:

    MatchResult match(const TokenRange& matchRange)
    {
        T matcher;
        MatchResult result = matcher.match(matchRange);
        currentMatcher = std::move(matcher);
        return result;
    }

    auto begin() const
    {
        return matcherList.begin();
    }

    auto end() const 
    {
        return matcherList.end();
    }

    auto rbegin() const 
    {
        return matcherList.rbegin();
    }

    auto rend() const 
    {
        return matcherList.rend();
    }

    bool empty() const
    {
        return begin() == end();
    }

    void complete()
    {
        if (currentMatcher.getMatchResult().isMatched())
            matcherList.push_back(std::move(currentMatcher));
    }

    void push_front(T&& matcher)
    {
        matcherList.push_front(std::move(matcher));
    }

    void push_back(T&& matcher)
    {
        matcherList.push_back(std::move(matcher));
    }

private:

    std::list<T>  matcherList;
    T  currentMatcher;
};



class NumericMatcher : public Matcher
{
public:

    MatchResult  match(const TokenRange& matchRange)
    {
        auto res = Is<clang::tok::numeric_constant>().match(matchRange);
        if (res.isMatched())
            return matchResult = res;

        res = any_of(
            token_is(clang::tok::char_constant),
            token_is(clang::tok::wide_char_constant)
        ).match(matchRange);

        if (res.isMatched())
            return matchResult = res;

        res = Is<clang::tok::identifier>().match(matchRange);

        if (!res.isMatched())
            return matchResult = MatchResult();

        auto &token = *res.begin();

        std::string  strVal(token.getIdentifierInfo()->getNameStart(), token.getLength());

        if (strVal == "false" || strVal == "true")
           return matchResult = res;

        return matchResult = MatchResult();
    }
};

class IdentifierMatcher : public Matcher
{
public:

    MatchResult  match(const TokenRange& matchRange)
    {
        return matchResult = Is<clang::tok::identifier>().match(matchRange);
    }
};


} }



