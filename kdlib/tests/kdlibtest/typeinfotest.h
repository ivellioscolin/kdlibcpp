#pragma once

#include "processtest.h"

#include "kdlib/typeinfo.h"
#include "kdlib/exceptions.h"
#include "kdlib/memaccess.h"

#include "test/testvars.h"

using namespace kdlib;


class TypeInfoTest : public ProcessTest 
{
public:

    TypeInfoTest() : ProcessTest( L"typetest" ) {}

};

TEST_F( TypeInfoTest, Ctor )
{
    EXPECT_EQ( L"structTest", loadType( L"structTest" )->getName() );
    EXPECT_EQ( L"structTest", loadType( L"targetapp!structTest" )->getName() );
    EXPECT_EQ( L"structTest", loadType( L"g_structTest" )->getName() );
    EXPECT_EQ( L"structTest", loadType( L"targetapp!g_structTest" )->getName() );

    EXPECT_THROW( loadType( L"nonExistingSymbol" ), DbgException );
}

TEST_F( TypeInfoTest, BaseTypeNames )
{
    EXPECT_EQ( L"Int1B", loadType( L"Int1B" )->getName() );
    EXPECT_EQ( L"Int2B", loadType( L"Int2B" )->getName() );
    EXPECT_EQ( L"Int4B", loadType( L"Int4B" )->getName() );
    EXPECT_EQ( L"Int8B", loadType( L"Int8B" )->getName() );
    EXPECT_EQ( L"UInt1B", loadType( L"UInt1B" )->getName() );
    EXPECT_EQ( L"UInt2B", loadType( L"UInt2B" )->getName() );
    EXPECT_EQ( L"UInt4B", loadType( L"UInt4B" )->getName() );
    EXPECT_EQ( L"UInt8B", loadType( L"UInt8B" )->getName() );

    EXPECT_EQ( L"Long", loadType( L"Long" )->getName() );
    EXPECT_EQ( L"ULong", loadType( L"ULong" )->getName() );
    EXPECT_EQ( L"Char", loadType( L"Char" )->getName() );
    EXPECT_EQ( L"WChar", loadType( L"WChar")->getName() );
    EXPECT_EQ( L"Bool", loadType( L"Bool")->getName() );
    EXPECT_EQ( L"Float", loadType( L"Float")->getName() );
    EXPECT_EQ( L"Double", loadType( L"Double")->getName() );
}

TEST_F( TypeInfoTest, BaseTypeSizes)
{
    EXPECT_EQ( 1, loadType( L"Int1B" )->getSize() );
    EXPECT_EQ( 2, loadType( L"Int2B" )->getSize() );
    EXPECT_EQ( 4, loadType( L"Int4B" )->getSize() );
    EXPECT_EQ( 8, loadType( L"Int8B" )->getSize() );
    EXPECT_EQ( 1, loadType( L"UInt1B" )->getSize() );
    EXPECT_EQ( 2, loadType( L"UInt2B" )->getSize() );
    EXPECT_EQ( 4, loadType( L"UInt4B" )->getSize() );
    EXPECT_EQ( 8, loadType( L"UInt8B" )->getSize() );

    EXPECT_EQ( 4, loadType( L"Long" )->getSize() );
    EXPECT_EQ( 4, loadType( L"ULong" )->getSize() );
    EXPECT_EQ( 1, loadType( L"Char" )->getSize() );
    EXPECT_EQ( 2, loadType( L"WChar")->getSize() );
    EXPECT_EQ( 1, loadType( L"Bool")->getSize() );
    EXPECT_EQ( 4, loadType( L"Float")->getSize() );
    EXPECT_EQ( 8, loadType( L"Double")->getSize() );
}

TEST_F( TypeInfoTest, BaseTypeVars)
{
    EXPECT_EQ( L"Char", loadType( L"charVar" )->getName() );
    EXPECT_EQ( L"WChar", loadType( L"wcharVar" )->getName() );
    EXPECT_EQ( L"Int2B", loadType( L"shortVar" )->getName() );
    EXPECT_EQ( L"Int4B", loadType( L"longVar" )->getName() );
    EXPECT_EQ( L"Int8B", loadType( L"longlongVar" )->getName() );
    EXPECT_EQ( L"UInt1B", loadType( L"ucharVar" )->getName() );
    EXPECT_EQ( L"UInt2B", loadType( L"ushortVar" )->getName() );
    EXPECT_EQ( L"UInt4B", loadType( L"ulongVar" )->getName() );
    EXPECT_EQ( L"UInt8B", loadType( L"ulonglongVar" )->getName() );
}

TEST_F( TypeInfoTest, BaseTypeArray )
{
    EXPECT_EQ( L"Int2B[2]", loadType( L"Int2B[2]" )->getName() );
    EXPECT_EQ( 2*2, loadType( L"Int2B[2]" )->getSize() );
    EXPECT_EQ( L"Int4B[2][3]", loadType( L"Int4B[2][3]" )->getName() );
    EXPECT_EQ( 4*2*3, loadType( L"Int4B[2][3]" )->getSize() );

    EXPECT_EQ( L"Int4B[2][3]", loadType(L"intMatrix")->getName() );
    EXPECT_EQ( sizeof(intMatrix), loadType(L"intMatrix")->getSize() ); 
        
    EXPECT_THROW( loadType( L"Int2B[1" ), TypeException );
    EXPECT_THROW( loadType( L"Int2B[1[1]]" ), TypeException );
    EXPECT_THROW( loadType( L"Int2B[-1]" ), TypeException );
}

TEST_F( TypeInfoTest, BaseTypePointer )
{
    EXPECT_EQ( L"UInt1B*", loadType( L"UInt1B*" )->getName() );
    EXPECT_EQ( ptrSize(), loadType( L"UInt1B*" )->getSize() );
    EXPECT_EQ( L"UInt1B***", loadType( L"UInt1B***" )->getName() );
    EXPECT_EQ( ptrSize(), loadType( L"UInt1B***" )->getSize() );

    EXPECT_EQ( L"UInt8B*", loadType( L"pbigValue")->getName() );
    EXPECT_EQ( ptrSize(), loadType( L"pbigValue")->getSize() );
    EXPECT_EQ( L"UInt8B**", loadType( L"ppbigValue")->getName() );
    EXPECT_EQ( ptrSize(), loadType( L"ppbigValue")->getSize() );

    EXPECT_EQ( L"UInt1B*", loadType( L"UInt1B(*)" )->getName() );
    EXPECT_THROW( loadType( L"Int2B(*" ), TypeException );
    EXPECT_THROW( loadType( L"Int2B*)" ), TypeException );
}

TEST_F( TypeInfoTest, Void )
{
    EXPECT_EQ( L"Void*", loadType( L"Void*" )->getName() );
    EXPECT_EQ( L"Void**", loadType( L"Void**" )->getName() );
    EXPECT_EQ( L"Void*", loadType( L"voidPtr" )->getName() );
}

TEST_F( TypeInfoTest, UdtGetField )
{
    TypeInfoPtr  structType;
    
    structType = loadType( L"structTest" );
    EXPECT_EQ( L"UInt4B", structType->getElement(L"m_field0")->getName() );

    structType = loadType( L"structWithNested" );
    EXPECT_NO_THROW( structType->getElement(L"m_field")->getName() );
    EXPECT_THROW( structType->getElement(L"m_nestedFiled")->getName(), TypeException );
    EXPECT_NO_THROW( structType->getElement(L"m_field3")->getName() );
    EXPECT_NO_THROW( structType->getElement(L"m_unnameStruct.m_field2")->getName() );
}

TEST_F( TypeInfoTest, Const )
{
    EXPECT_EQ( L"Bool", loadType( L"g_constBoolValue")->getName() );   
    EXPECT_TRUE( loadType( L"g_constNumValue")->isConstant() );
    EXPECT_EQ( 0x5555, *loadType( L"g_constNumValue") );
    EXPECT_THROW( ulongVar == (NumVariant)*loadType( L"ulongVar"), TypeException );
}

TEST_F( TypeInfoTest, Array )
{
    TypeInfoPtr  array1;
    TypeInfoPtr  array2;
    
    EXPECT_NO_THROW( array1 = loadType( L"ulonglongArray" ) );
    EXPECT_NO_THROW( array2 = loadType( L"arrIntMatrixPtrs" ) );
    
    EXPECT_EQ( sizeof(ulonglongArray), array1->getSize() );
    EXPECT_EQ( sizeof(arrIntMatrixPtrs), array2->getSize() );

    EXPECT_EQ( TEST_ARRAY_SIZE, array1->getElementCount() );
    EXPECT_EQ( 4, array2->getElementCount() );
}

TEST_F( TypeInfoTest, CppRef )
{
    TypeInfoPtr  ref1;
    TypeInfoPtr  ref2;
    
    EXPECT_NO_THROW( ref1 = loadType( L"ref_bigValue" ) );
    EXPECT_NO_THROW( ref2 = loadType( L"ref_pbigValue" ) );

    EXPECT_TRUE( ref1->isPointer() && ref2->isPointer() );
}

TEST_F( TypeInfoTest, Typedef )
{
    EXPECT_EQ( L"structTest", loadType( L"g_structTypeDef" )->getName() );
    EXPECT_EQ( L"structTest", loadType( L"structTestTypeDef" )->getName() );
}

TEST_F( TypeInfoTest, Enum )
{
    EXPECT_NO_THROW( loadType(L"enumType") );
    EXPECT_TRUE( loadType(L"enumType")->isEnum() );
    EXPECT_EQ( TWO, *loadType(L"enumType")->getElement(L"TWO") );
    EXPECT_TRUE( (NumVariant)*loadType(L"enumType")->getElement(L"TWO") == TWO );
}

TEST_F( TypeInfoTest, BitField )
{    
    TypeInfoPtr bitFieldStruct;
    TypeInfoPtr bitField;

    EXPECT_NO_THROW( bitFieldStruct = loadType(L"g_structWithBits") );
    EXPECT_NO_THROW( bitField = bitFieldStruct->getElement(L"m_bit5") );
    EXPECT_TRUE( bitField->isBitField() );
    EXPECT_EQ( 5, bitField->getBitOffset() );
    EXPECT_EQ( 1, bitField->getBitWidth() );
    EXPECT_EQ( 3, bitFieldStruct->getElement(L"m_bit6_8")->getBitWidth() );
}

TEST_F( TypeInfoTest, StaticField )
{
    TypeInfoPtr  typeInfo;
    TypeInfoPtr  fieldType;

    EXPECT_NO_THROW( typeInfo = loadType( L"g_classChild" ) );
    EXPECT_NO_THROW( fieldType = typeInfo->getElement(L"m_staticField") );

    EXPECT_THROW( typeInfo->getElementOffset(L"m_staticField"), TypeException );
    EXPECT_EQ( classChild::m_staticField,ptrSignDWord( typeInfo->getElementVa(L"m_staticField") ) );
}

TEST_F(  TypeInfoTest, StaticConstField )
{
    TypeInfoPtr  typeInfo;
    EXPECT_NO_THROW( typeInfo = loadType( L"g_classChild" ) );
    EXPECT_EQ( classChild::m_staticConst, *typeInfo->getElement(L"m_staticConst") );
}


TEST_F( TypeInfoTest, GetElement )
{
    EXPECT_THROW( loadType(L"Int1B")->getElement(L"nofiled"), TypeException );
    EXPECT_EQ( L"UInt4B", loadType(L"structTest")->getElement(L"m_field0")->getName() );
    EXPECT_EQ( L"Int4B", loadType(L"structWithNested")->getElement(L"m_unnameStruct.m_field2")->getName() );
    EXPECT_THROW( loadType(L"structWithNested")->getElement(L"m_nestedFiled"), TypeException);
}

TEST_F( TypeInfoTest, GetElementOffset )
{
    EXPECT_THROW( loadType(L"Int1B")->getElementOffset(L"nofiled"), TypeException );
    EXPECT_EQ( FIELD_OFFSET( structTest, m_field3 ), loadType(L"structTest")->getElementOffset(L"m_field3") );
    EXPECT_EQ( FIELD_OFFSET( structWithNested, m_unnameStruct.m_field2 ), loadType(L"structWithNested")->getElementOffset(L"m_unnameStruct.m_field2") );
}

TEST_F( TypeInfoTest, GetPtrSize )
{
    EXPECT_EQ( sizeof( &charVar ), loadType( L"charVar" )->getPtrSize() );
    EXPECT_EQ( sizeof( g_structTestPtr ), loadType( L"g_structTestPtr" )->getPtrSize() );
    EXPECT_EQ( sizeof( &g_testArray ), loadType( L"g_testArray" )->getPtrSize() );
}

TEST_F( TypeInfoTest, GetElementCount )
{
    EXPECT_THROW( loadType( L"UInt1B" )->getElementCount(), TypeException );
    EXPECT_THROW( loadType( L"UInt1B*" )->getElementCount(), TypeException );
    EXPECT_EQ( 20, loadType( L"UInt1B[20]" )->getElementCount() );
    EXPECT_EQ( 5, loadType( L"g_structTest" )->getElementCount() );
    EXPECT_EQ( sizeof(g_testArray)/sizeof(g_testArray[0]), loadType( L"g_testArray" )->getElementCount() );
    EXPECT_EQ( 3, loadType( L"enumType" )->getElementCount() );
}

