#pragma once

#include <Windows.h>

#include <vector>
#include <sstream>

#include "gtest/gtest.h"
#include "kdlib/dbgengine.h"
#include "kdlib/exceptions.h"

using namespace kdlib;

class ProcessTest : public ::testing::Test
{
public:

    kdlib::PROCESS_ID  StartTargetappWithParam( const std::wstring& cmdline ) 
    {
        std::vector<wchar_t>   buffer(0x1000);
        DWORD   len = GetCurrentDirectory( buffer.size(), &buffer[0] );

        std::wstring  path = std::wstring( &buffer[0], len );
        path += L"\\targetapp.exe"; 

        buffer.clear();
        buffer.insert( buffer.begin(), path.begin(), path.end() );
        buffer.insert( buffer.end(), L' ' );
        buffer.insert( buffer.end(), cmdline.begin(), cmdline.end() );
        buffer.insert( buffer.end(), 0);

        STARTUPINFO   startupInfo = {0};
        startupInfo.cb = sizeof(startupInfo);

        PROCESS_INFORMATION  processInfo = {0};

        BOOL result = CreateProcessW(
            path.c_str(),
            &buffer[0],
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &startupInfo,
            &processInfo );

        CloseHandle( processInfo.hProcess );
        CloseHandle( processInfo.hThread );

        return processInfo.dwProcessId;
    }
};

TEST_F( ProcessTest, StartProcess )
{
    PROCESS_DEBUG_ID   id;
    ASSERT_NO_THROW( id =  startProcess(L"targetapp.exe") );
    EXPECT_NO_THROW( terminateProcess(id) );
}

TEST_F( ProcessTest, AttachProcess )
{
    PROCESS_ID  pid = StartTargetappWithParam(L"processtest");

    PROCESS_DEBUG_ID   id;
    ASSERT_NO_THROW( id = attachProcess(pid) );
    EXPECT_NO_THROW( terminateProcess(id) );
}

TEST_F( ProcessTest, CreateOpenDump )
{
    PROCESS_DEBUG_ID   id;
    ASSERT_NO_THROW( id = startProcess(L"targetapp.exe") );
    EXPECT_NO_THROW( writeDump(L"targetapp.dmp", false) );
    EXPECT_NO_THROW( terminateProcess(id) );

    ASSERT_NO_THROW( id = loadDump(L"targetapp.dmp") );
    EXPECT_NO_THROW( closeDump(id) );
}


TEST_F( ProcessTest, StartMultiProcess )
{
    std::vector<PROCESS_DEBUG_ID>  ids;

    for ( int i = 0; i < 5; ++i )
    {
        PROCESS_DEBUG_ID   id;
        ASSERT_NO_THROW( id =  startProcess(L"targetapp.exe") );
        ids.push_back(id);
    }

    std::vector<PROCESS_DEBUG_ID>::iterator  it;

    for ( it = ids.begin(); it != ids.end(); ++it )
    {
        EXPECT_NO_THROW( terminateProcess(*it) );
    }
}


TEST_F( ProcessTest, OpenMultiDump )
{
    std::vector<PROCESS_DEBUG_ID>  ids;

    std::vector<std::wstring> dumpNames;

    for ( int i = 0; i < 5; ++i )
    {
        std::wstringstream  sstr;
        sstr << L"targetapp" << i << L".dmp";
        dumpNames.push_back( sstr.str() );
    }

    for ( int i = 0; i < 5; ++i )
    {
        PROCESS_DEBUG_ID   id;
        ASSERT_NO_THROW( id = startProcess(L"targetapp.exe") );
        EXPECT_NO_THROW( writeDump(dumpNames[i], false) );
        EXPECT_NO_THROW( terminateProcess(id) );

        ASSERT_NO_THROW( id = loadDump(dumpNames[i]) );
        ids.push_back(id);
    }

    std::vector<PROCESS_DEBUG_ID>::iterator  it;

    for ( it = ids.begin(); it != ids.end(); ++it )
    {
        EXPECT_NO_THROW( closeDump(*it) );
    }
}

TEST_F( ProcessTest, DISABLED_MixedProcessDump )
{
    PROCESS_DEBUG_ID   id;

    std::vector<PROCESS_DEBUG_ID>  dumpIds;
    std::vector<PROCESS_DEBUG_ID>  processIds;

    std::vector<std::wstring> dumpNames;

    for ( int i = 0; i < 5; ++i )
    {
        std::wstringstream  sstr;
        sstr << L"targetapp" << i << L".dmp";
        dumpNames.push_back( sstr.str() );
    }

    for ( int i = 0; i < 5; ++i )
    {
        ASSERT_NO_THROW( id =  startProcess(L"targetapp.exe") );
        EXPECT_NO_THROW( writeDump(dumpNames[i], false) );
        EXPECT_NO_THROW( terminateProcess(id) );
    }

    for ( int i = 0; i < 5; ++i )
    {
        ASSERT_NO_THROW( id = loadDump(dumpNames[i]) );
        dumpIds.push_back(id);

        ASSERT_NO_THROW( id =  startProcess(L"targetapp.exe") );
        processIds.push_back(id);
    }

    EXPECT_EQ( 10, getNumberProcesses() );

    for ( int i = 0; i < 5; ++i )
    {
        EXPECT_NO_THROW( closeDump( dumpIds[i] ) );
        EXPECT_NO_THROW( terminateProcess(processIds[i]) );
    }

    EXPECT_EQ( 0, getNumberProcesses() );
}


TEST_F(ProcessTest,  ChildProcess)
{
    PROCESS_DEBUG_ID   id1;
    ASSERT_NO_THROW( id1 =  startProcess(L"targetapp.exe childprocess", true) );

    EXPECT_EQ( 1, getNumberProcesses() );
    targetGo();
    EXPECT_EQ( 2, getNumberProcesses() );

    PROCESS_DEBUG_ID   id2;
    ASSERT_NO_THROW( id2 =  startProcess(L"targetapp.exe  childprocess", true) );

    EXPECT_EQ( 3, getNumberProcesses() );
    targetGo();
    EXPECT_EQ( 4, getNumberProcesses() );

    EXPECT_NO_THROW( terminateAllProcesses() );

    EXPECT_EQ( 0, getNumberProcesses() );
}

TEST_F(ProcessTest, ThreadInfo)
{
    ASSERT_NO_THROW( startProcess(L"targetapp.exe") );
    ASSERT_EQ( DebugStatusBreak, kdlib::targetGo() );

    unsigned long threadNumber;
    ASSERT_NO_THROW( threadNumber = getNumberThreads() );

    THREAD_DEBUG_ID   threadId;
    ASSERT_NO_THROW( threadId = getCurrentThreadId() );

    THREAD_ID  threadSysId;
    ASSERT_NO_THROW( threadSysId = getThreadSystemId(threadId));
    EXPECT_EQ(threadSysId, getThreadSystemId());
    EXPECT_THROW( getThreadSystemId(-2), DbgException );

    MEMOFFSET_64  threadOffset;
    ASSERT_NO_THROW( threadOffset = getThreadOffset() );
    EXPECT_EQ( threadOffset, getThreadOffset(threadId) );
    EXPECT_EQ( threadOffset, getCurrentThread() );
    EXPECT_THROW( getThreadOffset(-3), DbgException );

    EXPECT_EQ( threadId, getThreadIdByOffset(threadOffset) );
    EXPECT_THROW( getThreadIdByOffset(0xFFFF), DbgException );

    EXPECT_EQ( threadId, getThreadIdBySystemId(threadSysId) );
    EXPECT_THROW( getThreadIdBySystemId(0xFFFFFFF), DbgException );

    EXPECT_EQ( threadId, getThreadIdByIndex(0) );
    EXPECT_THROW( getThreadIdByIndex(0xFFFFFFF), DbgException );
}


TEST_F(ProcessTest, ProcessInfo)
{
    PROCESS_DEBUG_ID  processId;
    ASSERT_NO_THROW( processId = startProcess(L"targetapp.exe") );
    ASSERT_EQ( DebugStatusBreak, kdlib::targetGo() );

    unsigned long  processNumber;
    ASSERT_NO_THROW( processNumber = getNumberProcesses() );

    EXPECT_EQ( processId, getCurrentProcessId() );

    MEMOFFSET_64  processOffest;
    ASSERT_NO_THROW( processOffest = getProcessOffset(processId) );
    EXPECT_EQ( processOffest, getProcessOffset() );
    EXPECT_EQ( processOffest, getCurrentProcess() );

    EXPECT_EQ( processId, getProcessIdByOffset(processOffest) );
    EXPECT_THROW( getProcessIdByOffset(0xFFFF), DbgException );

    PROCESS_ID  processSysId;
    ASSERT_NO_THROW( processSysId = getProcessSystemId(processId) );
    EXPECT_EQ( processSysId, getProcessSystemId() );
    EXPECT_THROW( getProcessSystemId(-5), DbgException );

    EXPECT_EQ( processId, getProcessIdByIndex(0) );
    EXPECT_THROW( getProcessIdByIndex(0xFFFF), DbgException );
}

TEST_F(ProcessTest, GetNumberProcesses)
{
    PROCESS_DEBUG_ID   process_id, dump1_id, dump2_id;

    for ( int i = 0; i < 2; ++i )
    {
        ASSERT_NO_THROW( process_id =  startProcess(L"targetapp.exe") );
        std::wstringstream  sstr;
        sstr << L"targetapp" << i << L".dmp";
        EXPECT_NO_THROW( writeDump(sstr.str(), false) );
        EXPECT_NO_THROW( terminateProcess(process_id) );
    }

    ASSERT_EQ( 0, getNumberProcesses() );
    ASSERT_NO_THROW( dump1_id = loadDump(L"targetapp0.dmp") );
    ASSERT_EQ( 1, getNumberProcesses() );
    ASSERT_NO_THROW( dump2_id = loadDump(L"targetapp1.dmp") );
    ASSERT_EQ( 2, getNumberProcesses() );
    ASSERT_NO_THROW( process_id =  startProcess(L"targetapp.exe") );
    ASSERT_EQ( 3, getNumberProcesses() );

    ASSERT_NO_THROW( closeDump(dump1_id) );
    ASSERT_EQ( 2, getNumberProcesses() );
    ASSERT_NO_THROW( terminateProcess(process_id) );
    ASSERT_EQ( 1, getNumberProcesses() );
    ASSERT_NO_THROW( closeDump(dump2_id) );
    ASSERT_EQ( 0, getNumberProcesses() );

    EXPECT_THROW( closeDump(dump2_id), DbgException );
    EXPECT_THROW( terminateProcess(process_id), DbgException );
    ASSERT_EQ( 0, getNumberProcesses() );
}

