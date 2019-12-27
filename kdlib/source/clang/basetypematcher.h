#pragma once

#include "exprparser.h"
#include <kdlib/typeinfo.h>

namespace kdlib {

bool isBaseTypeKeyWord(const clang::Token& token);
bool isStandardIntType(const clang::Token& token);
bool isInt64KeyWord(const clang::Token& token);

namespace parser {

class BaseTypeMatcher : public Matcher
{
public:

    MatchResult  match(const TokenRange& matchRange);

    std::string  getTypeName() const
    {
        return m_typeName;
    }

    TypeInfoPtr getBaseType() const
    {
        return m_typeInfo;
    }

    bool isConst() const
    {
        return m_const;
    }

private:

    bool m_const;

    TypeInfoPtr  m_typeInfo;

    std::string  m_typeName;

};

}
}