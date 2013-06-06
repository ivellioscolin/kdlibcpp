#pragma once

#include <string>
#include <vector>

#include "kdlib/dbgtypedef.h"
#include "kdlib/dbgengine.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 addr64( MEMOFFSET_64 offset );
void readMemory( MEMOFFSET_64 offset, void* buffer, size_t length, bool phyAddr = false, unsigned long *readed = 0 );
bool readMemoryUnsafe( MEMOFFSET_64 offset, void* buffer, size_t length, bool phyAddr = false, unsigned long *readed = 0 );
bool isVaValid( MEMOFFSET_64 addr );
bool compareMemory( MEMOFFSET_64 addr1, MEMOFFSET_64 addr2, size_t length, bool phyAddr = false );

///////////////////////////////////////////////////////////////////////////////

unsigned char ptrByte( MEMOFFSET_64 offset );
unsigned short ptrWord( MEMOFFSET_64 offset );
unsigned long  ptrDWord( MEMOFFSET_64 offset );
unsigned long long  ptrQWord( MEMOFFSET_64 offset );
unsigned long long ptrMWord( MEMOFFSET_64 offset );
char ptrSignByte( MEMOFFSET_64 offset );
short ptrSignWord( MEMOFFSET_64 offset );
long ptrSignDWord( MEMOFFSET_64 offset );
long long ptrSignQWord( MEMOFFSET_64 offset );
long long ptrSignMWord( MEMOFFSET_64 offset );
float ptrSingleFloat( MEMOFFSET_64 offset );
double ptrDoubleFloat( MEMOFFSET_64 offset );

std::vector<unsigned char> loadBytes( MEMOFFSET_64 offset, unsigned long count, bool phyAddr = false );
std::vector<unsigned short> loadWords( MEMOFFSET_64 offset, unsigned long count, bool phyAddr = false );
std::vector<unsigned long> loadDWords( MEMOFFSET_64 offset, unsigned long count, bool phyAddr = false );
std::vector<unsigned long long> loadQWords( MEMOFFSET_64 offset, unsigned long count, bool phyAddr = false );
std::vector<char> loadSignBytes( MEMOFFSET_64 offset, unsigned long count, bool phyAddr = false );
std::vector<short> loadSignWords( MEMOFFSET_64 offset, unsigned long count, bool phyAddr = false );
std::vector<long> loadSignDWords( MEMOFFSET_64 offset, unsigned long count, bool phyAddr = false );
std::vector<long long> loadSignQWords( MEMOFFSET_64 offset, unsigned long count, bool phyAddr = false );
std::vector<float> loadFloats( MEMOFFSET_64 offset, unsigned long count, bool phyAddr = false );
std::vector<double> loadDoubles( MEMOFFSET_64 offset, unsigned long count, bool phyAddr = false );

std::string loadChars( MEMOFFSET_64 offset, unsigned long number, bool phyAddr = false );
std::wstring loadWChars( MEMOFFSET_64 offset, unsigned long  number, bool phyAddr = false );
std::string loadCStr( MEMOFFSET_64 offset );
std::wstring loadWStr( MEMOFFSET_64 offset );

MEMOFFSET_64 ptrPtr( MEMOFFSET_64 offset, size_t psize = 0 );
std::vector<MEMOFFSET_64> loadPtrs( MEMOFFSET_64 offset, unsigned long count, size_t psize = 0 );
std::vector<MEMOFFSET_64> loadPtrList( MEMOFFSET_64 offset, size_t psize = 0 );

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end
