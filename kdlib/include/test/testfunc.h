#pragma once

void 
__cdecl
CdeclFunc( int a, float b);

void
__stdcall
StdcallFunc( int a, float b );

void
__fastcall
FastcallFunc( int a, float b );

class FuncTestClass
{
public:
    void method();
    static void __cdecl staticMethod();
};

extern void ( *CdeclFuncPtr)( int a, float b);

extern void (FuncTestClass::*MethodPtr)();
extern void (__cdecl*CdeclStaticMethodPtr)();

extern FuncTestClass FuncReturnClass();

