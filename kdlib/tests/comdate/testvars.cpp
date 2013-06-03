#include "stdafx.h"

#include "test/testvars.h"

#pragma pack( push,4 )

const unsigned long g_constNumValue = 0x5555;
const bool g_constBoolValue = true;

unsigned char ucharVar = 10;
unsigned short ushortVar = 1020;
unsigned long ulongVar = 10002000;
unsigned long long ulonglongVar = 1234567890;
char charVar = -5;
wchar_t wcharVar = L'G';
short shortVar = -1020;
long longVar = -1002000;
long long longlongVar = -1234567890;
float floatVar = -5.99f;
double doubleVar = 6.00000001;
bool boolVar = true;

char  helloStr[] = "Hello";
wchar_t  helloWStr[] = L"Hello";
char  helloStr1[] = "Hello";

unsigned char ucharArray[] = {0, 10, 0x78, 128, 0xFF };
unsigned short ushortArray[] = {0, 10, 0xFF, 0x8000, 0xFFFF };
unsigned long ulongArray[] = {0, 0xFF, 0x8000, 0x80000000, 0xFFFFFFFF };
unsigned long long ulonglongArray[] = {0, 0xFF, 0xFFFFFFFF, 0x8000000000000000, 0xFFFFFFFFFFFFFFFF };
char charArray[] = { 0, -10, 80, -87, 127 };
short shortArray[] = { 0, -1, -128, 32000, 0x7FFF };
long longArray[] = { 0, -1, 1, 0x7FFFFFFF, 0x80000000 };
long long longlongArray[] = { 100, -200, 123456789, -123456789, -1 };
float floatArray[] = { 1.0f, 0.001f, -199.999f, 20000.01f, 0.111111f };
double doubleArray[] = { 1.0000000, 0.0000000001, -199.99999999998, 200000.00000001, 0.3333333333 };

unsigned long long bigValue = 0x8080808080808080;
unsigned long long *pbigValue = &bigValue;
unsigned long long **ppbigValue = &pbigValue;

unsigned long long &ref_bigValue = bigValue;
unsigned long long *&ref_pbigValue = pbigValue;

void* voidPtr = &bigValue;
void* voidPtrArray[] = { voidPtr, voidPtr, voidPtr };

structTest      g_structTest = { 0, 500, true, 1, NULL };
structTest      g_structTest1 = { 0, 500, true, 1, &g_structTest };
structTest      g_testArray[2] = { { 0, 500, true, 1 }, { 2, 1500, false, 1 } };
structTest      *g_structTestPtr = &g_structTest;
structTest      **g_structTestPtrPtr = &g_structTestPtr;
structTestTypeDef    g_structTypeDef = {};


int intMatrix[2][3] = { { 0, 1, 2}, { 3, 4, 5 } };
int intMatrix2[2][3] = { { 0, 1, 2}, { 3, 4, 5 } };
int intMatrix3[2][3] = { { 0, 1, 2}, { 3, 4, 5 } };
int intMatrix4[2][3] = { { 0, 1, 2}, { 3, 4, 5 } };
int (*ptrIntMatrix)[2][3] = &intMatrix;
int (* arrIntMatrixPtrs[4])[2][3] = { &intMatrix, &intMatrix2, &intMatrix3, &intMatrix4 };
char* strArray[] = { "Hello", "Bye" };
char *(*ptrStrArray)[2] = &strArray;


template<typename T>
class MemSetter {
public:

    MemSetter( T* tptr, size_t length, const T& value = T() )
    {
        for ( size_t i = 0; i < length; ++i)
            tptr[i] = value;
    }
};


char bigCStr[0x2000 + 1] = {};
MemSetter<char>   m1 = MemSetter<char>( bigCStr, 0x2000, 'a' );

wchar_t bigWStr[0x2000 + 1] = {};
MemSetter<wchar_t>  m2 = MemSetter<wchar_t>( bigWStr, 0x2000, L'a' );

structWithNested g_structWithNested = { 10, { 20 }, 100500 };
structWithNested::Nested g_structNested = { 300 };

enumType g_constEnumThree = THREE;

structWithBits g_structWithBits = { 4, 1, 5 };
structWithSignBits g_structWithSignBits = { 4, 1, 5 }; // high bit is sign extender, so, m_bit5 = -1

structWithArray  g_structWithArray = { { 0, 2 }, 3 };

int classChild::m_staticField = 100;
classChild  g_classChild;

unionTest  g_unionTest = { 123456 };

structNullSize* g_nullSizeArray = 0;

pstructAbstract g_structAbstract = 0;

struct {
    struct {
        int m_fieldNestedStruct;
    };
    int m_fieldOfUnNamed;
} g_unNamedStruct = { { 4 }, 5 };

std::string   classBase2::m_stdstr = "hello";

template<int itemNum>
class ListCreator
{
public:
    ListCreator( listEntry &listHead ) {
        
        listHead.flink = &listHead;
        listHead.blink = &listHead;

        for ( int i = 0; i < itemNum; ++i )
        {
            listStruct  *s = new listStruct(i);
            listEntry  *entry = &s->next;

            listHead.blink->flink = entry;
            entry->blink = listHead.blink;
            entry->flink = &listHead;
            listHead.blink = entry;
        }
    }
};

listEntry  g_listHead;
ListCreator<5> list1( g_listHead );

virtualChild       g_virtChild;

#pragma pack(pop)

