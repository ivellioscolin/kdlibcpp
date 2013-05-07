// targetapp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>

int breakOnRun();
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

    return breakOnRun();
}

int breakOnRun()
{
    __debugbreak();
    return 0;
}


