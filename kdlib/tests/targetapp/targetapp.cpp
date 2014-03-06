// targetapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>

#include <windows.h>

#include "test/testfunc.h"

int breakOnRun();
int breakpointTestRun();
int memTestRun();
int stackTestRun();
int loadUnloadModuleRun();

int _tmain(int argc, _TCHAR* argv[])
{
    if ( argc <= 1 )
        return breakOnRun();

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
