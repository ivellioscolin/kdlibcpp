#pragma once

#include "processtest.h"

#include "kdlib/typedvar.h"
#include "kdlib/exceptions.h"

#include "test/testvars.h"

using namespace kdlib;

class TypedVarTest : public ProcessTest 
{
public:

    TypedVarTest() : ProcessTest( L"typedvartest" ) {}
};


TEST_F( TypedVarTest, Ctor )
{
    EXPECT_NO_THROW( loadTypedVar(L"ucharVar") );
    EXPECT_NO_THROW( loadTypedVar(L"UInt1B", m_targetModule->getSymbolVa(L"ucharVar") ) );
    EXPECT_NO_THROW( loadTypedVar( loadType(L"UInt1B"),  m_targetModule->getSymbolVa(L"ucharVar") ) );

    EXPECT_THROW( loadTypedVar( TypeInfoPtr(),  m_targetModule->getSymbolVa(L"ucharVar") ), DbgException );
}

TEST_F( TypedVarTest, BaseTypeVars )
{
    EXPECT_EQ( ucharVar, *loadTypedVar(L"ucharVar") );
    EXPECT_EQ( ushortVar, *loadTypedVar(L"ushortVar") );
    EXPECT_EQ( ulongVar, *loadTypedVar(L"ulongVar") );
    EXPECT_EQ( ulonglongVar, *loadTypedVar(L"ulonglongVar") );
    EXPECT_EQ( charVar, *loadTypedVar(L"charVar") );
    EXPECT_EQ( wcharVar, *loadTypedVar(L"wcharVar") );
    EXPECT_EQ( shortVar, *loadTypedVar(L"shortVar") );
    EXPECT_EQ( longVar, *loadTypedVar(L"longVar") );
    EXPECT_EQ( ucharVar, *loadTypedVar(L"ucharVar") );
    EXPECT_EQ( longlongVar, *loadTypedVar(L"longlongVar") );
    EXPECT_EQ( floatVar, *loadTypedVar(L"floatVar") );
    EXPECT_EQ( doubleVar, *loadTypedVar(L"doubleVar") );
}

TEST_F( TypedVarTest, TypedVarPtr )
{
    TypedVarPtr     ptr;
    MEMOFFSET_64    ptrAddr = m_targetModule->getSymbolVa( L"g_structTestPtr" );

    EXPECT_NO_THROW( ptr = loadTypedVar(L"g_structTestPtr") );
    EXPECT_EQ( ptrPtr( ptrAddr ), *ptr );
    EXPECT_EQ( ptrAddr, ptr->getAddress() );

}

TEST_F( TypedVarTest, TypedVarArray )
{
    TypedVarPtr  var;
    MEMOFFSET_64  varAddr = m_targetModule->getSymbolVa( L"g_testArray" );

    EXPECT_NO_THROW( var = loadTypedVar(L"g_testArray") );
    EXPECT_EQ( ptrPtr(varAddr), *var);
    EXPECT_EQ( varAddr, var->getAddress() );
}

TEST_F( TypedVarTest, BitFields )
{
    TypedVarPtr  typedVar;

    EXPECT_NO_THROW( typedVar = loadTypedVar(L"g_structWithBits") );
    EXPECT_EQ( g_structWithBits.m_bit0_4, *typedVar->getElement(L"m_bit0_4") );
    EXPECT_EQ( g_structWithBits.m_bit5, *typedVar->getElement(L"m_bit5") );
    EXPECT_EQ( g_structWithBits.m_bit6_8, *typedVar->getElement(L"m_bit6_8") );

    EXPECT_NO_THROW( typedVar = loadTypedVar(L"g_structWithSignBits") );
    EXPECT_EQ( g_structWithSignBits.m_bit5, *typedVar->getElement(L"m_bit5") );
    EXPECT_EQ( g_structWithSignBits.m_bit6_8, *typedVar->getElement(L"m_bit6_8") );
}

TEST_F( TypedVarTest, GetAddress )
{
    EXPECT_EQ( loadTypedVar(L"ucharVar")->getAddress(), m_targetModule->getSymbolVa(L"ucharVar") );
    EXPECT_EQ( loadTypedVar(L"g_structTest")->getAddress(), m_targetModule->getSymbolVa(L"g_structTest") );
    EXPECT_EQ( loadTypedVar(L"g_structTestPtr")->getAddress(), m_targetModule->getSymbolVa(L"g_structTestPtr") );
}

TEST_F( TypedVarTest, GetSize )
{
    EXPECT_EQ( sizeof(ucharVar), loadTypedVar(L"ucharVar")->getSize() );
    EXPECT_EQ( sizeof(g_structTest), loadTypedVar(L"g_structTest")->getSize() );
    EXPECT_EQ( sizeof(pbigValue), loadTypedVar(L"pbigValue")->getSize() );
    EXPECT_EQ( sizeof(ulonglongArray), loadTypedVar(L"ulonglongArray")->getSize() );
}

TEST_F( TypedVarTest, GetField )
{
    EXPECT_THROW( loadTypedVar(L"ucharVar")->getElement(L"nofield"), TypeException );
    EXPECT_THROW( loadTypedVar(L"g_structTest")->getElement(L"nofield"), TypeException );
    EXPECT_EQ( g_structTest.m_field1, *loadTypedVar(L"g_structTest")->getElement(L"m_field1") );
    EXPECT_EQ( g_structWithNested.m_unnameStruct.m_field2, *loadTypedVar(L"g_structWithNested")->getElement(L"m_unnameStruct.m_field2") );
}

TEST_F( TypedVarTest, GetElementCount )
{
    EXPECT_THROW( loadTypedVar(L"ucharVar")->getElementCount(), TypeException );
    EXPECT_THROW( loadTypedVar(L"g_structTestPtr")->getElementCount(), TypeException );
    EXPECT_EQ( 5, loadTypedVar(L"g_structTest")->getElementCount() );
    EXPECT_EQ( ARRAYSIZE(g_testArray), loadTypedVar(L"g_testArray")->getElementCount() );
}

TEST_F( TypedVarTest, GetElementByIndex )
{
    EXPECT_THROW( loadTypedVar(L"ucharVar")->getElement(0), TypeException );
    EXPECT_THROW( loadTypedVar(L"g_structTestPtr")->getElement(0), TypeException );
    EXPECT_EQ( g_structTest.m_field1, *loadTypedVar(L"g_structTest")->getElement(1) );
    EXPECT_THROW( loadTypedVar(L"g_structTest")->getElement(10), TypeException );
    EXPECT_EQ( g_testArray[0].m_field1, *loadTypedVar(L"g_testArray")->getElement(0)->getElement(1) );
    EXPECT_THROW( loadTypedVar(L"g_testArray")->getElement( ARRAYSIZE(g_testArray) ), TypeException  );
}


TEST_F( TypedVarTest, ArithmOp )
{
    EXPECT_EQ( ucharVar + 20, *loadTypedVar(L"ucharVar") + 20);
}

TEST_F( TypedVarTest, Enum )
{
    TypedVarPtr  enumVar;
    ASSERT_NO_THROW( enumVar = loadTypedVar(L"g_constEnumThree") );
    EXPECT_EQ( g_constEnumThree, *enumVar );
}

TEST_F( TypedVarTest, getType )
{
    TypedVarPtr  var;

    ASSERT_NO_THROW( var = loadTypedVar(L"g_constEnumThree") );
    EXPECT_EQ( L"enumType", var->getType()->getName() );

    ASSERT_NO_THROW( var = loadTypedVar(L"strArray") );
    EXPECT_EQ( L"Char*[2]", var->getType()->getName() );
}



