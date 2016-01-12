#include "stdafx.h"

#pragma optimize("g", off)

#include "test/testfunc.h"

void
static
LocalStaticFunc( int a, float b )
{
    b = a + b;
}

void 
__cdecl
CdeclFunc( int a, float b)
{
   int c = a*10;
   float d = b/10;

   LocalStaticFunc(c,d);
}

void
__stdcall
StdcallFunc( int a, float b )
{
   LocalStaticFunc(a,b);
}

void
__fastcall
FastcallFunc( int a, float b )
{
   LocalStaticFunc(a,b);
}

void
_UnderscoreFunc( int a, float b )
{
   LocalStaticFunc(a,b);
}

void FuncTestClass::method()
{
    LocalStaticFunc(1,2);
}

void __cdecl FuncTestClass::staticMethod()
{
    LocalStaticFunc(2,3);
}

void ( *CdeclFuncPtr)( int a, float b) = &CdeclFunc;

void (FuncTestClass::*MethodPtr)() = &FuncTestClass::method;
void (__cdecl*CdeclStaticMethodPtr)() = &FuncTestClass::staticMethod;

void (*ArrayOfCdeclFuncPtr[])(int, float) = { &CdeclFunc, &CdeclFunc, &CdeclFunc };
void (FuncTestClass::*ArrayOfMethodPtr[])() = { &FuncTestClass::method, &FuncTestClass::method };



FuncTestClass FuncReturnClass()
{
    return FuncTestClass();
}

extern "C" 
{

void __stdcall PureCStdcallFunc( int a, float b )
{
    LocalStaticFunc(a,b);
}
void __cdecl PureCCdeclFunc( int a, float b )
{
    LocalStaticFunc(a,b);
}
void __fastcall PureCFastcallFunc( int a, float b )
{
    LocalStaticFunc(a,b);
}

}

void VariadicFunc(int, ...)
{
}
