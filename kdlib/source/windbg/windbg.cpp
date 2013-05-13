#include "stdafx.h"

#include <vector>

#include <DbgEng.h>

#include <boost/tokenizer.hpp>

#include "kdlib/dbgengine.h"
#include "kdlib/windbg.h"
#include "kdlib/dbgio.h"
#include "win/dbgmgr.h"

static volatile LONG g_initCount = 0;

//////////////////////////////////////////////////////////////////////////////

namespace kdlib {
namespace windbg {

///////////////////////////////////////////////////////////////////////////////

class WindbgOut : public DbgOut
{
    virtual void write( const std::wstring& str )
    {

        g_dbgMgr->control->ControlledOutputWide(  
            DEBUG_OUTCTL_AMBIENT_TEXT,
            DEBUG_OUTPUT_NORMAL, 
            L"%ws",
            str.c_str()
            );
    }

    virtual void writedml( const std::wstring& str )
    {

        g_dbgMgr->control->ControlledOutputWide(  
            DEBUG_OUTCTL_AMBIENT_DML,
            DEBUG_OUTPUT_NORMAL, 
            L"%ws",
            str.c_str()
            );
    }
};

WindbgOut  windbgOut;

///////////////////////////////////////////////////////////////////////////////

class WindbgErr : public DbgOut
{
    virtual void write( const std::wstring& str )
    {
        g_dbgMgr->control->OutputWide(  
            DEBUG_OUTPUT_ERROR, 
            L"%ws",
            str.c_str()
            );
    }

    virtual void writedml( const std::wstring& str )
    {
        g_dbgMgr->control->OutputWide(  
            DEBUG_OUTPUT_ERROR, 
            L"%ws",
            str.c_str()
            );
    }
};

WindbgOut  windbgErr;

///////////////////////////////////////////////////////////////////////////////

class WindbgIn : public DbgIn
{
    virtual std::wstring readline()
    {
        std::vector<wchar_t>  inputBuffer(0x10000);

        ULONG  read = 0;
        g_dbgMgr->control->InputWide( &inputBuffer[0], inputBuffer.size(), &read );

        return std::wstring( &inputBuffer[0] );
    }
};

WindbgIn  windbgIn;

///////////////////////////////////////////////////////////////////////////////

void WindbgExtension::parseArgs(const char* args)
{
    typedef  boost::escaped_list_separator<char>    char_separator_t;
    typedef  boost::tokenizer< char_separator_t >   char_tokenizer_t;  

    std::string                 argsStr( args );

    char_tokenizer_t            token( argsStr , char_separator_t( "", " \t", "\"" ) );
    ArgsList    argsList;

    for ( char_tokenizer_t::iterator   it = token.begin(); it != token.end(); ++it )
    {
        if ( *it != "" )
            argsList.push_back( *it );
    }

    m_argsList = argsList;
}


///////////////////////////////////////////////////////////////////////////////

} } // kdlib::windbg namespace end


extern kdlib::windbg::WindbgExtension* WinDbgExt;

HRESULT
CALLBACK
DebugExtensionInitialize(
    PULONG  Version,
    PULONG  Flags )
{
    *Version = DEBUG_EXTENSION_VERSION( 1, 0 );
    *Flags = 0;

    if ( 1 == InterlockedIncrement( &g_initCount ) )
    {
        kdlib::initialize();

        kdlib::dbgout =&kdlib::windbg::windbgOut;
        kdlib::dbgerr = &kdlib::windbg::windbgErr;
        kdlib::dbgin = &kdlib::windbg::windbgIn;

        WinDbgExt->setUp();
    }

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////

VOID
CALLBACK
DebugExtensionUninitialize()
{
    if ( 0 == InterlockedDecrement( &g_initCount ) )
    {
        WinDbgExt->tearDown();

        kdlib::uninitialize();
    }
}


///////////////////////////////////////////////////////////////////////////////

