#include "stdafx.h"

#ifndef _WIN64
#include <vector>
#include <sstream>

typedef unsigned long long      MEMOFFSET_64;
#define FAILED(x) x != 0
#define S_OK 0
typedef int HRESULT;
typedef void *PVOID;
typedef unsigned long ULONG;
typedef unsigned long long ULONG64;

typedef HRESULT(*write_mem_func_t)(MEMOFFSET_64, void*, ULONG, unsigned long *);
write_mem_func_t WriteVirtual = 0;
write_mem_func_t WritePhysical = 0;

typedef HRESULT(*valid_region_virtual_func_t)(MEMOFFSET_64, ULONG, ULONG64 *, ULONG *);
valid_region_virtual_func_t GetValidRegionVirtual = 0;

__declspec(dllexport) void setWriteFunc(write_mem_func_t a, write_mem_func_t b, valid_region_virtual_func_t c)
{
  WriteVirtual = a;
  WritePhysical = b;
  GetValidRegionVirtual = c;
}

__declspec(dllexport) MEMOFFSET_64 addr64(MEMOFFSET_64 offset)
{
  return offset;
}

template <typename Dst, typename Src>
Dst numeric_cast(Src a)
{
  return (Dst)a;
}


class MemoryException : public std::exception
{
public:

  MemoryException( MEMOFFSET_64 targetAddr, bool phyAddr = false ) : 
    m_targetAddress(targetAddr)
  {}
  MEMOFFSET_64     m_targetAddress;
};

__declspec(noinline) bool isVaRegionValid(MEMOFFSET_64 addr, size_t length)
{ 
  addr = addr64(addr);

  HRESULT  hres;
  ULONG64  validBase = 0;
  ULONG  validSize = 0;

  if (length > 0xFFFFFFFF)
    return false;

  hres = GetValidRegionVirtual(addr, numeric_cast<ULONG>(length), &validBase, &validSize);

  return hres == S_OK && validBase == addr && validSize == length;
}
__declspec(noinline) 
void writeMemory(MEMOFFSET_64 offset, const void* buffer, size_t length, bool phyAddr = false, unsigned long *written = 0)
{
  offset = addr64(offset);

  if (written)
    *written = 0;

  HRESULT hres;

  if (phyAddr == false)
  {
    offset = addr64(offset);

    hres = WriteVirtual(offset, const_cast<PVOID>(buffer), numeric_cast<ULONG>(length), written);
  }
  else
  {
    hres = WritePhysical(offset, const_cast<PVOID>(buffer), numeric_cast<ULONG>(length), written);
  }

  if (FAILED(hres))
    throw MemoryException(offset, phyAddr);
}
template<typename T>
void writeArray( MEMOFFSET_64 offset, const std::vector<T> &values, bool phyAddr )
{
    if (values.empty())
        return;

    const auto length = values.size() * sizeof(T);

    if (!phyAddr && !isVaRegionValid(offset, length))
        throw MemoryException(offset);

    writeMemory( offset, &values[0], length, phyAddr );
}

__declspec(dllexport) void __fastcall test_cv_allreg_i386(MEMOFFSET_64 offset, const std::vector<unsigned char> &values, bool phyAddr )
{
  __debugbreak();
    writeArray(offset, values, phyAddr);
}

int test_cv_allreg()
{
  test_cv_allreg_i386(0, { 0, 0 }, 0);
  return 0;
}

#else

typedef int (*dummy_subcall_t)(char* firstptr, char* lastptr);
dummy_subcall_t dummy_subcall = 0;

__declspec(noinline) int test_cv_allreg_amd64(char *aa, char* ab)
{
  char l = 0, r = aa[0];
  char* e = ab + r * r;

  __debugbreak();
  char *f, *lt, *le;
  le = aa + aa[0];
  f = lt = e;
  while (f > aa && dummy_subcall(ab, f - r))
    f -= r;
  while (lt < le && dummy_subcall(ab, lt + r))
    lt += r;
  return (f == lt) ? e[0] : 0;
}


int test_cv_allreg()
{
  test_cv_allreg_amd64("test", 0);

  return 0;
}

#endif
