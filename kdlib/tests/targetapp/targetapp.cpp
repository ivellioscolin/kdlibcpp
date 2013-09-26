// targetapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>

#include "test/testfunc.h"

int breakOnRun();
int breakpointTestRun();
int memTestRun();
int stackTestRun();

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
    __debugbreak();
    return std::string("Hello!");
}

float stackTestRun1( int a, const float& b, const std::string& c )
{
    stackTestRun2( a, b, c.c_str() );
    return 1.1f;
}

int stackTestRun()
{
    stackTestRun1( 10, 0.8f, "Bye" );

    return 0;
}
