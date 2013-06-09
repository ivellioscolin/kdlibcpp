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


extern void ( *CdeclFuncPtr)( int a, float b);


