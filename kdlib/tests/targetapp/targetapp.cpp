// targetapp.cpp : Defines the entry point for the console application.
//

//Русские комментарии рулят!!!!

#include "stdafx.h"

#include <string>
#include <vector>

#include <process.h>
#include <windows.h>

#include "test/testfunc.h"

int breakOnRun();
int breakpointTestRun();
int memTestRun();
int stackTestRun();
int loadUnloadModuleRun();
int startChildProcess();
void __cdecl sleepThread(void*);


int _tmain(int argc, _TCHAR* argv[])
{
    if ( argc <= 1 )
    {
        return breakOnRun();
    }

    std::wstring testGroup = std::wstring( argv[1] );

    if ( testGroup == L"memtest" )
        return breakOnRun();

    if ( testGroup == L"typetest" )
        return breakOnRun();

    if ( testGroup == L"typedvartest" )
        return breakOnRun();

    if ( testGroup == L"breakhandlertest" )
        return breakpointTestRun();

    if ( testGroup == L"stacktest" )
        return stackTestRun();

    if ( testGroup == L"loadunloadmodule" )
        return loadUnloadModuleRun();

    if ( testGroup == L"processtest" )
    {
        Sleep( INFINITE );
        return 0;
    }

    if (testGroup == L"multithread")
    {
        for (int i = 0; i < 4; ++i)
        {
            _beginthread(sleepThread, 0, 0);
        }

        Sleep(INFINITE);
        return 0;

    }

    if ( testGroup == L"childprocess" )
        return startChildProcess();

    return breakOnRun();
}

int breakOnRun()
{
    __debugbreak();
    return 0;
}

int breakpointTestRun()
{
    __debugbreak();

    CdeclFunc( 10, 10.0f );

    return 0;
}


std::string __fastcall stackTestRun2( int&a, double b, const char* c )
{
    int  localInt = 10;
    __debugbreak();
    
    if ( localInt/5 > 0 )
        return std::string("Hello!");
    else
        return std::string("Buy!");
}

float stackTestRun1( int a, const float& b, const std::string& c )
{
    static long staticLong = 1;
    double localDouble = 0.0;

    try {

        float localFloat = b;

        if ( staticLong == 1 )
        {

            char  localChars[0x100];
           //c.copy(localChars, c.size());
            strcpy_s(localChars, 0x100, c.c_str() );
            stackTestRun2( a, localFloat, localChars );
            static float staticFloat = 3.5f;
            return 1.1f + staticLong + staticFloat;
        }
    }
    catch(...)
    {
    }

    return (float)localDouble;
}

int stackTestRun()
{
    stackTestRun1( 10, 0.8f, "Bye" );

    return 0;
}


int loadUnloadModuleRun()
{
    __debugbreak();

    HMODULE  hlib = LoadLibrary(L"ws2_32.dll");
    FreeLibrary(hlib);

    return 0;
}

int startChildProcess()
{
    const DWORD nBufferLength = 0x1000;
    std::vector<wchar_t> buffer(nBufferLength);
    DWORD   len = GetCurrentDirectoryW( nBufferLength, &buffer[0] );

    std::wstring  path = std::wstring( &buffer[0], len );
    path += L"\\targetapp.exe"; 

    std::wstring  cmdline = L"processtest";

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
        CREATE_NO_WINDOW | DETACHED_PROCESS, 
        NULL,
        NULL,
        &startupInfo,
        &processInfo );

    DWORD  err = GetLastError();

    WaitForSingleObject( processInfo.hProcess, INFINITE );

    CloseHandle( processInfo.hProcess );
    CloseHandle( processInfo.hThread );

    return 0;
}

void __cdecl sleepThread(void*)
{
    Sleep(INFINITE);
}
