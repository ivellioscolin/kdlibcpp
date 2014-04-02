#pragma once

#include <Windows.h>

#include <vector>
#include <sstream>

#include "gtest/gtest.h"
#include "kdlib/dbgengine.h"

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
    kdlib::PROCESS_DEBUG_ID   id;
    ASSERT_NO_THROW( id =  kdlib::startProcess(L"targetapp.exe") );
    EXPECT_NO_THROW( kdlib::terminateProcess(id) );
}

TEST_F( ProcessTest, AttachProcess )
{
    kdlib::PROCESS_ID  pid = StartTargetappWithParam(L"processtest");

    kdlib::PROCESS_DEBUG_ID   id;
    ASSERT_NO_THROW( id = kdlib::attachProcess(pid) );
    EXPECT_NO_THROW( kdlib::terminateProcess(id) );
}

TEST_F( ProcessTest, CreateOpenDump )
{
    kdlib::PROCESS_DEBUG_ID   id;
    ASSERT_NO_THROW( id =  kdlib::startProcess(L"targetapp.exe") );
    EXPECT_NO_THROW( kdlib::writeDump(L"targetapp.dmp", false) );
    EXPECT_NO_THROW( kdlib::terminateProcess(id) );

    ASSERT_NO_THROW( id = kdlib::loadDump(L"targetapp.dmp") );
    EXPECT_NO_THROW( kdlib::closeDump(id) );
}


TEST_F( ProcessTest, StartMultiProcess )
{
    std::vector<kdlib::PROCESS_DEBUG_ID>  ids;

    for ( int i = 0; i < 5; ++i )
    {
        kdlib::PROCESS_DEBUG_ID   id;
        ASSERT_NO_THROW( id =  kdlib::startProcess(L"targetapp.exe") );
        ids.push_back(id);
    }

    std::vector<kdlib::PROCESS_DEBUG_ID>::iterator  it;

    for ( it = ids.begin(); it != ids.end(); ++it )
    {
        EXPECT_NO_THROW( kdlib::terminateProcess(*it) );
    }
}


TEST_F( ProcessTest, OpenMultiDump )
{
    std::vector<kdlib::PROCESS_DEBUG_ID>  ids;

    std::vector< std::wstring > dumpNames;

    for ( int i = 0; i < 5; ++i )
    {
        std::wstringstream  sstr;
        sstr << L"targetapp" << i << L".dmp";
        dumpNames.push_back( sstr.str() );
    }

    for ( int i = 0; i < 5; ++i )
    {
        kdlib::PROCESS_DEBUG_ID   id;
        ASSERT_NO_THROW( id =  kdlib::startProcess(L"targetapp.exe") );
        EXPECT_NO_THROW( kdlib::writeDump(dumpNames[i], false) );
        EXPECT_NO_THROW( kdlib::terminateProcess(id) );

        ASSERT_NO_THROW( id = kdlib::loadDump(dumpNames[i]) );
        ids.push_back(id);
    }

    std::vector<kdlib::PROCESS_DEBUG_ID>::iterator  it;

    for ( it = ids.begin(); it != ids.end(); ++it )
    {
        EXPECT_NO_THROW( kdlib::closeDump(*it) );
    }
}

TEST_F( ProcessTest, DISABLED_MixedProcessDump )
{
    kdlib::PROCESS_DEBUG_ID   id;

    std::vector<kdlib::PROCESS_DEBUG_ID>  dumpIds;
    std::vector<kdlib::PROCESS_DEBUG_ID>  processIds;

    std::vector< std::wstring > dumpNames;

    for ( int i = 0; i < 5; ++i )
    {
        std::wstringstream  sstr;
        sstr << L"targetapp" << i << L".dmp";
        dumpNames.push_back( sstr.str() );
    }

    for ( int i = 0; i < 5; ++i )
    {
        ASSERT_NO_THROW( id =  kdlib::startProcess(L"targetapp.exe") );
        EXPECT_NO_THROW( kdlib::writeDump(dumpNames[i], false) );
        EXPECT_NO_THROW( kdlib::terminateProcess(id) );
    }

    for ( int i = 0; i < 5; ++i )
    {
        ASSERT_NO_THROW( id = kdlib::loadDump(dumpNames[i]) );
        dumpIds.push_back(id);

        ASSERT_NO_THROW( id =  kdlib::startProcess(L"targetapp.exe") );
        processIds.push_back(id);
    }

    EXPECT_EQ( 10, kdlib::getNumberProcesses() );

    for ( int i = 0; i < 5; ++i )
    {
        EXPECT_NO_THROW( kdlib::closeDump( dumpIds[i] ) );
        EXPECT_NO_THROW( kdlib::terminateProcess(processIds[i]) );
    }

    EXPECT_EQ( 0, kdlib::getNumberProcesses() );
}


TEST_F(ProcessTest,  ChildProcess)
{
    kdlib::PROCESS_DEBUG_ID   id1;
    ASSERT_NO_THROW( id1 =  kdlib::startProcess(L"targetapp.exe childprocess", true) );

    EXPECT_EQ( 1, kdlib::getNumberProcesses() );
    kdlib::targetGo();
    EXPECT_EQ( 2, kdlib::getNumberProcesses() );

    kdlib::PROCESS_DEBUG_ID   id2;
    ASSERT_NO_THROW( id2 =  kdlib::startProcess(L"targetapp.exe  childprocess", true) );

    EXPECT_EQ( 3, kdlib::getNumberProcesses() );
    kdlib::targetGo();
    EXPECT_EQ( 4, kdlib::getNumberProcesses() );

    EXPECT_NO_THROW( kdlib::terminateAllProcesses() );

    EXPECT_EQ( 0, kdlib::getNumberProcesses() );
}
