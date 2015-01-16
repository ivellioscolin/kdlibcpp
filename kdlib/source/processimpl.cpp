#include "stdafx.h"

#include "kdlib/process.h"
#include "kdlib/exceptions.h"
#include "kdlib/dbgengine.h"

#include "autoswitch.h"

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
        NOT_IMPLEMENTED();
    }

    virtual PROCESS_ID getSystemId() {
        ProcessAutoSwitch autoSwitch(m_processId);
        return getProcessSystemId();
    }

    virtual MEMOFFSET_64 getPebOffset() {
        NOT_IMPLEMENTED();
    }

    virtual bool isCurrent() {
        NOT_IMPLEMENTED();
    }

    virtual void setCurrent() {
        NOT_IMPLEMENTED();
    }

    virtual unsigned long getNumberThreads() {
        NOT_IMPLEMENTED();
    }

    virtual TargetThreadPtr getThreadByIndex(unsigned long index) {
        NOT_IMPLEMENTED();
    }

    virtual unsigned long getModuleThreads() {
        NOT_IMPLEMENTED();
    }

    virtual ModulePtr getModuleByIndex(unsigned long index) {
        NOT_IMPLEMENTED();
    }

    virtual unsigned long getNumberBreakpoints() {
        NOT_IMPLEMENTED();
    }

    virtual BreakpointPtr getBreakpoint(unsigned long index) {
        NOT_IMPLEMENTED();
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

///////////////////////////////////////////////////////////////////////////////

class TargetThreadImpl : public TargetThread
{

public:

    TargetThreadImpl() {
    }

    virtual ~TargetThreadImpl() {
    }

protected:

    virtual THREAD_ID getSystemId() {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getTebOffset() {
        NOT_IMPLEMENTED();
    }

    virtual TargetProcessPtr getProcess() {
        NOT_IMPLEMENTED();
    }

    virtual bool isCurrent() {
        NOT_IMPLEMENTED();
    }
    
    virtual void setCurrent() {
        NOT_IMPLEMENTED();
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
};

TargetThreadPtr TargetThread::getCurrent() {
    return boost::make_shared<TargetThreadImpl>();
}

TargetThreadPtr TargetThread::getByIndex(unsigned long index) {
    return TargetThreadPtr();
}

///////////////////////////////////////////////////////////////////////////////



} //namespace kldib
