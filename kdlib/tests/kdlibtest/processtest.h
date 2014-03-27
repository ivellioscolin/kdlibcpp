#pragma once

#include <Windows.h>

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
