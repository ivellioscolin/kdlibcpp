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

class TargetSystemImpl : public TargetSystem
{
public:

    TargetSystemImpl()
    {
       m_systemId = getCurrentSystemId();
    }

    TargetSystemImpl(SYSTEM_DEBUG_ID systemId):
        m_systemId(systemId)
        {}

    virtual std::wstring getDescription() 
    {
        if ( isCurrent() )
            return  kdlib::getSystemDesc();

        ContextAutoRestore  contextRestore;

        setCurrentSystemById(m_systemId);

        return  kdlib::getSystemDesc();
    }

    virtual SYSTEM_DEBUG_ID getId() 
    {
        return m_systemId;
    }

    virtual bool isDumpAnalyzing() 
    {
        if (isCurrent())
             return kdlib::isDumpAnalyzing();

        ContextAutoRestore  contextRestore;

        setCurrentSystemById(m_systemId);

        return kdlib::isDumpAnalyzing();
     }

    virtual bool isKernelDebugging() 
    {
        if (isCurrent())
            return kdlib::isKernelDebugging();

        ContextAutoRestore  contextRestore;

        kdlib::setCurrentSystemById(m_systemId);

        return kdlib::isKernelDebugging();
    }

    virtual bool is64bitSystem() 
    {
        if (isCurrent())
            return kdlib::is64bitSystem();

        ContextAutoRestore  contextRestore;

        setCurrentSystemById(m_systemId);

        return kdlib::is64bitSystem();
    }

    virtual bool isCurrent() 
    {
        return getCurrentSystemId() == m_systemId;
    }

    virtual void setCurrent()
    {
        setCurrentSystemById(m_systemId);
    }

    virtual unsigned long getNumberProcesses() 
    {
        if (isCurrent())
            return TargetProcess::getNumber();

        ContextAutoRestore  contextRestore;

        setCurrentSystemById(m_systemId);

        return TargetProcess::getNumber();
    }

    virtual TargetProcessPtr getProcessByIndex(unsigned long index)
    {
        if (isCurrent())
            return TargetProcess::getByIndex(index);

        ContextAutoRestore  contextRestore;
   
        setCurrentSystemById(m_systemId);

        return TargetProcess::getByIndex(index);
    }

    virtual TargetProcessPtr getProcessById(PROCESS_DEBUG_ID id)
    {
        if (isCurrent())
            return TargetProcess::getById(id);

        ContextAutoRestore  contextRestore;

        setCurrentSystemById(m_systemId);

        return TargetProcess::getById(id);
    }

    virtual TargetProcessPtr getCurrentProcess() 
    {
        if (isCurrent())
            return TargetProcess::getCurrent();

        ContextAutoRestore  contextRestore;

        setCurrentSystemById(m_systemId);

        return TargetProcess::getCurrent();
    }

private:

    SYSTEM_DEBUG_ID   m_systemId;
};


unsigned long  TargetSystem::getNumber()
{
    return getNumberSystems();
}

TargetSystemPtr TargetSystem::getCurrent()
{
    return boost::make_shared<TargetSystemImpl>();
}

TargetSystemPtr TargetSystem::getByIndex(unsigned long index)
{
    return boost::make_shared<TargetSystemImpl>(getSystemIdByIndex(index));
}

TargetSystemPtr TargetSystem::getById(SYSTEM_DEBUG_ID id)
{
    return boost::make_shared<TargetSystemImpl>(id);
}

///////////////////////////////////////////////////////////////////////////////

class TargetProcessImpl : public TargetProcess
{

public:

    TargetProcessImpl() 
    {
        m_processId = getCurrentProcessId();
        m_systemId = getCurrentSystemId();
    }

    TargetProcessImpl(PROCESS_DEBUG_ID processId) 
    {
        m_processId = processId;
        m_systemId = getCurrentSystemId();
    }

    virtual ~TargetProcessImpl() 
    {}

protected:

    virtual PROCESS_DEBUG_ID getId()
    {
        return m_processId;
    }
    
    virtual std::wstring getExecutableName()
    {
        if ( isCurrent() )
            return kdlib::getProcessExecutableName();

        ContextAutoRestore  contextRestore;

        switchContext();

        return kdlib::getProcessExecutableName();
    }


    virtual PROCESS_ID getSystemId() 
    {
        if (isCurrent())
            return kdlib::getProcessSystemId();

        ContextAutoRestore  contextRestore;

        switchContext();

        return kdlib::getProcessSystemId();
    }

    virtual MEMOFFSET_64 getPebOffset() 
    {
        if (isCurrent())
            return kdlib::getProcessOffset();

        ContextAutoRestore  contextRestore;

        switchContext();

        return kdlib::getProcessOffset();
    }

    virtual bool isCurrent() {
        return getCurrentSystemId() == m_systemId && getCurrentProcessId() == m_processId;
    }

    virtual void setCurrent() {
        NOT_IMPLEMENTED();
    }

    virtual unsigned long getNumberThreads()
    {
        if (isCurrent())
            return kdlib::getNumberThreads();

        ContextAutoRestore  contextRestore;

        switchContext();

        return kdlib::getNumberThreads();
    }

    virtual TargetThreadPtr getThreadByIndex(unsigned long index)
    {
        if (isCurrent())
            return TargetThread::getByIndex(index);

        ContextAutoRestore  contextRestore;

        switchContext();

        return TargetThread::getByIndex(index);
    }

    virtual TargetThreadPtr getThreadById(THREAD_DEBUG_ID id)
    {
        if (isCurrent())
            return TargetThread::getById(id);

        ContextAutoRestore  contextRestore;

        switchContext();

        return TargetThread::getById(id);
    }

    virtual TargetThreadPtr getCurrentThread()
    {
        if (isCurrent())
            return TargetThread::getCurrent();

        ContextAutoRestore  contextRestore;

        switchContext();

        return TargetThread::getCurrent();
    }

    virtual unsigned long getNumberModules()
    {
        if (isCurrent())
            return kdlib::getNumberModules();

        ContextAutoRestore   contextRestore;

        switchContext();
        
        return kdlib::getNumberModules();
    }

    virtual ModulePtr getModuleByIndex(unsigned long index)
    {
        if (isCurrent())
            return kdlib::loadModule(kdlib::getModuleOffsetByIndex(index));

        ContextAutoRestore   contextRestore;

        switchContext();

        return kdlib::loadModule(kdlib::getModuleOffsetByIndex(index));
    }

    virtual unsigned long getNumberBreakpoints() 
    {
        if (isCurrent())
            return kdlib::getNumberBreakpoints();

        switchContext();

        return kdlib::getNumberBreakpoints();
    }

    virtual BreakpointPtr getBreakpoint(unsigned long index)
    {
        if (isCurrent())
            return kdlib::getBreakpointByIndex(index);

        ContextAutoRestore  contextRestore;

        switchContext();

        return kdlib::getBreakpointByIndex(index);
    }

protected:

    void switchContext()
    {
        if (m_systemId != getCurrentSystemId())
            setCurrentSystemById(m_systemId);

        if (m_processId != getCurrentProcessId())
            setCurrentProcessById(m_processId);
    }

    PROCESS_DEBUG_ID  m_processId;
    SYSTEM_DEBUG_ID  m_systemId;
};

TargetProcessPtr TargetProcess::getCurrent()
{
    return boost::make_shared<TargetProcessImpl>();
}

TargetProcessPtr TargetProcess::getByIndex(unsigned long index)
{
    return boost::make_shared<TargetProcessImpl>(getProcessIdByIndex(index));
}

TargetProcessPtr TargetProcess::getById(PROCESS_DEBUG_ID id)
{
    return boost::make_shared<TargetProcessImpl>(id);
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
        m_systemId = getCurrentSystemId();
    }

    TargetThreadImpl() {
        m_threadId = getCurrentThreadId();
        m_processId = getCurrentProcessId();
        m_systemId = getCurrentSystemId();
    }

    virtual ~TargetThreadImpl() {
    }

protected:

    virtual SYSTEM_DEBUG_ID getId()
    {
        return m_threadId;
    }

    virtual THREAD_ID getSystemId() 
    {
        if (isCurrent())
            return kdlib::getThreadSystemId();

        ContextAutoRestore  contextRestore;

        switchContext();

        return kdlib::getThreadSystemId();
    }

    virtual MEMOFFSET_64 getTebOffset() 
    {
        if (isCurrent())
            return kdlib::getThreadOffset(); 

        ContextAutoRestore  contextRestore;

        switchContext();

        return kdlib::getThreadOffset();
    }

    virtual TargetProcessPtr getProcess()
    {
        return boost::make_shared<TargetProcessImpl>(m_processId);
    }

    virtual bool isCurrent() {
        return getCurrentSystemId() == m_systemId && getCurrentProcessId() == m_processId && getCurrentThreadId() == m_threadId;
    }
    
    virtual void setCurrent() {
        setCurrentSystemById(m_systemId);
        if ( !isKernelDebugging() )
            setCurrentProcessById(m_processId);
        setCurrentThreadById(m_threadId);
    }

    virtual StackPtr getStack() {
        if (isCurrent())
            return kdlib::getStack();

        ContextAutoRestore  contextRestore;

        switchContext();

        return kdlib::getStack();
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

    virtual MEMOFFSET_64 getInstructionOffset()
    {
    
        if (isCurrent())
            return kdlib::getInstructionOffset();
            
        ContextAutoRestore  contextRestore;
        switchContext();

        return kdlib::getInstructionOffset();
    }

    virtual MEMOFFSET_64 getStackOffset()
    {
        if (isCurrent())
            return kdlib::getStackOffset();
            
        ContextAutoRestore  contextRestore;
        switchContext();

        return kdlib::getStackOffset();
    }
   
    virtual MEMOFFSET_64 getFrameOffset()
    {
        if (isCurrent())
            return kdlib::getFrameOffset();
            
        ContextAutoRestore  contextRestore;
        switchContext();

        return kdlib::getFrameOffset();
    }

protected:

    void switchContext()
    {
        if (m_systemId != getCurrentSystemId())
            setCurrentSystemById(m_systemId);

        if (m_processId != getCurrentProcessId())
            setCurrentProcessById(m_processId);

        if (m_threadId != getCurrentThreadId())
            setCurrentThreadById(m_threadId);
    }


    THREAD_DEBUG_ID  m_threadId;
    PROCESS_DEBUG_ID  m_processId;
    SYSTEM_DEBUG_ID  m_systemId;
};


TargetThreadPtr TargetThread::getCurrent() {
    return boost::make_shared<TargetThreadImpl>();
}

TargetThreadPtr TargetThread::getByIndex(unsigned long index) {
    return boost::make_shared<TargetThreadImpl>(getThreadIdByIndex(index));
}

TargetThreadPtr TargetThread::getById(THREAD_DEBUG_ID id)
{
    return boost::make_shared<TargetThreadImpl>(id);
}

unsigned long TargetThread::getNumber() 
{
    return getNumberThreads();
}

///////////////////////////////////////////////////////////////////////////////



} //namespace kldib
