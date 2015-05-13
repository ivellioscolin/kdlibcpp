
#pragma once

#include "basefixture.h"

#include "kdlib/process.h"
#include "kdlib/stack.h"

using namespace kdlib;

class TargetTest : public BaseFixture
{
public:
};

TEST_F(TargetTest, getTargetProcess )
{
    ASSERT_NO_THROW(startProcess(L"targetapp.exe") );
    ASSERT_NO_THROW(startProcess(L"targetapp.exe"));

    TargetProcessPtr  targetProcess;
    ASSERT_NO_THROW( targetProcess = TargetProcess::getCurrent());
    EXPECT_TRUE( 0 != targetProcess );
    ASSERT_NO_THROW( targetProcess = TargetProcess::getByIndex(0));
    EXPECT_TRUE( 0 != targetProcess );
    ASSERT_NO_THROW(targetProcess = TargetProcess::getByIndex(1));
    EXPECT_TRUE(0 != targetProcess);
    EXPECT_THROW(TargetProcess::getByIndex(2), IndexException);
}

TEST_F(TargetTest, getSystemId)
{
    ASSERT_NO_THROW( startProcess(L"targetapp.exe") );
    ASSERT_NO_THROW( startProcess(L"targetapp.exe") );
    TargetProcessPtr  targetProcess;
    ASSERT_NO_THROW( targetProcess = TargetProcess::getCurrent() );
    ASSERT_TRUE( 0 != targetProcess );
    EXPECT_NE( 0, targetProcess->getSystemId() );
    EXPECT_NE( 0, TargetProcess::getByIndex(1)->getSystemId() );
}

TEST_F(TargetTest, getPebOffset)
{
    ASSERT_NO_THROW(startProcess(L"targetapp.exe"));
    ASSERT_NO_THROW(startProcess(L"targetapp.exe"));
    EXPECT_NE(0, TargetProcess::getCurrent()->getPebOffset());
    EXPECT_NE(0, TargetProcess::getByIndex(0)->getPebOffset());
    EXPECT_NE(0, TargetProcess::getByIndex(1)->getPebOffset());
}

TEST_F(TargetTest, getExecutableName)
{
    ASSERT_NO_THROW(startProcess(L"targetapp.exe"));
    ASSERT_NO_THROW(startProcess(L"targetapp.exe"));
    EXPECT_NE(L"", TargetProcess::getCurrent()->getExecutableName());
    EXPECT_NE(L"", TargetProcess::getByIndex(0)->getExecutableName());
    EXPECT_NE(L"", TargetProcess::getByIndex(1)->getExecutableName());
}

TEST_F(TargetTest, getNumberThreads)
{
    PROCESS_DEBUG_ID  id;
    ASSERT_NO_THROW(id = startProcess(L"targetapp.exe multithread")); 
    ASSERT_NO_THROW(targetGo());
    EXPECT_EQ(5, TargetProcess::getCurrent()->getNumberThreads());
}

TEST_F(TargetTest, enumThreads)
{
    PROCESS_DEBUG_ID  procId;
    ASSERT_NO_THROW(procId = startProcess(L"targetapp.exe multithread"));
    ASSERT_NO_THROW(targetGo());

    TargetProcessPtr  process;
    ASSERT_NO_THROW(process = TargetProcess::getCurrent());

    unsigned long numberThreads = 0;
    ASSERT_NO_THROW(numberThreads = process->getNumberThreads());

    for (unsigned long i = 0; i < numberThreads; ++i)
    {
        TargetThreadPtr  thread;
        ASSERT_NO_THROW(thread = process->getThreadByIndex(i));
    }
}

TEST_F(TargetTest, EnumModules)
{
    PROCESS_DEBUG_ID  procId;
    ASSERT_NO_THROW(procId = startProcess(L"targetapp.exe multithread"));
    ASSERT_NO_THROW(targetGo());

    TargetProcessPtr  process;
    ASSERT_NO_THROW(process = TargetProcess::getCurrent());

    unsigned long numberModules = 0;
    ASSERT_NO_THROW(numberModules = process->getNumberModules());
    ASSERT_LT(0, numberModules);

    for (unsigned long i = 0; i < numberModules; ++i)
    {
        ModulePtr  module;
        ASSERT_NO_THROW(module = process->getModuleByIndex(i));
    }

}

TEST_F(TargetTest, currentThread)
{
    ASSERT_NO_THROW(startProcess(L"targetapp.exe"));

    TargetProcessPtr  targetProcess;
    ASSERT_NO_THROW(targetProcess = TargetProcess::getCurrent());

    TargetThreadPtr  targetThread;
    ASSERT_NO_THROW(targetThread = targetProcess->getCurrentThread());
    
    EXPECT_TRUE(targetThread->isCurrent());
    EXPECT_NE(0, targetThread->getSystemId());
    EXPECT_NE(0, targetThread->getTebOffset());
}

TEST_F(TargetTest, setCurrentThread)
{
    PROCESS_DEBUG_ID  procId;
    ASSERT_NO_THROW(procId = startProcess(L"targetapp.exe multithread"));
    ASSERT_NO_THROW(targetGo());

    TargetProcessPtr  process;
    ASSERT_NO_THROW(process = TargetProcess::getCurrent());

    std::set<MEMOFFSET_64>  tebList;

    unsigned long numberThreads = 0;
    ASSERT_NO_THROW(numberThreads = process->getNumberThreads());

    for (unsigned long i = 0; i < numberThreads; ++i)
    {
        TargetThreadPtr  thread;
        ASSERT_NO_THROW(thread = process->getThreadByIndex(i));
        ASSERT_NO_THROW(thread->setCurrent());
        EXPECT_TRUE(thread->isCurrent());
        tebList.insert(process->getCurrentThread()->getTebOffset());
    }

    EXPECT_EQ(numberThreads, tebList.size());
}

TEST_F(TargetTest, threadGetProcess)
{
    ASSERT_NO_THROW(startProcess(L"targetapp.exe"));

    TargetProcessPtr  targetProcess;
    ASSERT_NO_THROW(targetProcess = TargetProcess::getCurrent());

    TargetThreadPtr  targetThread;
    ASSERT_NO_THROW(targetThread = targetProcess->getCurrentThread());

    EXPECT_EQ(targetProcess->getSystemId(), targetThread->getProcess()->getSystemId());
}


TEST_F(TargetTest, restoreContext)
{
    ASSERT_NO_THROW(startProcess(L"targetapp.exe"));
    ASSERT_NO_THROW(startProcess(L"targetapp.exe"));

    ASSERT_NO_THROW(targetGo());

    ASSERT_NO_THROW(setCurrentStackFrameByIndex(2));

    unsigned int processNumber = TargetProcess::getNumber();

    for (unsigned int i = 0; i < processNumber; ++i)
        TargetProcess::getByIndex(i)->getExecutableName();

    EXPECT_EQ(2, getCurrentStackFrameNumber());
}

TEST_F(TargetTest, TwoSystem)
{
    PROCESS_DEBUG_ID   id;

    ASSERT_NO_THROW(id = startProcess(L"targetapp.exe  processtest"));
    ASSERT_NO_THROW(writeDump(L"targetapp0.dmp", false));
    ASSERT_NO_THROW(terminateProcess(id));

    ASSERT_NO_THROW(id = startProcess(L"targetapp.exe  processtest"));
    ASSERT_NO_THROW(writeDump(L"targetapp1.dmp", false));
    ASSERT_NO_THROW(terminateProcess(id));

    ASSERT_NO_THROW(loadDump(L"targetapp0.dmp"));
    ASSERT_NO_THROW(loadDump(L"targetapp1.dmp"));

    EXPECT_LE(2UL, TargetSystem::getNumber());

    for (unsigned long i = 0; i < TargetSystem::getNumber(); ++i)
    {
        TargetSystemPtr  targetSystem;
        EXPECT_NO_THROW(targetSystem = TargetSystem::getByIndex(i));

        std::wstring  targetSystemDesc;
        EXPECT_NO_THROW(targetSystemDesc = targetSystem->getDescription());
    }
}


