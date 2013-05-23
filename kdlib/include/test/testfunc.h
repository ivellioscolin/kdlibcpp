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

//extern "C"
//void 
//__cdecl
//FuncWithName3( int a, float b );
//
//extern "C"
//void
//__stdcall
//FuncWithName4( int a, float b );
//
//extern "C"
//void
//__fastcall
//FuncWithName5( int a, float b );
//


