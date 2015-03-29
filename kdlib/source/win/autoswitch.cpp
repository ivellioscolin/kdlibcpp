#include "stdafx.h"

#include "win/autoswitch.h"

namespace kdlib
{

///////////////////////////////////////////////////////////////////////////////

ContextAutoRestore::ContextAutoRestore()
{
    HRESULT      hres;

    memset( &m_localContext, 0, sizeof(m_localContext) );
    memset( &m_currentFrame, 0, sizeof(m_currentFrame) );

    g_dbgMgr->setQuietNotiification(true);

    hres = g_dbgMgr->system->GetCurrentThreadId(&m_currentThread);
    if (FAILED(hres))
    {
        m_currentThread = -1;
    }

    hres = g_dbgMgr->system->GetCurrentProcessId(&m_currentProcess);
    if (FAILED(hres))
    {
        m_currentProcess = -1;
    }

    hres = g_dbgMgr->system->GetCurrentSystemId(&m_currentSystem);
    if (FAILED(hres))
    {
        m_currentSystem = -1;
    }

    ULONG  registerNumber = 0;
    hres = g_dbgMgr->registers->GetNumberRegisters(&registerNumber);
    if (FAILED(hres))
    {
        m_savedRegCtx = false;
    }
    else
    {
        m_regValues.resize(registerNumber);
        hres = g_dbgMgr->registers->GetValues2(DEBUG_REGSRC_EXPLICIT, registerNumber, NULL, 0, &m_regValues[0]);
        m_savedRegCtx = (S_OK == hres);
    }

    hres = g_dbgMgr->symbols->GetScope(0ULL, NULL, &m_localContext, sizeof(m_localContext));   
    m_savedLocalContext = (S_OK == hres);

    hres = g_dbgMgr->symbols->GetScope(0ULL, &m_currentFrame, NULL, 0);   
    m_savedCurrentFrame = (S_OK == hres);
}


///////////////////////////////////////////////////////////////////////////////

ContextAutoRestore::~ContextAutoRestore()
{
    if (m_currentSystem != -1)
        g_dbgMgr->system->SetCurrentSystemId(m_currentSystem);

    if (m_currentProcess != -1)
        g_dbgMgr->system->SetCurrentProcessId(m_currentProcess);

    if (m_currentThread != -1)
        g_dbgMgr->system->SetCurrentThreadId(m_currentThread);

    if (m_savedRegCtx)
        g_dbgMgr->registers->SetValues2(DEBUG_REGSRC_EXPLICIT, static_cast<ULONG>(m_regValues.size()), NULL, 0, &m_regValues[0]);

    if (m_savedLocalContext)
        g_dbgMgr->symbols->SetScope(0ULL, NULL, &m_localContext, sizeof(m_localContext));

    if (m_savedCurrentFrame)
        g_dbgMgr->symbols->SetScope(0ULL, &m_currentFrame, NULL, 0);

    g_dbgMgr->setQuietNotiification(false);
}

///////////////////////////////////////////////////////////////////////////////

} // namespace kdlib
