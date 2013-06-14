// targetapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>

#include "test/testfunc.h"

int breakOnRun();
int breakpointTestRun();
int memTestRun();

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

