#pragma once

#include "procfixture.h"
#include "kdlib/heap.h"



class NetTest : public NetProcessFixture
{
};


TEST_F(NetTest, ProcessIsManaged)
{
    EXPECT_TRUE(TargetProcess::getCurrent()->isManaged());
}

TEST_F(NetTest, NetModuleIsManaged)
{
    EXPECT_TRUE(m_targetModule->isManaged());
    EXPECT_FALSE( loadModule(L"ntdll")->isManaged());
}


TEST_F(NetTest, NetHeap)
{
    kdlib::TargetHeapPtr  targetHeap;
    ASSERT_NO_THROW(targetHeap = TargetProcess::getCurrent()->getManagedHeap());
    EXPECT_LT(0, targetHeap->getCount() );
    EXPECT_LT(0, targetHeap->getCount(L"managedapp*") );

    kdlib::TargetHeapEnumPtr  heapEnum;
    ASSERT_NO_THROW(heapEnum = targetHeap->getEnum(L"managedapp*"));
    
    std::wstring  typeName;
    size_t  size;
    MEMOFFSET_64  address;
    EXPECT_TRUE( heapEnum->next(address, typeName, size) );
}

//TEST_F(NetTest, NetModuleEnumTypes)
//{
//    EXPECT_EQ( 
//        TypeNameList( { L"managedapp.Class1",
//        L"managedapp.Class1.Nested",
//        L"managedapp.Program" } ),
//        m_targetModule->enumTypes() );
//
//    EXPECT_EQ( 
//        TypeNameList( { L"managedapp.Class1",
//        L"managedapp.Class1.Nested" } ),
//        m_targetModule->enumTypes(L"*Class1*") );
//
//}
//
//TEST_F(NetTest, ClassField)
//{
//    TypeInfoPtr  classBase;
//
//    ASSERT_NO_THROW( classBase = m_targetModule->getTypeByName(L"managedapp.TestClass") );
//
//    std::wstring  desc;
//    EXPECT_NO_THROW( desc = classBase->str() );
//}


template <typename T>
class NetTestObj : public T
{
protected:

      virtual void SetUp() 
      {
            T::SetUp();

            kdlib::TargetHeapPtr  targetHeap;
            ASSERT_NO_THROW(targetHeap = TargetProcess::getCurrent()->getManagedHeap());

            kdlib::TargetHeapEnumPtr  heapEnum;
            ASSERT_NO_THROW(heapEnum = targetHeap->getEnum(L"managedapp.TestClass"));

            std::wstring  typeName;
            size_t  size;
            MEMOFFSET_64  address;
            ASSERT_TRUE( heapEnum->next(address, typeName, size) );
            
            ASSERT_NO_THROW( m_testClassVar = TargetProcess::getCurrent()->getManagedVar(address) );
      }

      virtual void TearDown()
      {
          T::TearDown();
      }

      kdlib::TypedVarPtr   m_testClassVar;
};

typedef ::testing::Types<NetProcessFixture, NetDumpFixture>  NetTargetTypes;

TYPED_TEST_CASE(NetTestObj, NetTargetTypes);

TYPED_TEST(NetTestObj, BaseField)
{
    EXPECT_EQ( 'a', *m_testClassVar->getElement(L"charField") );
    EXPECT_EQ( 3456, *m_testClassVar->getElement(L"shortField") );
}

TYPED_TEST(NetTestObj, HeapVarArray)
{
    EXPECT_EQ( 4, m_testClassVar->getElement(L"intArray")->getElementCount() );
    EXPECT_EQ (8888, *m_testClassVar->getElement(L"intArray")->getElement(3) );
}


TYPED_TEST(NetTestObj, HeapVarMultiSizeArray)
{
    EXPECT_EQ( 2, m_testClassVar->getElement(L"floatArray")->getElementCount() );
    EXPECT_FLOAT_EQ(0.3f, *m_testClassVar->getElement(L"floatArray")->getElement(0)->getElement(1)->getElement(0) );
}

TYPED_TEST(NetTestObj, HeapVarString)
{
    EXPECT_EQ(L"Hello", m_testClassVar->getElement(L"strField")->getStrValue());
}

TYPED_TEST(NetTestObj, HeapVarClass)
{
    EXPECT_EQ(-555, *m_testClassVar->getElement(L"class1Field")->getElement(L"Field1"));
}

TYPED_TEST(NetTestObj, HeapFieldEnum)
{
    EXPECT_EQ(4, *m_testClassVar->getElement(L"daysField"));
}

TYPED_TEST(NetTestObj, Str)
{
    std::wstring  str;
    EXPECT_NO_THROW(str = m_testClassVar->str() );
}

TYPED_TEST(NetTestObj, InheritedField)
{
    EXPECT_EQ( 0xAABBCCDD, *m_testClassVar->getElement(L"longField") );
}

TYPED_TEST(NetTestObj, StaticField)
{
    EXPECT_EQ(L"staticField",  m_testClassVar->getElement(L"staticStrField")->getStrValue() );
}
