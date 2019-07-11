#include <stdafx.h>

#include "gtest/gtest.h"
#include "kdlib/kdlib.h"

#include "test/testvars.h"

#include "procfixture.h"

using namespace kdlib;

TEST(ExprEval, Literal)
{
    EXPECT_EQ(1, evalExpr(L"1"));
    EXPECT_EQ(0, evalExpr(L"0"));
    EXPECT_EQ(0x1234, evalExpr(L"0x1234"));
    EXPECT_EQ(077, evalExpr(L"077"));

    EXPECT_EQ(2147483648, evalExpr(L"2147483648"));
    EXPECT_EQ(0x80000000, evalExpr(L"0x80000000"));
    EXPECT_EQ(2147483648L, evalExpr(L"2147483648L"));
    EXPECT_EQ(0x80000000L, evalExpr(L"0x80000000L"));
    EXPECT_EQ(2147483648U, evalExpr(L"2147483648L"));
    EXPECT_EQ(0x80000000U, evalExpr(L"0x80000000L"));
    EXPECT_EQ(2147483648UL, evalExpr(L"2147483648L"));
    EXPECT_EQ(0x80000000UL, evalExpr(L"0x80000000L"));
    
    EXPECT_EQ(9223372036854775808, evalExpr(L"9223372036854775808"));
    EXPECT_EQ(0x8000000000000000, evalExpr(L"0x8000000000000000"));
    EXPECT_EQ(9223372036854775808U, evalExpr(L"9223372036854775808"));
    EXPECT_EQ(0x8000000000000000U, evalExpr(L"0x8000000000000000"));
    EXPECT_EQ(9223372036854775808ULL, evalExpr(L"9223372036854775808ULL"));
    EXPECT_EQ(0x8000000000000000ULL, evalExpr(L"0x8000000000000000ULL"));

    EXPECT_THROW(evalExpr(L"92233720368547758080"), TypeException);
    EXPECT_THROW(evalExpr(L"0x80000000000000000"), TypeException);
}

TEST(ExprEval, FloatLiteral)
{
    EXPECT_FLOAT_EQ(1e10, (float)evalExpr("1e10"));
    EXPECT_FLOAT_EQ(1e-5L, (float)evalExpr("1e-5L"));
    EXPECT_FLOAT_EQ(1., (float)evalExpr("1."));
    EXPECT_FLOAT_EQ(.1, (float)evalExpr(".1"));
    EXPECT_FLOAT_EQ(.2f, (float)evalExpr(".2f"));
    EXPECT_FLOAT_EQ(0.2e+3f, (float)evalExpr("0.2e+3f"));
}

TEST(ExprEval, CharLiteral)
{
    EXPECT_EQ('a' , evalExpr("'a'"));
    EXPECT_EQ(L'A', evalExpr("L'A'"));
    EXPECT_EQ('\xFF', evalExpr("'\xFF'"));
    EXPECT_EQ('abc', evalExpr("'abc'"));
    EXPECT_EQ('\t', evalExpr("'\t'"));
    EXPECT_EQ('\13', evalExpr("'\13'"));
    EXPECT_EQ(L'AA', evalExpr("L'AA'"));
    EXPECT_EQ(L'AAAAA', evalExpr("L'AAAAA'"));

    EXPECT_THROW(evalExpr("''"), TypeException);
    EXPECT_THROW(evalExpr("'aaaaa'"), TypeException);
}

TEST(ExprEval, NumberExpression)
{
    EXPECT_EQ(2 + 2, evalExpr(L"2 + 2"));
    EXPECT_EQ(2 + 2 * 2, evalExpr(L"2 + 2 * 2"));
    EXPECT_EQ(2 * 2 + 2, evalExpr(L"2 * 2 + 2"));
    EXPECT_EQ(2 * 2 + 2 * 2, evalExpr(L"2 * 2 + 2 * 2 "));
    EXPECT_EQ(5 - 5 * 5  + 5, evalExpr(L"5 - 5 * 5  + 5"));
    EXPECT_EQ(5 / 5 * 5 - 5, evalExpr(L"5 / 5 * 5 - 5"));

    EXPECT_THROW(evalExpr(L"5 */ 5"), DbgException);
    EXPECT_THROW(evalExpr(L"**5"), DbgException);
    EXPECT_THROW(evalExpr(L"5/"), DbgException);
}

TEST(ExprEval, DivisionError)
{
    EXPECT_THROW(evalExpr("5/0"), DbgException);
    EXPECT_THROW(evalExpr("5/0.0"), DbgException);
}

TEST(ExprEval, UnMinus)
{
    EXPECT_EQ(-2 + 3, evalExpr(L"-2 + 3"));
    EXPECT_EQ( 2 + -3, evalExpr(L"2 + -3"));
    EXPECT_EQ(+2 - 3, evalExpr(L"+2 - 3"));
}

TEST(ExprEval, Mod)
{
    EXPECT_EQ(50 % 3, evalExpr(L"50 % 3"));
    EXPECT_EQ(4 + 0x20 % 3, evalExpr(L"4 + 0x20 % 3"));
}

TEST(ExprEval, Shift)
{
    EXPECT_EQ(2 << 3, evalExpr(L"2 << 3"));
    EXPECT_EQ(7000 >> 3, evalExpr(L"7000 >> 3"));
    EXPECT_EQ(5 + 6 << 2, evalExpr(L"5 + 6 << 2"));
    EXPECT_EQ(7 >> 3 - 1, evalExpr(L"7 >> 3 - 1"));

    EXPECT_THROW(evalExpr(L"3 >>"), DbgException);
    EXPECT_THROW(evalExpr(L">> 2"), DbgException);
}

TEST(ExprEval, Bitwise)
{
    EXPECT_EQ(0xFA01894B & 0xC58F6ACD, evalExpr(L"0xFA01894B & 0xC58F6ACD"));
    EXPECT_EQ(0xFA01894B | 0xC58F6ACD, evalExpr(L"0xFA01894B | 0xC58F6ACD"));
    EXPECT_EQ(0xFA01894B ^ 0xC58F6ACD, evalExpr(L"0xFA01894B ^ 0xC58F6ACD"));
    EXPECT_EQ(~0xFA01894B, evalExpr(L"~0xFA01894B"));
}

TEST(ExprEval, DISABLED_IncDec)
{
    TypedValue   a = 10;
    EXPECT_EQ(++a.getValue(), evalExpr(L"++a", makeScope({ {L"a", a} })));
    EXPECT_EQ(a.getValue()++, evalExpr(L"a++", makeScope({ {L"a", a} })));
    EXPECT_EQ(--a.getValue(), evalExpr(L"--a", makeScope({ {L"a", a} })));
    EXPECT_EQ(a.getValue()--, evalExpr(L"a--", makeScope({ {L"a", a} })));
}

TEST(ExprEval, ScopeVar)
{
    TypedValue   a = 10;
    TypedValue   b = -20;
    EXPECT_EQ(a + 10, evalExpr(L"a + 10", makeScope({ {L"a", a} })));
    EXPECT_EQ(a * b * 10, evalExpr(L"a * b * 10", makeScope({ { L"a", a }, {L"b", b } })));
}

TEST(ExprEval, Compare)
{
    EXPECT_TRUE((bool)evalExpr(L"-10 == -10"));
    EXPECT_FALSE((bool)evalExpr(L"-10 == 1 - 10"));
    EXPECT_TRUE((bool)evalExpr(L"5 != -1 * 5"));
    EXPECT_FALSE((bool)evalExpr(L"5 != 2 + 3"));
    EXPECT_TRUE((bool)evalExpr(L"3 + 8 < 3 * 8"));
    EXPECT_FALSE((bool)evalExpr(L"3 + 8 < -3 * 8"));
    EXPECT_TRUE((bool)evalExpr(L"3 + 8 <= 3 + 8"));
    EXPECT_TRUE((bool)(L"3 + 8 <= 3 + 9"));
    EXPECT_FALSE((bool)evalExpr(L"3 * 8 <= 3 + 9"));
    EXPECT_TRUE((bool)evalExpr(L"-10 > -20"));
    EXPECT_FALSE((bool)evalExpr(L"4 % 2 > +0"));
    EXPECT_TRUE((bool)evalExpr(L"5 >= 10 - 5"));
    EXPECT_TRUE((bool)evalExpr(L"7 >= 49 / 8"));
    EXPECT_FALSE((bool)evalExpr(L"0 >= 5%4"));
}

TEST(ExprEval, Array)
{
    long  arrayOrig[] = { 100, 200, 400, 500 };
    TypedValue  arrayVal = makeArrayValue<long>({ 100, 200, 400, 500 });
    size_t  i = 2;

    auto scope = makeScope({ { L"arrayOrig", arrayVal },{ L"i", i } });

    EXPECT_EQ(arrayOrig[3], evalExpr(L"arrayOrig[3]", scope));
    EXPECT_EQ(arrayOrig[i], evalExpr(L"arrayOrig[i]", scope));

    EXPECT_THROW(evalExpr(L"arrayOrig[]", scope), DbgException);
    EXPECT_THROW(evalExpr(L"arrayOrig[[1", scope), DbgException);
    EXPECT_THROW(evalExpr(L"arrayOrig]1[", scope), DbgException);
    EXPECT_THROW(evalExpr(L"arrayOrig[1[]", scope), DbgException);
}

TEST(ExprEval, Struct)
{
    struct TestStruct {
        int  field1;
        float  field2;
        struct {
            char  field1;
        } field3;
    }  st = { 199, 0.999f, 'a' };

    auto testStructType = compileType(L" \
        struct TestStruct { \
            int  field1; \
            float  field2; \
            struct { \
                char  field1;  \
            } field3; \
        }; ", 
        L"TestStruct");

    auto stVar = makeValue(testStructType, st);

    auto scope = makeScope({ { L"st", stVar } });

    EXPECT_EQ(st.field1, evalExpr(L"st.field1", scope));
    EXPECT_FLOAT_EQ(st.field2, (float)evalExpr(L"st.field2", scope));
    EXPECT_EQ(st.field3.field1, evalExpr(L"st.field3.field1", scope));

    EXPECT_THROW(evalExpr(L"st.", scope), DbgException);
    EXPECT_THROW(evalExpr(L"st..", scope), DbgException);
    EXPECT_THROW(evalExpr(L"st.aaaaa", scope), DbgException);
}

TEST(ExprEval, Braces)
{
    EXPECT_EQ(2 * (2 + 2), evalExpr(L"2 * (2 + 2)"));
    EXPECT_EQ(-(2 * (2 + 2)), evalExpr(L"-(2 * (2 + 2))"));
    EXPECT_EQ( ((( 2 + 2 ) / 2 ) * 4), evalExpr(L"((( 2 + 2 ) / 2 ) * 4)"));

    EXPECT_THROW(evalExpr(L"2 * (2 +"), DbgException);
    EXPECT_THROW(evalExpr(L"(2 * (2 + 2)"), DbgException);
    EXPECT_THROW(evalExpr(L"(((("), DbgException);
}

TEST(ExprEval, Sizeof)
{
    EXPECT_EQ(sizeof(int), evalExpr(L"sizeof(int)"));
    EXPECT_EQ(sizeof(int*), evalExpr(L"sizeof(int*)"));
    EXPECT_EQ(sizeof(const int), evalExpr(L"sizeof(const int)"));
    EXPECT_EQ(sizeof 1, evalExpr(L"sizeof 1"));
    EXPECT_EQ(sizeof(1), evalExpr(L"sizeof(1)"));
    EXPECT_EQ(sizeof(int[4]), evalExpr(L"sizeof(int[4])"));

    EXPECT_THROW(evalExpr(L"sizeof int"), DbgException);
}

TEST(ExprEval, SizeofComplex)
{
    EXPECT_EQ(sizeof(int*[10]), evalExpr("sizeof(int*[10])"));
    EXPECT_EQ(sizeof(int(*)[10]), evalExpr("sizeof(int(*)[10])"));
    EXPECT_EQ(sizeof(int(&)[10]), evalExpr("sizeof(int(&)[10])"));
    EXPECT_EQ(sizeof(int((*)[1])[10]), evalExpr("sizeof(int((*)[1])[10])"));

    EXPECT_THROW(evalExpr("sizeof(int([1](*))[10])"), DbgException);
    EXPECT_THROW(evalExpr("sizeof(int[10]&)"), DbgException);
}

TEST(ExprEval, SizeofExpr)
{
    EXPECT_EQ(sizeof(long int) >> 2, evalExpr("sizeof(long int) >> 2"));
    EXPECT_EQ(1 << sizeof(long int), evalExpr("1 << sizeof(long int)"));
    EXPECT_EQ(sizeof(2 < 4), evalExpr("sizeof(2 < 4)"));
}

TEST(ExprEval, Bool)
{
    EXPECT_EQ(true, evalExpr(L"true"));
    EXPECT_EQ(false, evalExpr(L"false"));
    EXPECT_EQ(sizeof(bool), evalExpr(L"sizeof(bool)"));  
    EXPECT_EQ(sizeof(true), evalExpr(L"sizeof(true)"));

    EXPECT_THROW(evalExpr(L"True"), DbgException);
}

TEST(ExprEval, BooleanOperation)
{
    EXPECT_TRUE((bool)evalExpr(L"true || false"));
    EXPECT_FALSE((bool)evalExpr(L"false || false"));
    EXPECT_TRUE((bool)evalExpr(L"true && true"));
    EXPECT_FALSE((bool)evalExpr(L"false && true"));
    EXPECT_TRUE((bool)evalExpr(L"!false"));
    EXPECT_TRUE((bool)evalExpr(L"!!true"));
    EXPECT_FALSE((bool)evalExpr(L"!true"));

    EXPECT_EQ(2 < 3 || 3 < 2, evalExpr(L"2 < 3 || 3 < 2"));
    EXPECT_EQ(2 > 3 || 3 > 2, evalExpr(L"2 > 3 || 3 > 2"));
    EXPECT_EQ(2 == 3 && 5 > 2, evalExpr(L"2 == 3 && 5 > 2"));
}

TEST(ExprEval, Ternary)
{
    EXPECT_EQ(2 < 3 ? 2 : 3, evalExpr("2 < 3 ? 2 : 3"));
    EXPECT_EQ(2 > 3 ? 2 : 3 + 2, evalExpr("2 > 3 ? 2 : 3 + 2"));
    EXPECT_EQ(true ? (2 + 3) : (2 - 3), evalExpr("true ? (2 + 3) : (2 - 3)"));
    EXPECT_EQ(false ? 7 : 8 ? 2 : 3, evalExpr("false ? 7 : 8 ? 2 : 3"));
    EXPECT_EQ((false ? 7 : 8 ) ? ( true ? 0 : 1) : 3, evalExpr("(false ? 7 : 8 ) ? ( true ? 0 : 1) : 3"));

    EXPECT_THROW(evalExpr(L"?"), DbgException);
    EXPECT_THROW(evalExpr(L"?:"), DbgException);
    EXPECT_THROW(evalExpr(L"0 ? : 5"), DbgException);
}

TEST(ExprEval, PointerArithm)
{
    EXPECT_EQ((int*)10 + 1, evalExpr("(int*)10 + 1"));
    EXPECT_EQ(1 + (int*)10, evalExpr("1 + (int*)10"));
    EXPECT_THROW(evalExpr("(int*)10 + (int*)20"), DbgException);

    EXPECT_EQ((long*)10 - 1, evalExpr("(long*)10 - 1"));
    EXPECT_EQ((long*)100 - (long*)90, evalExpr("(long*)100 - (long*)90"));
    EXPECT_THROW(evalExpr("10 - (long*)1"), DbgException);

    EXPECT_THROW(evalExpr("10 / (long*)2"), DbgException);
    EXPECT_THROW(evalExpr("(long*)10 / 2"), DbgException);

    EXPECT_THROW(evalExpr("10 % (long*)2"), DbgException);
    EXPECT_THROW(evalExpr("(long*)10 % 2"), DbgException);

    EXPECT_THROW(evalExpr("(long*)10 << 2"), DbgException);
    EXPECT_THROW(evalExpr("(long*)10 >> 2"), DbgException);
    EXPECT_THROW(evalExpr("10 << (long*)2"), DbgException);
    EXPECT_THROW(evalExpr("10 >> (long*)2"), DbgException);

    EXPECT_THROW(evalExpr("(long*)10 & 0xFF"), DbgException);
    EXPECT_THROW(evalExpr("0xFF & (long*)10"), DbgException);
    EXPECT_THROW(evalExpr("(long*)10 | 0xFF"), DbgException);
    EXPECT_THROW(evalExpr("0xFF | (long*)10"), DbgException);
    EXPECT_THROW(evalExpr("(long*)10 ^ 0xFF"), DbgException);
    EXPECT_THROW(evalExpr("0xFF ^ (long*)10"), DbgException);

    EXPECT_THROW(evalExpr("!(long*)10"), DbgException);
    EXPECT_THROW(evalExpr("~(long*)10"), DbgException);
    EXPECT_THROW(evalExpr("-(long*)10"), DbgException);
}

TEST(ExprEval, NullPtr)
{
    EXPECT_EQ(0, evalExpr("(void*)0"));
    EXPECT_EQ(0, evalExpr("(int**)nullptr"));
}

class ExprEvalTarget : public ProcessFixture
{
public:

    ExprEvalTarget() : ProcessFixture(L"typetest") {}

};

TEST_F(ExprEvalTarget, StructPtr)
{
    EXPECT_EQ(g_structTestPtr->m_field0, evalExpr(L"g_structTestPtr->m_field0", m_targetModule->getScope()));
}

TEST_F(ExprEvalTarget, Deref)
{
    EXPECT_EQ(*pbigValue, evalExpr(L"*pbigValue", m_targetModule->getScope()));
    EXPECT_EQ(**ppbigValue, evalExpr(L"**ppbigValue", m_targetModule->getScope()));
    EXPECT_EQ( (*g_structTestPtr).m_field0, evalExpr(L"(*g_structTestPtr).m_field0", m_targetModule->getScope()));  

    EXPECT_THROW(evalExpr(L"*2"), DbgException);
    EXPECT_THROW(evalExpr(L"*bigValue"), DbgException);
}

TEST_F(ExprEvalTarget, Ref)
{
    EXPECT_EQ((&g_classChild)->m_childField, evalExpr("(&g_classChild)->m_childField", m_targetModule->getScope()));
    EXPECT_EQ(*(&g_classChild.m_childField), evalExpr("*(&g_classChild.m_childField)", m_targetModule->getScope()));

    EXPECT_THROW(evalExpr("(&&g_classChild)->m_childField"), DbgException);
    EXPECT_THROW(evalExpr("(g_classChild&)->m_childField"), DbgException);
}


TEST_F(ExprEvalTarget, CastBase)
{
    EXPECT_EQ((__int64)charVar, evalExpr("(__int64)charVar", m_targetModule->getScope()));
    EXPECT_DOUBLE_EQ((double)charVar, (double)evalExpr("(double)charVar", m_targetModule->getScope()));
    EXPECT_EQ(loadTypedVar(L"charVar")->getAddress(), evalExpr("(__int64)&charVar", m_targetModule->getScope()));
    EXPECT_EQ(loadTypedVar(L"g_testArray")->getAddress(), evalExpr("(long long)g_testArray", m_targetModule->getScope()));
    EXPECT_EQ( (short)enumType::THREE, evalExpr("(short)enumType::THREE", m_targetModule->getScope()));

    EXPECT_THROW(evalExpr("(int)g_classChild", m_targetModule->getScope()), DbgException);
}

TEST_F(ExprEvalTarget, CastPtr)
{
    EXPECT_EQ(*(unsigned char*)pbigValue, evalExpr("*(unsigned char*)pbigValue", m_targetModule->getScope()));
    EXPECT_EQ(*(char*)pbigValue, evalExpr("*(char*)pbigValue", m_targetModule->getScope()));
    EXPECT_EQ(*(unsigned char*)(__int64)pbigValue, evalExpr("*(unsigned char*)(__int64)pbigValue", m_targetModule->getScope()));
    EXPECT_EQ(*(long*)shortArray, evalExpr("*(long*)shortArray", m_targetModule->getScope()));
}

TEST_F(ExprEvalTarget, CastArray)
{
    EXPECT_THROW(evalExpr("((char[8])pbigValue)[3]", m_targetModule->getScope()), DbgException);
    EXPECT_THROW(evalExpr("((char[])pbigValue)[3]", m_targetModule->getScope()), DbgException);
}

TEST_F(ExprEvalTarget, CastRef)
{   
    EXPECT_EQ((int&)longlongVar, evalExpr("(int&)longlongVar", m_targetModule->getScope()));
    EXPECT_EQ((const int&)longlongVar, evalExpr("(const int&)longlongVar", m_targetModule->getScope()));
}

TEST_F(ExprEvalTarget, CastUdt)
{
    EXPECT_THROW(evalExpr("(int)g_classChild", m_targetModule->getScope()), DbgException);
    EXPECT_THROW(evalExpr("(int*)g_classChild", m_targetModule->getScope()), DbgException);
    EXPECT_THROW(evalExpr("(structTest)g_classChild", m_targetModule->getScope()), DbgException);
}

TEST_F(ExprEvalTarget, EnumVal)
{
    EXPECT_EQ(enumType::THREE, evalExpr("enumType::THREE", m_targetModule->getScope()));
}

TEST_F(ExprEvalTarget, StaticField)
{
    EXPECT_EQ(classChild::m_staticField, evalExpr("classChild::m_staticField", m_targetModule->getScope()));
}

TEST_F(ExprEvalTarget, DISABLED_ConstField)
{
    EXPECT_EQ(classChild::m_staticConst, evalExpr("classChild::m_staticConst", m_targetModule->getScope()));
}

TEST_F(ExprEvalTarget, NamespaceConst)
{
    EXPECT_EQ(testspace::constInt, evalExpr("testspace::constInt", m_targetModule->getScope()));   
}

TEST_F(ExprEvalTarget, DISABLED_NamespaceConstField)
{
    EXPECT_EQ(testspace::testClass1::m_constLong, evalExpr("testspace::testClass1::m_constLong", m_targetModule->getScope()));
}

TEST_F(ExprEvalTarget, StdList)
{
    EXPECT_EQ(sizeof(std::list<int>), evalExpr("sizeof(std::list<int,std::allocator<int> >)", m_targetModule->getScope())); 
}

TEST_F(ExprEvalTarget, StructTemplate)
{
    EXPECT_EQ(sizeof(TestStructTemplate<int>), evalExpr("sizeof(TestStructTemplate<int>)", m_targetModule->getScope()));
}

TEST_F(ExprEvalTarget, ClassTemplate)
{
    EXPECT_EQ(sizeof(TestClassTemplate<int>), evalExpr("sizeof(TestClassTemplate<int>)", m_targetModule->getScope()));
}

TEST_F(ExprEvalTarget, ClassTemplateUnusedType)
{
    EXPECT_EQ(sizeof(ClassNoField<UnusedStruct>), evalExpr("sizeof(ClassNoField<UnusedStruct>)", m_targetModule->getScope()));
}

TEST_F(ExprEvalTarget, Eval1)
{
    EXPECT_EQ(g_testArray[1].m_field1, evalExpr("g_testArray[1].m_field1", m_targetModule->getScope()));
    EXPECT_EQ(g_structTest1.m_field4->m_field1 + 200, evalExpr("g_structTest1.m_field4->m_field1 + 200", m_targetModule->getScope()));
    EXPECT_EQ( (g_testArray + 1)->m_field1 % 4, evalExpr("(g_testArray + 1)->m_field1 % 4", m_targetModule->getScope()));
}

