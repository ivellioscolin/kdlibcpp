#include "stdafx.h"

#include "win/autoswitch.h"

namespace kdlib
{

///////////////////////////////////////////////////////////////////////////////

ContextAutoRestore::ContextAutoRestore()
{
    HRESULT      hres;

    g_dbgMgr->setQuietNotiification(true);

    hres = g_dbgMgr->system->GetCurrentThreadId(&m_currentThread);
    if (FAILED(hres))
    {
        g_dbgMgr->setQuietNotiification(false);
        throw DbgEngException(L"IDebugSystemObjects::GetCurrentThreadId", hres);
    }

    hres = g_dbgMgr->system->GetCurrentProcessId(&m_currentProcess);
    if (FAILED(hres))
    {
        g_dbgMgr->setQuietNotiification(false);
        throw DbgEngException(L"IDebugSystemObjects::GetCurrentProcessId", hres);
    }

    ULONG  registerNumber = 0;
    hres = g_dbgMgr->registers->GetNumberRegisters(&registerNumber);
    if (FAILED(hres))
    {
        g_dbgMgr->setQuietNotiification(false);
        throw DbgEngException(L"IDebugRegister::GetNumberRegisters", hres);
    }

    m_regValues.resize(registerNumber);

    m_savedRegCtx = true;

    hres = g_dbgMgr->registers->GetValues2(DEBUG_REGSRC_EXPLICIT, registerNumber, NULL, 0, &m_regValues[0]);
    if (FAILED(hres))
    {
        m_savedRegCtx = false;
    }

    m_savedLocalContext = true;

    hres = g_dbgMgr->symbols->GetScope(0ULL, NULL, &m_localContext, sizeof(m_localContext));
    if (FAILED(hres))
    {
        m_savedLocalContext = false;
    }
}


///////////////////////////////////////////////////////////////////////////////

ContextAutoRestore::~ContextAutoRestore()
{
    g_dbgMgr->system->SetCurrentProcessId(m_currentProcess);

    g_dbgMgr->system->SetCurrentThreadId(m_currentThread);

    if (m_savedRegCtx)
        g_dbgMgr->registers->SetValues2(DEBUG_REGSRC_EXPLICIT, static_cast<ULONG>(m_regValues.size()), NULL, 0, &m_regValues[0]);

    if (m_savedLocalContext)
        g_dbgMgr->symbols->SetScope(0ULL, NULL, &m_localContext, sizeof(m_localContext));

    g_dbgMgr->setQuietNotiification(false);
}

///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib
