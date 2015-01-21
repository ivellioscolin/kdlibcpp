#include "stdafx.h"

#include "kdlib/process.h"
#include "kdlib/exceptions.h"
#include "kdlib/dbgengine.h"
#include "kdlib/breakpoint.h"

#include "win/autoswitch.h"
#include "win/dbgmgr.h"

namespace kdlib
{

///////////////////////////////////////////////////////////////////////////////

class TargetProcessImpl : public TargetProcess
{

public:

    TargetProcessImpl() 
    {
        m_processId = getCurrentProcessId();
    }

    TargetProcessImpl(PROCESS_DEBUG_ID processId) :
        m_processId(processId)
    {}

    virtual ~TargetProcessImpl() 
    {}

protected:

    virtual std::wstring getExecutableName() {

        ContextAutoRestore  contextRestore;

        if (m_processId != getCurrentProcessId())
            setCurrentProcessById(m_processId);

        const ULONG bufChars = (MAX_PATH * 2);

        boost::scoped_array< WCHAR > exeName(new WCHAR[bufChars]);
        memset(&exeName[0], 0, bufChars * sizeof(WCHAR));

        HRESULT  hres;
        ULONG tmp;
        hres = g_dbgMgr->system->GetCurrentProcessExecutableNameWide(&exeName[0], bufChars, &tmp);
        if (FAILED(hres))
            throw DbgEngException(L"IDebugSystemObjects::GetCurrentProcessExecutableNameWide", hres);

        return std::wstring(&exeName[0]);
    }

    virtual PROCESS_ID getSystemId() 
    {
        ContextAutoRestore  contextRestore;

        if (m_processId != getCurrentProcessId())
            setCurrentProcessById(m_processId);

        HRESULT  hres;
        ULONG  systemId;

        hres = g_dbgMgr->system->GetCurrentProcessSystemId(&systemId);

        if (FAILED(hres))
            throw DbgEngException(L"IDebugSystemObjects::GetCurrentProcessSystemId", hres);

        return systemId;
    }

    virtual MEMOFFSET_64 getPebOffset() 
    {
        ContextAutoRestore  contextRestore;

        if (m_processId != getCurrentProcessId())
            setCurrentProcessById(m_processId);

        HRESULT  hres;
        MEMOFFSET_64  offset;

        hres = g_dbgMgr->system->GetCurrentProcessDataOffset(&offset);

        if (FAILED(hres))
            throw DbgEngException(L"IDebugSystemObjects::GetCurrentProcessSystemId", hres);

        return offset;
    }

    virtual bool isCurrent() {
        NOT_IMPLEMENTED();
    }

    virtual void setCurrent() {
        NOT_IMPLEMENTED();
    }

    virtual unsigned long getNumberThreads()
    {
        ContextAutoRestore  contextRestore;

        if (m_processId != getCurrentProcessId())
            setCurrentProcessById(m_processId);

        HRESULT     hres;
        ULONG       number;

        hres = g_dbgMgr->system->GetNumberThreads(&number);
        if (FAILED(hres))
            throw DbgEngException(L"IDebugSystemObjects::GetNumberThreads", hres);

        return number;
    }

    virtual TargetThreadPtr getThreadByIndex(unsigned long index)
    {
        ContextAutoRestore  contextRestore;

        if (m_processId != getCurrentProcessId())
            setCurrentProcessById(m_processId);

        return TargetThread::getByIndex(index);
    }

    virtual TargetThreadPtr getCurrentThread()
    {
        ContextAutoRestore   contextRestore;

        if (m_processId != getCurrentProcessId())
            setCurrentProcessById(m_processId);

        return TargetThread::getCurrent();
    }

    virtual unsigned long getNumberModules() {
        NOT_IMPLEMENTED();
    }

    virtual ModulePtr getModuleByIndex(unsigned long index) {
        NOT_IMPLEMENTED();
    }

    virtual unsigned long getNumberBreakpoints() 
    {
        ContextAutoRestore  contextRestore;

        if (m_processId != getCurrentProcessId())
            setCurrentProcessById(m_processId);

        return getNumberBreakpoints();
    }

    virtual BreakpointPtr getBreakpoint(unsigned long index)
    {
        ContextAutoRestore  contextRestore;

        if (m_processId != getCurrentProcessId())
            setCurrentProcessById(m_processId);

        return getBreakpointByIndex(index);
    }

protected:

    PROCESS_DEBUG_ID  m_processId;
};

TargetProcessPtr TargetProcess::getCurrent()
{
    return boost::make_shared<TargetProcessImpl>();
}

TargetProcessPtr TargetProcess::getByIndex(unsigned long index)
{
    return boost::make_shared<TargetProcessImpl>(getProcessIdByIndex(index));
}

unsigned long TargetProcess::getNumber() 
{
    return getNumberProcesses();
}

///////////////////////////////////////////////////////////////////////////////

class TargetThreadImpl : public TargetThread
{

public:

    TargetThreadImpl(THREAD_DEBUG_ID id)
    {
        m_threadId = id;
        m_processId = getCurrentProcessId();
    }

    TargetThreadImpl() {
        m_threadId = getCurrentThreadId();
        m_processId = getCurrentProcessId();
    }

    virtual ~TargetThreadImpl() {
    }

protected:

    virtual THREAD_ID getSystemId() 
    {
        ContextAutoRestore  contextRestore;

        switchContext();

        HRESULT  hres;
        ULONG  systemId;

        hres = g_dbgMgr->system->GetCurrentThreadSystemId(&systemId);

        if (FAILED(hres))
            throw DbgEngException(L"IDebugSystemObjects::GetCurrentThreadSystemId", hres);

        return systemId;
    }

    virtual MEMOFFSET_64 getTebOffset() 
    {
        ContextAutoRestore  contextRestore;

        switchContext();

        HRESULT  hres;
        MEMOFFSET_64  offset;

        hres = g_dbgMgr->system->GetCurrentThreadDataOffset(&offset);

        if (FAILED(hres))
            throw DbgEngException(L"IDebugSystemObjects::GetCurrentThreadSystemId", hres);

        return offset;
    }

    virtual TargetProcessPtr getProcess()
    {
        return boost::make_shared<TargetProcessImpl>(m_processId);
    }

    virtual bool isCurrent() {
        return getCurrentProcessId() == m_processId && getCurrentThreadId() == m_threadId;
    }
    
    virtual void setCurrent() {
        setCurrentProcessById(m_processId);
        setCurrentThreadById(m_threadId);
    }

    virtual StackPtr getStack() {
        NOT_IMPLEMENTED();
    }

    virtual unsigned long getNumberRegisters() {
        NOT_IMPLEMENTED();
    }

    virtual NumVariant getRegisterByName(const std::wstring& regName) {
        NOT_IMPLEMENTED();
    }

    virtual NumVariant getRegisterByIndex(unsigned long regIndex) {
        NOT_IMPLEMENTED();
    }

protected:

    void switchContext()
    {
        if (m_processId != getCurrentProcessId())
            setCurrentProcessById(m_processId);

        if (m_threadId != getCurrentThreadId())
            setCurrentThreadById(m_threadId);
    }


    THREAD_DEBUG_ID  m_threadId;
    PROCESS_DEBUG_ID  m_processId;
};


TargetThreadPtr TargetThread::getCurrent() {
    return boost::make_shared<TargetThreadImpl>();
}

TargetThreadPtr TargetThread::getByIndex(unsigned long index) {
    return boost::make_shared<TargetThreadImpl>(getThreadIdByIndex(index));
}

unsigned long TargetThread::getNumber() 
{
    return getNumberThreads();
}

///////////////////////////////////////////////////////////////////////////////



} //namespace kldib
