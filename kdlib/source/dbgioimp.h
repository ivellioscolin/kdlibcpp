#pragma once 

#include <ostream>

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class DbgOut
{
public:

    virtual void write( const std::wstring& str, bool dml = false) = 0;
};

class DbgIn
{
public:
    virtual std::wstring readline() = 0;
};

extern DbgOut *dbgout;
extern DbgOut *dbgerr;
extern DbgIn  *dbgin;

///////////////////////////////////////////////////////////////////////////////

} // end kdlib namespace