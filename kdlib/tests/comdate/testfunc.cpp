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
   LocalStaticFunc(a,b);
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