#include <stdafx.h>

#include "gtest/gtest.h"
#include "kdlib/kdlib.h"

#include "test/testvars.h"

using namespace kdlib;

TEST(TypeEvalTest, BaseType)
{
    EXPECT_EQ(L"Int1B", evalType("char")->getName());
    EXPECT_EQ(L"Int2B", evalType("short")->getName());
    EXPECT_EQ(L"Int4B", evalType("int")->getName());
    EXPECT_EQ(L"Int4B", evalType("long")->getName());
    EXPECT_EQ(L"Int8B", evalType("long long")->getName());
    EXPECT_EQ(L"UInt4B", evalType("unsigned")->getName());
    EXPECT_EQ(L"UInt1B", evalType("unsigned char")->getName());
    EXPECT_EQ(L"UInt2B", evalType("unsigned short")->getName());
    EXPECT_EQ(L"UInt4B", evalType("unsigned int")->getName());
    EXPECT_EQ(L"UInt4B", evalType("unsigned long")->getName());
    EXPECT_EQ(L"UInt8B", evalType("unsigned long long")->getName());
    EXPECT_EQ(L"Int8B", evalType("__int64")->getName());
    EXPECT_EQ(L"UInt8B", evalType("unsigned __int64")->getName());

    EXPECT_EQ(L"Int4B", evalType("signed")->getName());
    EXPECT_EQ(L"Int1B", evalType("signed char")->getName());
    EXPECT_EQ(L"Int2B", evalType("signed short")->getName());
    EXPECT_EQ(L"Int4B", evalType("signed int")->getName());
    EXPECT_EQ(L"Int4B", evalType("signed long")->getName());
    EXPECT_EQ(L"Int8B", evalType("signed long long")->getName());

    EXPECT_EQ(L"UInt2B", evalType("short unsigned")->getName());
    EXPECT_EQ(L"UInt4B", evalType("long unsigned")->getName());
    EXPECT_EQ(L"Int2B", evalType("short signed")->getName());
    EXPECT_EQ(L"Int4B", evalType("long signed")->getName());

    EXPECT_THROW(evalType("long char"), TypeException);
    EXPECT_THROW(evalType("long short"), TypeException);
    EXPECT_THROW(evalType("long long long"), TypeException);
    EXPECT_THROW(evalType("signed unsigned long"), TypeException);
    EXPECT_THROW(evalType("unsigned signed int"), TypeException);
}

TEST(TypeEvalTest, ExBaseType)
{
    EXPECT_EQ(L"Int2B", evalType("wchar_t")->getName());
    EXPECT_EQ(L"Int1B", evalType("int8_t")->getName());
    EXPECT_EQ(L"UInt1B", evalType("uint8_t")->getName());
    EXPECT_EQ(L"Int2B", evalType("int16_t")->getName());
    EXPECT_EQ(L"UInt2B", evalType("uint16_t")->getName());
    EXPECT_EQ(L"Int4B", evalType("int32_t")->getName());
    EXPECT_EQ(L"UInt4B", evalType("uint32_t")->getName());
    EXPECT_EQ(L"Int8B", evalType("int64_t")->getName());
    EXPECT_EQ(L"UInt8B", evalType("uint64_t")->getName());
    EXPECT_EQ(ptrSize() == 4 ? L"UInt4B" : L"UInt8B", evalType("size_t")->getName());
    EXPECT_EQ(ptrSize() == 4 ? L"Int4B" : L"Int8B", evalType("intptr_t")->getName());
    EXPECT_EQ(ptrSize() == 4 ? L"UInt4B" : L"UInt8B", evalType("uintptr_t")->getName());

    EXPECT_THROW(evalType("long int8_t"), TypeException);
    EXPECT_THROW(evalType("int8_t int8_t"), TypeException);
}

TEST(TypeEvalTest, SignedBaseType)
{
    EXPECT_EQ(L"Int2B", evalType("signed short")->getName());

}

TEST(TypeEvalTest, ConstBaseType)
{
    EXPECT_EQ(L"Int4B", evalType("const long const")->getName());

    EXPECT_THROW(evalType("const const"), TypeException);
    EXPECT_THROW(evalType("const *"), TypeException);
}

TEST(TypeEvalTest, ConstStdTypesType)
{
    EXPECT_EQ(L"UInt4B", evalType("const uint32_t")->getName());
    EXPECT_EQ(L"UInt4B", evalType("uint32_t const const")->getName());
}

TEST(TypeEvalTest, ConstStuct)
{
    static const char sourceCode[] = " \
    struct TestStruct {                \
    };                                 \
    TestStruct  var1;                  \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_EQ(L"TestStruct", evalType("const TestStruct", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct", evalType("const TestStruct const", typeProvider)->getName());
}

TEST(TypeEvalTest, Float)
{
    EXPECT_EQ(L"Float", evalType("float")->getName());
    EXPECT_EQ(L"Double", evalType("double")->getName());

    EXPECT_EQ(sizeof(long float), evalType("long float")->getSize());
    EXPECT_EQ(sizeof(long double), evalType("long double")->getSize());
    EXPECT_EQ(evalType("long float")->getName(), evalType("float long")->getName());

    EXPECT_THROW(evalType("long long float"), TypeException);
    EXPECT_THROW(evalType("long long double"), TypeException);
    EXPECT_THROW(evalType("unsigned double"), TypeException);
    EXPECT_THROW(evalType("double int"), TypeException);
}

TEST(TypeEvalTest, PtrBaseType)
{
    EXPECT_EQ(L"Int4B*", evalType("long*")->getName());
    EXPECT_EQ(L"UInt2B**", evalType("unsigned short **")->getName());
    EXPECT_EQ(L"Int2B***", evalType("short (***)")->getName());

    EXPECT_EQ(L"Int2B**", evalType("int16_t**")->getName());

    EXPECT_THROW(evalType("*long"), TypeException);
}

TEST(TypeEvalTest, RefType)
{
    EXPECT_EQ(L"Int4B", evalType("long&")->getName());
    EXPECT_EQ(L"Int4B", evalType("long&&")->getName());
    EXPECT_EQ(sizeof(char&), evalType("char&")->getSize());
    EXPECT_EQ(sizeof(char&&), evalType("char&")->getSize());

    EXPECT_EQ(sizeof(int*&), evalType("int*&")->getSize());
    EXPECT_EQ(L"Int4B*", evalType("int*&")->getName());

    EXPECT_EQ(sizeof(int(&)[10]), evalType("int(&)[10]")->getSize());
    EXPECT_EQ(L"Int4B[10]", evalType("int(&)[10]")->getName());

    EXPECT_THROW(evalType("long&[1]"), TypeException);
    EXPECT_THROW(evalType("long&*"), TypeException);
    EXPECT_THROW(evalType("long&&&"), TypeException);
}

TEST(TypeEvalTest, ArrayBaseType)
{
    EXPECT_EQ(L"Int4B[10]", evalType("int[10]")->getName());
    EXPECT_EQ(L"Int8B[2][3]", evalType("long long[2][3]")->getName());
    EXPECT_EQ(L"UInt8B[1]", evalType("uint64_t[1]")->getName());

    EXPECT_EQ(sizeof(short[10][2]), evalType("short[10][2]")->getSize());
}

TEST(TypeEvalTest, IncomleteArrayType)
{
    EXPECT_NO_THROW(evalType("int[]"));
    EXPECT_NO_THROW(evalType("int[][2]"));
    EXPECT_THROW(evalType("int[][]"), TypeException);
    EXPECT_THROW(evalType("int[2][]"), TypeException);
}

TEST(TypeEvalTest, ComplexBaseType)
{
    EXPECT_EQ(L"Int1B(*)[20]", evalType("char (*)[20]")->getName());
    EXPECT_EQ(sizeof(char(*)[20]), evalType("char (*)[20]")->getSize());

    EXPECT_EQ(L"Int1B(*[5])[20]", evalType("char (*[5])[20]")->getName());
    EXPECT_EQ(sizeof(char(*[5])[20]), evalType("char (*[5])[20]")->getSize());

    EXPECT_EQ(L"Int1B(*(*)[5])[20]", evalType("char (*(*)[5])[20]")->getName());
    EXPECT_EQ(sizeof(char(*(*)[5])[20]), evalType("char (*(*)[5])[20]")->getSize());
}

TEST(TypeEvalTest, IncompleteComplexType)
{
    EXPECT_EQ(sizeof(int(*)[]), evalType("int(*)[]")->getSize());
    EXPECT_EQ(sizeof(int(*)[][4]), evalType("int(*)[][4]")->getSize());
}

TEST(TypeEvalTest, StructType)
{
    struct TestStruct {
        int  field1;
        char  field2;
    };

    static const char sourceCode[] = " \
    struct TestStruct {                \
        int  field1;                   \
        char  field2;                  \
    };                                 \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_EQ(sizeof(TestStruct), evalExpr("sizeof(TestStruct)", ScopePtr(new ScopeList), typeProvider));
    EXPECT_EQ(sizeof(TestStruct::field1), evalExpr("sizeof(TestStruct::field1)", ScopePtr(new ScopeList), typeProvider));

    EXPECT_EQ(L"TestStruct*", evalType("TestStruct*", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct**", evalType("TestStruct**", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct[20]", evalType("TestStruct[20]", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct(*)[20]", evalType("TestStruct(*)[20]", typeProvider)->getName());

    EXPECT_THROW(evalType("NotExist", typeProvider), TypeException);
    EXPECT_THROW(evalType("TestStruct::notExist", typeProvider), TypeException);
    EXPECT_THROW(evalType("TestStruct::", typeProvider), TypeException);
}

TEST(TypeEvalTest, EnumType)
{
    enum TestEnum {
        ONE = 1,
        TEN = 10
    };

    static const char sourceCode[] = "\
    enum TestEnum {                   \
        ONE = 1,                      \
        TEN = 10                      \
    };                                \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_EQ(TestEnum::TEN, evalType("TestEnum::TEN", typeProvider)->getValue());
}

TEST(TypeEvalTest, Namespace)
{
    static const char sourceCode[] = " \
    namespace testspace {              \
    struct TestStruct {                \
        int  field1;                   \
        char  field2;                  \
    }; }                               \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_NO_THROW(evalType("testspace::TestStruct", typeProvider));
    EXPECT_NO_THROW(evalType("testspace::TestStruct::field1", typeProvider));
}


TEST(TypeEvalTest, Template)
{
    static const char sourceCode[] = " \
    template< typename T>              \
    struct TestStruct1 {               \
        T  field;                      \
    };                                 \
    TestStruct1<unsigned int>   val;   \
    TestStruct1<short*>  val0;         \
    template<typename T1>              \
    struct TestStruct2 {               \
        T1   field;                    \
    };                                 \
    TestStruct2<TestStruct1<double>> val1;  \
    template<typename T1, typename T2> \
    struct TestStruct4 {               \
        T1  t1;                        \
        T2  t2;                        \
    };                                 \
    TestStruct4<int, char>  val5;      \
    TestStruct4<TestStruct1<int>, unsigned long>  val6; \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_NO_THROW(evalType("TestStruct1<unsigned>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct1<short*>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct1<long double>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct2<TestStruct1<double> >", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct4<int, signed char>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct4<TestStruct1<int>, unsigned long>", typeProvider));

    EXPECT_THROW(evalType("TestStruct1<float>", typeProvider), TypeException);
    EXPECT_THROW(evalType("TestStruct1<unsigned", typeProvider), TypeException);  
    EXPECT_THROW(evalType("TestStruct1<unsigned, unsigned", typeProvider), TypeException);
}

TEST(TypeEvalTest, TemplateNumeric)
{
    static const char sourceCode[] = "\
    template<int>                     \
    struct TestStruct0 {              \
    };                                \
    template<int, unsigned short>     \
    struct TestStruct1 {              \
    };                                \
    TestStruct0<4>  val2;             \
    TestStruct0<-4>  val3;            \
    TestStruct0<true>  val4;          \
    TestStruct1<2, 0xFFFF>  val5;     \
    TestStruct0<'a'>  val6;           \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_NO_THROW(evalType("TestStruct0<'a'>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct0<0x4>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct0<-4>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct1<2,0xFFFF>", typeProvider));

    EXPECT_THROW(evalType("TestStruct0<0.1>", typeProvider), TypeException);
}

TEST(TypeEvalTest, TemplateConst)
{
    static const char sourceCode[] = "           \
    struct TestStruct {                          \
    };                                           \
    template<typename T1, typename...>           \
    struct TestStruct1 {                         \
    };                                           \
    TestStruct1<const int>   val1;               \
    TestStruct1<const int,const char>   val2;    \
    TestStruct1<const TestStruct>  val3;         \
    TestStruct1<const TestStruct1<char> >  val4; \
    TestStruct1<const TestStruct1<char>, int>  val5; \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_EQ(L"TestStruct1<const int>", evalType("TestStruct1<int const>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct1<const int>", evalType("TestStruct1<const int>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct1<const int>", evalType("TestStruct1<const int const>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct1<const int>", evalType("TestStruct1<const const int>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct1<const int,const char>", evalType("TestStruct1<const int, const char>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct1<const TestStruct>", evalType("TestStruct1<const TestStruct>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct1<const TestStruct>", evalType("TestStruct1<TestStruct const>", typeProvider)->getName());
    //EXPECT_EQ(L"TestStruct1<const TestStruct1<char>>", evalType("TestStruct1<const TestStruct1<char>>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct1<const TestStruct1<char>,int>", evalType("TestStruct1<const TestStruct1<char>, int>", typeProvider)->getName());
    
    EXPECT_THROW(evalType("TestStruct1<const>", typeProvider), TypeException);
    EXPECT_THROW(evalType("TestStruct1<int, char>", typeProvider), TypeException);
}

TEST(TypeEvalTest, TemplateConstPtr)
{
    static const char sourceCode[] = "           \
    template<typename T1, typename...>           \
    struct TestStruct1 {                         \
    };                                           \
    TestStruct1<const int*>   val1;              \
    TestStruct1<const int* const>  val2;         \
    TestStruct1<const int* const *const>  val3;  \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_EQ(L"TestStruct1<const int *>", evalType("TestStruct1<const int*>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct1<const int *>", evalType("TestStruct1<int const *>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct1<const int *const>", evalType("TestStruct1<const int* const>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct1<const int *const>", evalType("TestStruct1<const int const * const>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct1<const int *const *const>", evalType("TestStruct1<const int *const *const>", typeProvider)->getName());

    EXPECT_THROW(evalType("TestStruct1<const*>", typeProvider), TypeException);
    EXPECT_THROW(evalType("TestStruct1<const *int>", typeProvider), TypeException);
}

TEST(TypeEvalTest, TemplateConstRef)
{
    static const char sourceCode[] = "\
    template<typename T>              \
    struct TestStruct1 {              \
    };                                \
    TestStruct1<long&>   val1;        \
    TestStruct1<const long&>  val2;   \
    TestStruct1<long&&>  val3;        \
    TestStruct1<const long&&>  val4;  \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_EQ(L"TestStruct1<long &>", evalType("TestStruct1<long&>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct1<long &&>", evalType("TestStruct1<long &&>", typeProvider)->getName());

    EXPECT_EQ(L"TestStruct1<const long &>", evalType("TestStruct1<const long&>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct1<const long &&>", evalType("TestStruct1<const long&&>", typeProvider)->getName());

    EXPECT_EQ(L"TestStruct1<const long &>", evalType("TestStruct1<const long& const>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct1<const long &&>", evalType("TestStruct1<const long&& const>", typeProvider)->getName());
}

TEST(TypeEvalTest, TemplateNamespace)
{
    static const char sourceCode[] = " \
    namespace testspace {              \
    template<typename T>               \
    struct TestStruct {                \
        T     field1;                  \
        char  field2;                  \
    };                                 \
    TestStruct<float>   testVal;       \
    }                                  \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_NO_THROW(evalType("testspace::TestStruct<float>", typeProvider));
    EXPECT_NO_THROW(evalType("testspace::TestStruct<float>::field1", typeProvider));
}

TEST(TypeEvalTest, TemplateNamespace2)
{
    static const char sourceCode[] = " \
    namespace testspace {              \
    template<typename T1, typename T2> \
    struct TestStruct {                \
        T1     field1;                 \
        T2     field2;                 \
    };                                 \
    TestStruct<int,TestStruct<int,int>>   testVal;       \
    }                                  \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_NO_THROW(evalType("testspace::TestStruct<int,testspace::TestStruct<int,int> >", typeProvider));
    EXPECT_NO_THROW(evalType("testspace::TestStruct<int,testspace::TestStruct<int,int> >::field1", typeProvider));
}

TEST(TypeEvalTest, TemplateConstExpr)
{
    static const char sourceCode[] = " \
    template<int>                      \
    struct TestStruct {                \
    };                                 \
    TestStruct<10>   testVal1;         \
    TestStruct<-10>  testVal2;         \
    TestStruct<0>    testVal3;         \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_NO_THROW(evalType("TestStruct<-10>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<+10>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<!1>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<~0xFFFFFFFF>", typeProvider));
}


TEST(TypeEvalTest, TemplateConstExpr2)
{
    static const char sourceCode[] = " \
    template<int>                      \
    struct TestStruct {                \
    };                                 \
    TestStruct<10>   testVal1;         \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_NO_THROW(evalType("TestStruct<5+5>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<15-5>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<2+3+5>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<(10-5)+5>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<5+(10-(2+3))>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<(10-5)*2>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<20/2>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<120%11>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<5<<1>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<20>>1>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<11 & 0xFE>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<0x2|8>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<0x18 ^ 0x12>", typeProvider));

    EXPECT_THROW(evalType("TestStruct0<5+(10-(2+3)>", typeProvider), TypeException);
    EXPECT_THROW(evalType("TestStruct0<1/0>", typeProvider), TypeException);
}

TEST(TypeEvalTest, TemplateConstExpr3)
{
    static const char sourceCode[] = " \
    template<bool>                     \
    struct TestStruct {                \
    };                                 \
    TestStruct<true>   testVal1;       \
    TestStruct<false>  testVal2;       \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);
    EXPECT_NO_THROW(evalType("TestStruct<true>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<false>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<1==1>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<1!=0>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<1<2>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<1<=2>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<1>2>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<true>=false>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<true || 0>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<0x11 && 1>", typeProvider));

    EXPECT_THROW(evalType("TestStruct0<5&&&5>", typeProvider), TypeException);
}

TEST(TypeEvalTest, TemplateClose)
{
    static const char sourceCode[] = " \
    template<typename T1, typename T2> \
    struct TestStruct {                \
        T1     field1;                 \
        T2     field2;                 \
    };                                 \
    TestStruct<int,TestStruct<int,int>>   testVal1;  \
    TestStruct<int,TestStruct<int, TestStruct<int,int>>>     testVal2; \
    TestStruct<int,TestStruct<int,TestStruct<int,TestStruct<int,int>>>>  testVal3; \
    TestStruct<int,TestStruct<int,TestStruct<int,TestStruct<int,TestStruct<int,int>>>>>  testVal4; \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_NO_THROW(evalType("TestStruct<int,TestStruct<int,int> >", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<int,TestStruct<int,int>>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<int,TestStruct<int,TestStruct<int,int> > >", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<int,TestStruct<int,TestStruct<int,int>>>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<int,TestStruct<int,TestStruct<int,int>> >", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<int,TestStruct<int,TestStruct<int,int> >>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<int,TestStruct<int,TestStruct<int,int> >>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<int,TestStruct<int,TestStruct<int,TestStruct<int,int>>>>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<int,TestStruct<int,TestStruct<int,TestStruct<int,TestStruct<int,int>>>>>", typeProvider));
}

TEST(TypeEvalTest, TemplateUnusedType)
{
    static const char sourceCode[] = " \
    template<typename T1, typename T2> \
    struct TestStruct {                \
    };                                 \
    TestStruct<int,TestStruct<int, TestStruct<int,int>>>     testVal; \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_NO_THROW(evalType("TestStruct<int,TestStruct<int,int>>", typeProvider));
}

TEST(TypeEvalTest, Void)
{
    EXPECT_EQ(L"Void", evalType("void")->getName());
    EXPECT_EQ(L"Void*", evalType("void *")->getName());
    EXPECT_EQ(L"Void*[1]", evalType("void *[1]")->getName());

    EXPECT_THROW(evalType("void&"), TypeException);
    EXPECT_THROW(evalType("void&&"), TypeException);
    EXPECT_THROW(evalType("void[5]"), TypeException);
}

TEST(TypeEvalTest, TemplateIncompleteArray)
{
    static const char sourceCode[] = "              \
    template<typename T1>                           \
    struct TestStruct {                             \
    };                                              \
    TestStruct<int[]>                  testVal1;    \
    TestStruct<int*[]>                 testVal2;    \
    TestStruct<void*[]>                testVal3;    \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);
    
    EXPECT_NO_THROW(evalType("TestStruct<int[]>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<int*[]>", typeProvider));
    EXPECT_NO_THROW(evalType("TestStruct<void*[]>", typeProvider));
}

TEST(TypeEvalTest, TemplateVoid)
{
    static const char sourceCode[] = "              \
    template<typename T1, typename ...>             \
    struct TestStruct {                             \
    };                                              \
    TestStruct<void>                   testVal1;    \
    TestStruct<const void>             testVal2;    \
    TestStruct<void,void*,void*[1]>    testVal3;    \
    TestStruct<void,void*,void*[]>     testVal4;    \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_EQ(L"TestStruct<void>", evalType("TestStruct<void>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct<const void>", evalType("TestStruct<void const>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct<const void>", evalType("TestStruct<const void>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct<void,void *,void *[1]>", evalType("TestStruct<void,void*,void *[1]>", typeProvider)->getName());
    EXPECT_EQ(L"TestStruct<void,void *,void *[]>", evalType("TestStruct<void,void *,void*[]>", typeProvider)->getName());
}

TEST(TypeEvalTest, getTempalteArgs)
{
    static const char sourceCode[] = "                          \
    template<typename T1, typename ...>                         \
    struct TestStruct {                                         \
    };                                                          \
    TestStruct<int, const int, long *const> testVal1;    \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);
    TypeInfoPtr type = evalType("TestStruct<int, const int, long *const>", typeProvider);

    EXPECT_EQ(L"int", type->getTemplateArg(0));
    EXPECT_EQ(L"const int", type->getTemplateArg(1));
    EXPECT_EQ(L"long *const", type->getTemplateArg(2));
}


TEST(TypeEvalTest, TemplateWcharT)
{
    static const char sourceCode[] = "                          \
    template<typename T1, typename ...>                         \
    struct TestStruct {                                         \
    };                                                          \
    TestStruct<wchar_t>  testVal1;                              \
    ";

    TypeInfoProviderPtr  typeProvider = getTypeInfoProviderFromSource(sourceCode);

    EXPECT_EQ(L"TestStruct<wchar_t>", evalType("TestStruct<wchar_t>", typeProvider)->getName());
}
