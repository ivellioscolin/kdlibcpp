#pragma once

#include <vector>
#include <string>

#include <atlcomcli.h>
#include <DbgEng.h>

#include "kdlib/exceptions.h"

namespace kdlib {
namespace windbg {

///////////////////////////////////////////////////////////////////////////////

class WindbgExtension {

public:

    WindbgExtension() {}
    virtual ~WindbgExtension() {}

    virtual void setUp() {}
    virtual void tearDown() {}
    virtual void parseArgs(const char* args);

protected:

    typedef std::vector< std::string > ArgsList;

    const ArgsList& getArgs() const {
        return  m_argsList;
    }

    ArgsList  m_argsList;


};

///////////////////////////////////////////////////////////////////////////////

} } // end kdlib::windbg namespace


///////////////////////////////////////////////////////////////////////////////

#define KDLIB_WINDBG_EXTENSION_INIT(X)                                       \
X* KdlibExtImpl = new X();                                                   \
kdlib::windbg::WindbgExtension  *WinDbgExt = KdlibExtImpl


#define KDLIB_EXT_COMMAND_METHOD(METHOD_NAME)  void METHOD_NAME(void)

#define KDLIB_EXT_COMMAND_METHOD_IMPL(CLASS_NAME, METHOD_NAME)               \
extern "C"                                                                   \
HRESULT                                                                      \
CALLBACK                                                                     \
METHOD_NAME ( __in PDEBUG_CLIENT client,  __in_opt PCSTR args)               \
{                                                                            \
    ULONG  mask;                                                             \
    client->GetOutputMask( &mask );                                          \
    client->SetOutputMask( mask & ~DEBUG_OUTPUT_PROMPT );                    \
    CComQIPtr<IDebugControl4> ctrl = client;                                 \
    try  {                                                                   \
        KdlibExtImpl->parseArgs(args);                                       \
        KdlibExtImpl->METHOD_NAME();                                         \
    }                                                                        \
    catch( kdlib::DbgException& de )                                         \
    {                                                                        \
        ctrl->OutputWide( DEBUG_OUTPUT_ERROR,  L"Kdlib exception: %ws", de.getDesc().c_str() ); \
    }                                                                        \
    catch(...)                                                               \
    {                                                                        \
        ctrl->OutputWide( DEBUG_OUTPUT_ERROR,  L"Unknown pykd exception" );  \
    }                                                                        \
    client->SetOutputMask( mask );                                           \
    return S_OK;                                                             \
}                                                                            \
void CLASS_NAME::METHOD_NAME()

///////////////////////////////////////////////////////////////////////////////
