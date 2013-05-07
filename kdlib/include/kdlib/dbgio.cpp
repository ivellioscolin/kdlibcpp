#include "stdafx.h"

#include <iostream>

#include "kdlib/dbgio.h"

#include "dbgioimp.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class DbgStdOut: public DbgOut
{
public:
    virtual void write( const std::wstring& str, bool dml )
    {
        std::wcout << str;
    }
};

DbgStdOut  dbgstdout;

///////////////////////////////////////////////////////////////////////////////

class DbgStdErr : public DbgOut
{
public:
    virtual void write( const std::wstring& str, bool dml )
    {
        std::wcerr << str;
    }
};

DbgStdErr  dbgstderr;

///////////////////////////////////////////////////////////////////////////////

class DbgStdIn : public DbgIn
{
public:
    virtual std::wstring readline()
    {
        std::wstring str;
        std::wcin >> str;
        return str;
    }
};

DbgStdIn  dbgstdin;

///////////////////////////////////////////////////////////////////////////////

DbgOut  *dbgout = &dbgstdout;
DbgOut  *dbgerr = &dbgstderr;
DbgIn  *dbgin = &dbgstdin;

///////////////////////////////////////////////////////////////////////////////

void dprintln( const std::wstring &str, bool dml  )
{
    dprint( str + L"\r\n", dml );
}

///////////////////////////////////////////////////////////////////////////////

void dprint( const std::wstring &str, bool dml  )
{
    dbgout->write( str, dml );
}

///////////////////////////////////////////////////////////////////////////////

void eprintln( const std::wstring &str )
{
    eprint( str + L"\r\n" );
}

///////////////////////////////////////////////////////////////////////////////

void eprint( const std::wstring &str )
{
    dbgerr->write( str + L"\r\n" );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring dreadline()
{
    return dbgin->readline();
}


///////////////////////////////////////////////////////////////////////////////

} // end kdlib namespace

