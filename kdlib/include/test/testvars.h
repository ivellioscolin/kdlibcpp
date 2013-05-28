#pragma once

#include <string>

#pragma pack( push, 4 )

extern char  helloStr[];
extern wchar_t  helloWStr[];
extern char  helloStr1[];

extern unsigned char ucharVar;
extern unsigned short ushortVar;
extern unsigned long ulongVar;
extern unsigned long long ulonglongVar;
extern char charVar;
extern wchar_t wcharVar;
extern short shortVar;
extern long longVar;
extern long long longlongVar;
extern float floatVar;
extern double doubleVar;


#define TEST_ARRAY_SIZE  5
extern unsigned char ucharArray[TEST_ARRAY_SIZE];
extern unsigned short ushortArray[TEST_ARRAY_SIZE];
extern unsigned long ulongArray[TEST_ARRAY_SIZE];
extern unsigned long long ulonglongArray[TEST_ARRAY_SIZE];
extern char charArray[TEST_ARRAY_SIZE];
extern short shortArray[TEST_ARRAY_SIZE];
extern long longArray[TEST_ARRAY_SIZE];
extern long long longlongArray[TEST_ARRAY_SIZE];
extern float floatArray[TEST_ARRAY_SIZE];
extern double doubleArray[TEST_ARRAY_SIZE];

extern unsigned long long bigValue;
extern unsigned long long *pbigValue;
extern unsigned long long **ppbigValue;

extern unsigned long long &ref_bigValue;
extern unsigned long long *&ref_pbigValue;

extern const unsigned long long bigConstValue;

extern void* voidPtr;
extern void* voidPtrArray[];

struct structTest {
    unsigned long m_field0;
    unsigned long long m_field1;
    bool m_field2;
    unsigned short m_field3;
    structTest* m_field4;
};

extern structTest      g_structTest;
extern structTest      g_structTest1;
extern structTest      g_testArray[2];
extern structTest      *g_structTestPtr;
extern structTest      **g_structTestPtrPtr;

typedef struct structTest   structTestTypeDef;
extern structTestTypeDef    g_structTypeDef;


extern int intMatrix[2][3];
extern int intMatrix2[2][3];
extern int intMatrix3[2][3];
extern int intMatrix4[2][3];
extern int (*ptrIntMatrix)[2][3];
extern int (*arrIntMatrixPtrs[4])[2][3];

extern char* strArray[];
extern char *(*ptrStrArray)[2];

extern char  bigCStr[];
extern wchar_t  bigWStr[];

struct structWithNested {

    struct Nested 
    {
        int m_nestedFiled;
    };

    int m_field;

    struct {

         int m_field2;

    } m_unnameStruct;

    struct {

         int m_field3;
    } ;
};

extern structWithNested g_structWithNested;
extern structWithNested::Nested g_structNested;

enum enumType {
    ONE = 1,
    TWO = 2,
    THREE = 3
};

extern enumType g_constEnumThree;

struct structWithBits {
    unsigned long m_bit0_4  : 5;
    unsigned long m_bit5    : 1;
    unsigned long m_bit6_8  : 3;
};

extern structWithBits g_structWithBits;


struct structWithSignBits {
    long m_bit0_4  : 5;
    long m_bit5    : 1;
    long m_bit6_8  : 3;
};

extern structWithSignBits g_structWithSignBits;


struct structWithArray {
    int   m_arrayField[2];
    int   m_noArrayField;
};

extern structWithArray  g_structWithArray;


union unionTest {
    long m_value;
    double m_doubleValue;
    structTest m_structValue;
};

extern unionTest  g_unionTest;

struct structNullSize {};
extern structNullSize* g_nullSizeArray;

struct structAbstract;
typedef struct structAbstract  *pstructAbstract;
extern pstructAbstract g_structAbstract;


class classBase1 {
public:
    int m_baseField;
    void baseMethod() const {}
    virtual void virtMethod1() =  0;
    virtual void virtMethod2() =  0;

    classBase1() :
        m_baseField( -100 )
        {}
};


class classBase2 {
    static std::string   m_stdstr;
    long    m_count;
    int     m_baseField;
public:
    classBase2() : 
        m_count( 1234 ),
        m_baseField( 100 )
        {}
};



class classChild : public classBase1, public classBase2 {

public:

    static const int  m_staticConst = 100;

    static int  m_staticField;

public:
    int m_childField;
    int m_childField2;
    structTest m_childField3;
    enumType m_enumField;
    void childMethod() const {}
    virtual void virtMethod1() {}
    virtual void virtMethod2() {}

    classChild() :
        m_enumField( THREE )
        {}
};

extern classChild  g_classChild;


#pragma pack ( pop )

