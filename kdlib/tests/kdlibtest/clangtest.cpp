#include <stdafx.h>

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
        wchar_t  a15;            \
        unsigned wchar_t  a16;   \
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

    EXPECT_EQ( 24, compiledStruct->getElementCount() );

    EXPECT_EQ( L"Int4B", compiledStruct->getElement(L"a5")->getName() );
    EXPECT_EQ( L"Float", compiledStruct->getElement(L"a12")->getName() );
    EXPECT_EQ( L"a3", compiledStruct->getElementName(3) );
    EXPECT_EQ( L"a8", compiledStruct->getElementName(8) );

    EXPECT_EQ( 9, compiledStruct->getElementIndex( L"a9" ) );

    EXPECT_TRUE( compiledStruct->isStaticMember(L"b5") );
    EXPECT_FALSE( compiledStruct->isStaticMember(L"b6") );
    EXPECT_TRUE( compiledStruct->isStaticMember(22) );
    EXPECT_FALSE( compiledStruct->isStaticMember(23) );

   
    EXPECT_THROW( compiledStruct->getElement(L"aa")->getName(), TypeException );
    EXPECT_THROW( compiledStruct->getElement(25), IndexException );

    std::wstring  desc;
    EXPECT_NO_THROW( desc = compiledStruct->str() );
}


TEST_F(ClangTest, CompileOption)
{
    const size_t  defaultPtrSize = sizeof(void*);

    TypeInfoPtr  compiledStruct1, compiledStruct2, compiledStruct3;

    ASSERT_NO_THROW( compiledStruct1 = compileType(test_code1, L"TestStruct", L"--target=i686-pc-windows-msvc -w") );
    ASSERT_NO_THROW( compiledStruct2 = compileType(test_code1, L"TestStruct", L"--target=x86_64-pc-windows-msvc -w") );
    ASSERT_NO_THROW( compiledStruct3 = compileType(test_code1, L"TestStruct") );

    EXPECT_TRUE( compiledStruct1->getSize() < compiledStruct2->getSize() );
    EXPECT_EQ( 4, compiledStruct1->getElement(L"b1")->getSize());
    EXPECT_EQ( 8, compiledStruct2->getElement(L"b1")->getSize());
    EXPECT_EQ( defaultPtrSize, compiledStruct3->getElement(L"b1")->getSize());
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
        union {                        \
            long  c5;                  \
            float  c6;                 \
        };                             \
        union {                        \
            char  c7;                  \
            short  c8;                 \
        } b4;                          \
        int  b5;                       \
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
    EXPECT_EQ( L"c6", testStruct->getElementName(6) );
    EXPECT_EQ( L"c8", testStruct->getElement(L"b4")->getElementName(1) );

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

static const wchar_t test_code6[] = L"\
    namespace ns1 {                   \
    namespace ns2 {                   \
       struct Test {                  \
           int  f1;                   \
       };                             \
    } }                               \
    ";

TEST_F(ClangTest, Namespace)
{
    TypeInfoPtr  testStruct;
    ASSERT_NO_THROW( testStruct = compileType(test_code6, L"ns1::ns2::Test") );

    std::wstring  desc;
    EXPECT_NO_THROW( desc = testStruct->str() );
}

static const wchar_t test_code7[] = L"\
    class Test {                      \
        long long mem1;               \
     };                               \
    ";

TEST_F(ClangTest, SimpleClass)
{
    TypeInfoPtr  testClass;
    ASSERT_NO_THROW( testClass = compileType(test_code7, L"Test") );

    std::wstring  desc;
    EXPECT_NO_THROW( desc = testClass->str() );
}

TEST_F(ClangTest, InvalidDef)
{
    EXPECT_THROW( compileType( L"struct Test { charintfloatdouble  mem1; };", L"Test")->str(), TypeException);
//    EXPECT_THROW( compileType( L"struct Test { char mem1;", L"Test")->str(), TypeException);
    EXPECT_THROW( compileType( L"aaaaaa func(bbbb p1);", L"func")->str(), TypeException);
}

TEST_F(ClangTest, StructNoDef)
{
    TypeInfoPtr  structNoDef;
    ASSERT_NO_THROW( structNoDef = compileType(L"struct StructNoDef;", L"StructNoDef") );

    EXPECT_EQ(L"StructNoDef", structNoDef->getName());

    std::wstring desc;
    EXPECT_NO_THROW(desc = structNoDef->str() );

    EXPECT_THROW(structNoDef->getElementCount(), TypeException);
    EXPECT_THROW(structNoDef->getElement(0), TypeException);
}

TEST_F(ClangTest, DISABLED_Function)
{
    const std::wstring  src = L"#include \"../../../kdlib/include/test/testfunc.h\"";

    EXPECT_EQ( L"Void(__cdecl)(Int4B, Float)", compileType(src, L"CdeclFunc")->getName());
    EXPECT_EQ( L"Void(__stdcall)(Int4B, Float)", compileType(src, L"StdcallFunc")->getName());
}

TEST_F(ClangTest, Include)
{

    const std::wstring  src = L"#include \"../../../kdlib/include/test/testvars.h\"";

    TypeInfoProviderPtr  typeProvider;

    ASSERT_NO_THROW( typeProvider = getTypeInfoProviderFromSource(src) );

    for ( auto typeName : {L"structTest", L"structWithNested", L"structWithArray", L"unionTest", L"structWithBits", L"structWithSignBits",
        /*L"testspace::testClass1",*/ L"structWithEnum", L"enumType" } )
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
    std::wstring  src = L"#include <windows.h>\r\n";
    std::wstring  opt = L"-I\"C:/Program Files (x86)/Windows Kits/8.1/Include/um\" \
        -I\"C:/Program Files (x86)/Windows Kits/8.1/Include/shared\" -w";

    TypeInfoProviderPtr  typeProvider;

    ASSERT_NO_THROW( typeProvider = getTypeInfoProviderFromSource(src, opt) );

    for ( auto typeName :  {L"tagPOINT", L"tagWNDCLASSA", L"CreateFileA"} )
    {
        TypeInfoPtr  type1;
        ASSERT_NO_THROW( type1 = typeProvider->getTypeByName(typeName) );

        std::wstring  desc;
        EXPECT_NO_THROW( desc = type1->str() );
    }
}


TEST_F(ClangTest, NtddkH)
{
    std::wstring  src = L"#include <ntddk.h>\r\n";
    std::wstring  opt = L"-I\"C:/Program Files (x86)/Windows Kits/10/Include/10.0.16299.0/km\" \
        -I\"C:/Program Files (x86)/Windows Kits/8.1/Include/shared\" \
        -D_AMD64_ --target=x86_64-pc-windows-msvc -w";

    TypeInfoProviderPtr  typeProvider;

    ASSERT_NO_THROW( typeProvider = getTypeInfoProviderFromSource(src, opt) );

    for ( auto typeName :  {L"_UNICODE_STRING", L"_IRP", L"_MDL", L"_FILE_OBJECT", L"_DEVICE_OBJECT", L"_PS_CREATE_NOTIFY_INFO",
     L"ZwCreateFile"} )
    {
        TypeInfoPtr  type1;
        ASSERT_NO_THROW( type1 = typeProvider->getTypeByName(typeName) );

        std::wstring  desc;
        EXPECT_NO_THROW( desc = type1->str() );
    }
}

static const wchar_t test_typedef_src[] = L"\
                                            \
    typedef  int   int_def;                 \
                                            \
    typedef struct _Test {                  \
        int      field1;                    \
        int_def  field2;                    \
     } TestDef;                             \
                                            \
     typedef TestDef   TEST;                \
     typedef TestDef*  PTEST;               \
     typedef TestDef   **PPTEST;            \
    ";

TEST_F(ClangTest, Typedef)
{
    TypeInfoProviderPtr  typeProvider;

    ASSERT_NO_THROW( typeProvider = getTypeInfoProviderFromSource(test_typedef_src) );

    TypeInfoPtr  intType;
    ASSERT_NO_THROW( intType = typeProvider->getTypeByName(L"int_def") );
    ASSERT_EQ(L"Int4B", intType->getName() );

    TypeInfoPtr  structType;
    ASSERT_NO_THROW( structType = typeProvider->getTypeByName(L"TestDef") );

    ASSERT_EQ(L"_Test", structType->getName() );
    ASSERT_EQ(L"Int4B", structType->getElement(L"field1")->getName() );
    ASSERT_EQ(L"Int4B", structType->getElement(L"field2")->getName() );

    ASSERT_EQ(L"_Test", typeProvider->getTypeByName(L"TEST")->getName() );
    ASSERT_EQ(L"_Test*", typeProvider->getTypeByName(L"PTEST")->getName());
    ASSERT_EQ(L"_Test**", typeProvider->getTypeByName(L"PPTEST")->getName());
}

TEST_F(ClangTest, TypeProviderEnum)
{
   const std::wstring  src = L"#include \"../../../kdlib/include/test/testvars.h\"";

    TypeInfoProviderPtr  typeProvider;

    ASSERT_NO_THROW( typeProvider = getTypeInfoProviderFromSource(src) );

    TypeInfoEnumeratorPtr  typeEnum;
    size_t  count;

    ASSERT_NO_THROW( typeEnum = typeProvider->getTypeEnumerator() );
    for ( count = 0; 0 != typeEnum->Next(); ++count);
    EXPECT_LT(1000, count);

    ASSERT_NO_THROW( typeEnum = typeProvider->getTypeEnumerator(L"struct*") );
    for ( count = 0; 0 != typeEnum->Next(); ++count);
    EXPECT_EQ(14, count);
}



TEST_F(ClangTest, TemplateStruct)
{
    static const wchar_t srcCode[] =  L"\
    template<typename T1, typename T2>      \
    struct TestStruct {                     \
        T1  field1;                         \
        T2  field2;                         \
    };                                      \
    TestStruct<int,float>  var1;            \
    TestStruct<char, unsigned long>  var2;  \
    ";

    TypeInfoProviderPtr  typeProvider;
    ASSERT_NO_THROW(typeProvider = getTypeInfoProviderFromSource(srcCode));

    EXPECT_EQ(L"Int4B", typeProvider->getTypeByName(L"TestStruct<int,float>")->getElement(0)->getName());
    EXPECT_EQ(L"UInt4B", typeProvider->getTypeByName(L"TestStruct<char,unsigned long>")->getElement(1)->getName());
    EXPECT_TRUE(typeProvider->getTypeByName(L"TestStruct<char,unsigned long>")->isTemplate());
    EXPECT_THROW(typeProvider->getTypeByName(L"TestStruct"), TypeException);
    EXPECT_THROW(typeProvider->getTypeByName(L"TestStruct<float>"), TypeException);
}

TEST_F(ClangTest, TemplateNestedStruct)
{
    static const wchar_t srcCode[] = L"\
    template<typename T>                    \
    struct TestStruct {                     \
        T   field;                          \
    };                                      \
    TestStruct<TestStruct<int>>  var1;      \
    TestStruct<TestStruct<TestStruct<int>>>  var2;  \
    ";

    TypeInfoProviderPtr  typeProvider;
    ASSERT_NO_THROW(typeProvider = getTypeInfoProviderFromSource(srcCode));

    EXPECT_NO_THROW(typeProvider->getTypeByName(L"TestStruct<TestStruct<int> >"));
    EXPECT_NO_THROW(typeProvider->getTypeByName(L"TestStruct<TestStruct<TestStruct<int> > >"));
}

TEST_F(ClangTest, TemplateValueStruct)
{
    static const wchar_t srcCode[] = L"\
    template<int v1, long v2>          \
    struct TestStruct {                \
        int  field1;                   \
    };                                 \
    TestStruct<2,-3>  var1;            \
    ";

    TypeInfoProviderPtr  typeProvider;
    ASSERT_NO_THROW(typeProvider = getTypeInfoProviderFromSource(srcCode));

    EXPECT_NO_THROW(typeProvider->getTypeByName(L"TestStruct<2,-3>"));
    EXPECT_THROW(typeProvider->getTypeByName(L"TestStruct<2,3>"), TypeException);
}

TEST_F(ClangTest, MultipleInheritance)
{
    static const wchar_t srcCode[] = L"\
    class Base1 {                      \
        int  field1;                   \
    };                                 \
    class Base2 {                      \
        int  field2;                   \
    };                                 \
    class TestClass : public Base1, private Base2 {  \
        char  field3;                  \
    };                                 \
    ";

    TypeInfoProviderPtr  typeProvider;
    ASSERT_NO_THROW(typeProvider = getTypeInfoProviderFromSource(srcCode));

    TypeInfoPtr  typeInfo;
    ASSERT_NO_THROW(typeInfo = typeProvider->getTypeByName(L"TestClass"));
    EXPECT_EQ(3, typeInfo->getElementCount());
    EXPECT_EQ(2, typeInfo->getBaseClassesCount());
    EXPECT_EQ(L"field1", typeInfo->getBaseClass(0)->getElementName(0));
    EXPECT_EQ(L"field2", typeInfo->getBaseClass(L"Base2")->getElementName(0));
    EXPECT_FALSE(typeInfo->getBaseClassOffset(0) == typeInfo->getBaseClassOffset(L"Base2"));

    std::wstring  str;
    EXPECT_NO_THROW(str = typeInfo->str());
}

TEST_F(ClangTest, NestedEnum)
{
    static const wchar_t srcCode[] = L"\
    struct Test {                      \
        enum {                         \
           VAL = 1                     \
        };                             \
        enum NestedEnum {              \
          VAL1 = 2                     \
        };                             \
    };                                 \
    ";

    TypeInfoPtr  testStruct;
    ASSERT_NO_THROW(testStruct = compileType(srcCode, L"Test"));

    EXPECT_EQ(1, *testStruct->getElement(L"VAL"));
    EXPECT_EQ(2, *testStruct->getElement(L"VAL1"));


}

TEST_F(ClangTest, PtrToIncompleteArray)
{
    static const wchar_t srcCode[] = L" \
    struct Test {                       \
        int(*a)[];                      \
    };                                  \
    ";

    TypeInfoPtr  testStruct;
    ASSERT_NO_THROW(testStruct = compileType(srcCode, L"Test"));
    EXPECT_NO_THROW(testStruct->getElement(L"a")->getSize());
    EXPECT_NO_THROW(testStruct->getElement(L"a")->deref());
    EXPECT_TRUE(testStruct->getElement(L"a")->deref()->isIncomplete());
    EXPECT_THROW(testStruct->getElement(L"a")->deref()->getSize(), TypeException);
}

TEST_F(ClangTest, PtrToIncompleteStruct)
{
    static const wchar_t srcCode[] = L" \
    struct Test1;                       \
    struct Test2 {                      \
        Test1*  t;                      \
    };                                  \
    ";

    TypeInfoPtr  testStruct;
    ASSERT_NO_THROW(testStruct = compileType(srcCode, L"Test2"));
    EXPECT_NO_THROW(testStruct->getElement(L"t")->getSize());
    EXPECT_NO_THROW(testStruct->getElement(L"t")->deref());
    EXPECT_TRUE(testStruct->getElement(L"t")->deref()->isIncomplete());
    EXPECT_THROW(testStruct->getElement(L"t")->deref()->getSize(), TypeException);
}

TEST_F(ClangTest, isInheritedMember)
{
    static const wchar_t srcCode[] = L" \
    struct Base {                       \
       int mbase;                       \
    };                                  \
    struct Test : Base {                \
       int mchild;                      \
    };                                  \
    ";

    TypeInfoPtr  testStruct;
    ASSERT_NO_THROW(testStruct = compileType(srcCode, L"Test"));
    EXPECT_TRUE(testStruct->isInheritedMember(L"mbase"));
    EXPECT_FALSE(testStruct->isInheritedMember(L"mchild"));
    EXPECT_TRUE(testStruct->isInheritedMember(0));
    EXPECT_FALSE(testStruct->isInheritedMember(1));
 }

TEST_F(ClangTest, EnumFuncNames)
{
    static const wchar_t srcCode[] = L"      \
    void func(int, char);                    \
    inline void func1() {}                   \
    template <int> int func2() { return 0; } \
    template<> int func2<2>() { return 1; }  \
    namespace testns {                       \
        char func3(void);                    \
    }                                        \
    class testcls {                          \
      void method(int);                      \
    };                                       \
    ";

    auto  symEnum = getSymbolProviderFromSource(srcCode)->getSymbolEnumerator();

    std::vector<std::wstring> symbols;
    std::vector<std::wstring> types;
    while (symEnum->Next())
    {
        symbols.push_back(symEnum->getName());
        types.push_back(symEnum->getType()->getName());
    }

    EXPECT_EQ( std::vector<std::wstring>({ 
        L"func",
        L"func1",
        L"func2<2>", 
        L"testns::func3",
        L"testcls::method"
        }), symbols);

    EXPECT_EQ(std::vector<std::wstring>({
        L"Void(__cdecl)(Int4B, Char)",
        L"Void(__cdecl)()",
        L"Int4B(__cdecl)()",
        L"Char(__cdecl)()",
        kdlib::is64bitSystem() ? L"Void(__cdecl testcls::)(Int4B)" : L"Void(__thiscall testcls::)(Int4B)"
        }), types );
}

TEST_F(ClangTest, Func)
{
    static const wchar_t srcCode[] = L"      \
    void func(int, char);                    \
    inline void func1() {}                   \
    template <int> int func2() { return 0; } \
    template<> int func2<2>() { return 1; }  \
    namespace testns {                       \
        char func3(void);                    \
    }                                        \
    class testcls {                          \
      void method(int);                      \
    };                                       \
    template<typename T>                     \
    class testcls1 {                         \
       void method();                        \
    };                                       \
    template<>                               \
    class testcls1<int> {                    \
        void method();                       \
    };                                       \
    ";

    EXPECT_EQ(L"Void(__cdecl)(Int4B, Char)", compileType(srcCode, L"func")->getName());
    EXPECT_EQ(L"Void(__cdecl)()", compileType(srcCode, L"func1")->getName());
    EXPECT_EQ(L"Int4B(__cdecl)()", compileType(srcCode, L"func2<2>")->getName());
    EXPECT_EQ(L"Char(__cdecl)()", compileType(srcCode, L"testns::func3")->getName());
    if (kdlib::is64bitSystem())
    {
        EXPECT_EQ(L"Void(__cdecl testcls::)(Int4B)", compileType(srcCode, L"testcls::method")->getName());
        EXPECT_EQ(L"Void(__cdecl testcls1<int>::)()", compileType(srcCode, L"testcls1<int>::method")->getName());
    }
    else
    {
        EXPECT_EQ(L"Void(__thiscall testcls::)(Int4B)", compileType(srcCode, L"testcls::method")->getName());
        EXPECT_EQ(L"Void(__thiscall testcls1<int>::)()", compileType(srcCode, L"testcls1<int>::method")->getName());
    }

    EXPECT_THROW(compileType(srcCode, L"func2"), TypeException);
    EXPECT_THROW(compileType(srcCode, L"func3"), TypeException);
    EXPECT_THROW(compileType(srcCode, L"method"), TypeException);
    EXPECT_THROW(compileType(srcCode, L"testcls1::method"), TypeException);
    EXPECT_THROW(compileType(srcCode, L"testcls1<char>::method"), TypeException);
}
