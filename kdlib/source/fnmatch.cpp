#include "stdafx.h"

#include <boost/regex.hpp>


/////////////////////////////////////////////////////////////////////////////////

namespace kdlib {


const boost::regex  r1("\\?");
const boost::regex  r2("\\*");
const boost::regex  r3("\\.");

bool fnmatch( const std::string& pattern, const std::string& str)
{
    std::string mask = pattern;
    mask = boost::regex_replace(mask, r1, ".");
    mask = boost::regex_replace(mask, r2, ".*");
    mask = boost::regex_replace(mask, r3, "\\.");

    boost::smatch  matchResult;
    boost::regex  regEx(mask);
    return boost::regex_match(str, matchResult, regEx);
}

const boost::wregex  wr1(L"\\?");
const boost::wregex  wr2(L"\\*");
const boost::wregex  wr3(L"\\.");

bool fnmatch( const std::wstring& pattern, const std::wstring& str)
{
    std::wstring mask = pattern;
    mask = boost::regex_replace(mask, wr1, L".");
    mask = boost::regex_replace(mask, wr2, L".*");
    mask = boost::regex_replace(mask, wr3, L"\\.");

    boost::wsmatch  matchResult;
    boost::wregex  regEx(mask);
    return boost::regex_match(str, matchResult, regEx);
}

}

/////////////////////////////////////////////////////////////////////////////////
