#pragma once

#include "procfixture.h"

#include "kdlib/typeinfo.h"

using namespace kdlib;

class ClangTest : public ProcessFixture
{
public:

    ClangTest() : ProcessFixture( L"typetest" ) {}

};

static const wchar_t test_code1[] = L"  \
    struct TestStruct {          \
        bool   a0;               \
        char   a1;               \
        unsigned char  a2;       \
        short  a3;               \
        unsigned short  a4;      \
        int    a5;               \
        unsigned  a6;            \
        long  a7;                \
        unsigned long  a8;       \
        long long      a9;       \
        unsigned long long  a10; \
        long long  a11;          \
        float  a12;              \
        double  a13;             \
        long double  a14;        \
        void*   b0;              \
        char*   b1;              \
        unsigned char   b2[10];  \
        short&  b3;              \
        const unsigned short  b4;\
        static short  b5;        \
        volatile int  b6;        \
    };";


TEST_F(ClangTest, CompileStruct)
{
    TypeInfoPtr  compiledStruct;

    ASSERT_NO_THROW( compiledStruct = compileType(test_code1, L"TestStruct") );

    EXPECT_TRUE( compiledStruct->isUserDefined() );
    EXPECT_TRUE( compiledStruct->getElement(L"b2")->isArray() );
    EXPECT_TRUE( compiledStruct->getElement(L"b0")->isPointer() );
    EXPECT_TRUE( compiledStruct->getElement(L"b0")->deref()->isVoid() );
    EXPECT_TRUE( compiledStruct->getElement(L"a3")->isBase() );

    EXPECT_EQ( L"TestStruct", compiledStruct->getName() );

    EXPECT_LT( 0, compiledStruct->getSize() );

    EXPECT_EQ( 22, compiledStruct->getElementCount() );

    EXPECT_EQ( L"Int4B", compiledStruct->getElement(L"a5")->getName() );
    EXPECT_EQ( L"Float", compiledStruct->getElement(L"a12")->getName() );
    EXPECT_EQ( L"a3", compiledStruct->getElementName(3) );
    EXPECT_EQ( L"a8", compiledStruct->getElementName(8) );

    EXPECT_EQ( 9, compiledStruct->getElementIndex( L"a9" ) );

    EXPECT_TRUE( compiledStruct->isStaticMember(L"b5") );
    EXPECT_FALSE( compiledStruct->isStaticMember(L"b6") );
    EXPECT_TRUE( compiledStruct->isStaticMember(20) );
    EXPECT_FALSE( compiledStruct->isStaticMember(21) );

   
    EXPECT_THROW( compiledStruct->getElement(L"aa")->getName(), TypeException );
    EXPECT_THROW( compiledStruct->getElement(23), IndexException );

    std::wstring  desc;
    EXPECT_NO_THROW( desc = compiledStruct->str() );
}



TEST_F(ClangTest, CompileOption)
{
    TypeInfoPtr  compiledStruct1, compiledStruct2;

    ASSERT_NO_THROW( compiledStruct1 = compileType(test_code1, L"TestStruct", L"--target=i686-pc-windows-msvc -w") );
    ASSERT_NO_THROW( compiledStruct2 = compileType(test_code1, L"TestStruct", L"--target=x86_64-pc-windows-msvc -w") );

    EXPECT_TRUE( compiledStruct1->getSize() < compiledStruct2->getSize() );
    EXPECT_EQ( 4, compiledStruct1->getElement(L"b1")->getSize());
    EXPECT_EQ( 8, compiledStruct2->getElement(L"b1")->getSize());
}

static const wchar_t  test_code2[] = L"\
    struct TestStruct;                 \
    struct TestStruct {                \
        struct TestStruct*  next;      \
        int  value;                    \
    };                                 \
    ";

TEST_F(ClangTest, ForwardDecl)
{
    TypeInfoPtr  testStruct;
    ASSERT_NO_THROW( testStruct = compileType(test_code2, L"TestStruct") );
    EXPECT_TRUE( testStruct->getElement(L"next")->isPointer() );
    EXPECT_EQ( is64bitSystem() ? 8 : 4, testStruct->getElement(L"next")->getSize() );

    ASSERT_NO_THROW( testStruct = testStruct->getElement(L"next")->deref() );
    EXPECT_EQ( L"TestStruct", testStruct->getName() );

    std::wstring  desc;
    EXPECT_NO_THROW( desc = testStruct->getElement(L"next")->str() );
}

static const wchar_t test_code3[] = L" \
    struct Test1 {                     \
        int  a1;                       \
        int  a2;                       \
    };                                 \
                                       \
    struct Test2  {                    \
        int  b1;                       \
        struct Test1 b2;               \
        struct {                       \
            int  c1;                   \
            int  c2;                   \
        } b3;                          \
        struct {                       \
            int   c3;                  \
            int   c4;                  \
        };                             \
        int  b4;                       \
        float;                         \
    };                                 \
    ";

TEST_F(ClangTest, Nested)
{
    TypeInfoPtr  testStruct;
    ASSERT_NO_THROW( testStruct = compileType(test_code3, L"Test2") );

    EXPECT_EQ( L"a2", testStruct->getElement(L"b2")->getElementName(1) );
    EXPECT_EQ( L"c2", testStruct->getElement(L"b3")->getElementName(1) );
    EXPECT_EQ( L"c4", testStruct->getElementName(4) );

    std::wstring  desc;
    EXPECT_NO_THROW( desc = testStruct->str() );
}


static const wchar_t test_code4[] = L"\
    union Test {                      \
        int  a1;                      \
        unsigned long long  a2;       \
        float  a3;                    \
    };                                \
    ";


TEST_F(ClangTest, Union)
{
    TypeInfoPtr  testUnion;
    ASSERT_NO_THROW( testUnion = compileType(test_code4, L"Test") );

    std::wstring  desc;
    EXPECT_NO_THROW( desc = testUnion->str() );
}


static const wchar_t test_code5[] = L"\
    struct Test {                     \
        int  a1 : 11;                 \
        int  a2 : 8;                  \
        int  a3 : 31;                 \
    };                                \
    ";

TEST_F(ClangTest, BitField)
{
    TypeInfoPtr  testStruct;
    ASSERT_NO_THROW( testStruct = compileType(test_code5, L"Test") );

    EXPECT_TRUE( testStruct->getElement(L"a2")->isBitField() );
    EXPECT_EQ( 11, testStruct->getElement(L"a2")->getBitOffset() );
    EXPECT_EQ( 8, testStruct->getElement(L"a2")->getBitWidth() );

    std::wstring  desc;
    EXPECT_NO_THROW( desc = testStruct->str() );
}

static const wchar_t test_code6[] = L" \
    #include \"../../../kdlib/include/test/testvars.h\" \
    ";



TEST_F(ClangTest, Include)
{
    TypeInfoProviderPtr  typeProvider;

    ASSERT_NO_THROW( typeProvider = getTypeInfoProviderFromSource(test_code6) );

    for ( auto typeName :  {L"structTest", L"structWithNested", L"structWithArray", L"unionTest"} )
    {
        TypeInfoPtr  structFromSrc, structFromPdb;
        std::wstring  str1, str2;

        ASSERT_NO_THROW( structFromSrc = typeProvider->getTypeByName(typeName) );
        ASSERT_NO_THROW( structFromPdb = loadType(typeName) );
        EXPECT_NO_THROW( str1 = structFromSrc->str() );
        EXPECT_NO_THROW( str2 = structFromPdb->str() );
        EXPECT_TRUE( str1 == str2 );
    }
}


TEST_F(ClangTest, WindowsH)
{
    std::wstring  src = L"#include <windef.h>\r\n";
    std::wstring  typeName = L"tagPOINT";  
    std::wstring  opt = L"-I\"C:/Program Files (x86)/Windows Kits/8.1/Include/um\" \
        -I\"C:/Program Files (x86)/Windows Kits/8.1/Include/shared\" -Wno-missing-declarations -Wno-invalid-token-paste";

    TypeInfoProviderPtr  typeProvider;

    ASSERT_NO_THROW( typeProvider = getTypeInfoProviderFromSource(src, opt) );

    TypeInfoPtr  type1;

    ASSERT_NO_THROW( type1 = typeProvider->getTypeByName(typeName) );

    std::wstring  desc;
    EXPECT_NO_THROW( desc = type1->str() );
}

