#pragma once

#include "procfixture.h"
#include "kdlib/memaccess.h"
#include "kdlib/exceptions.h"
#include "test/testvars.h"

using namespace kdlib;

class MemoryTest : public ProcessFixture 
{
public:

    MemoryTest() : ProcessFixture( L"memtest" ) {}
};

TEST_F( MemoryTest, ReadMemory )
{
    std::string  _helloStr(helloStr);
    size_t size = _helloStr.size()*sizeof(std::string::value_type);

    std::vector<char>  buffer(size);
    EXPECT_NO_THROW( readMemory( m_targetModule->getSymbolVa(L"helloStr"), &buffer[0], size) );
    EXPECT_TRUE( std::equal( buffer.begin(), buffer.end(), _helloStr.begin() ) );

    EXPECT_THROW( readMemory( 0, &buffer[0],1), MemoryException );

    EXPECT_FALSE( readMemoryUnsafe( 0, &buffer[0],1) );
}

TEST_F( MemoryTest, CompareMemory )
{
    std::string  _helloStr(helloStr);
    size_t size = _helloStr.size()*sizeof(std::string::value_type);
    EXPECT_TRUE( compareMemory( m_targetModule->getSymbolVa(L"helloStr"), m_targetModule->getSymbolVa(L"helloStr1"), size) );
    EXPECT_TRUE( compareMemory( m_targetModule->getSymbolVa(L"helloStr"), m_targetModule->getSymbolVa(L"helloStr"), size) );
    EXPECT_FALSE( compareMemory( m_targetModule->getSymbolVa(L"helloStr"), m_targetModule->getSymbolVa(L"helloWStr"), size ) );
    EXPECT_THROW( compareMemory( m_targetModule->getSymbolVa(L"helloStr"), 0, 1 ), MemoryException );
}

TEST_F( MemoryTest, LoadBytes )
{
    std::vector<unsigned char>  _ucharArray = loadBytes( m_targetModule->getSymbolVa(L"ucharArray"), TEST_ARRAY_SIZE);
    EXPECT_TRUE( std::equal( &ucharArray[0], &ucharArray[TEST_ARRAY_SIZE-1], _ucharArray.begin() ) );
}

TEST_F( MemoryTest, LoadWords )
{
    std::vector<unsigned short>  _ushortArray = loadWords( m_targetModule->getSymbolVa(L"ushortArray"), TEST_ARRAY_SIZE);
    EXPECT_TRUE( std::equal( &ushortArray[0], &ushortArray[TEST_ARRAY_SIZE-1], _ushortArray.begin() ) ); 
}

TEST_F( MemoryTest, LoadDWords )
{
    std::vector<unsigned long>  _ulongArray = loadDWords( m_targetModule->getSymbolVa(L"ulongArray"), TEST_ARRAY_SIZE);
    EXPECT_TRUE( std::equal( &ulongArray[0], &ulongArray[TEST_ARRAY_SIZE-1], _ulongArray.begin() ) ); 
}

TEST_F( MemoryTest, LoadQWords )
{
    std::vector<unsigned long long>  _ulonglongArray = loadQWords( m_targetModule->getSymbolVa(L"ulonglongArray"), TEST_ARRAY_SIZE);
    EXPECT_TRUE( std::equal( &ulonglongArray[0], &ulonglongArray[TEST_ARRAY_SIZE-1], _ulonglongArray.begin() ) );
}

TEST_F( MemoryTest, LoadSignBytes )
{
    std::vector<char> _charArray = loadSignBytes( m_targetModule->getSymbolVa(L"charArray"), TEST_ARRAY_SIZE);
    EXPECT_TRUE( std::equal( &charArray[0], &charArray[TEST_ARRAY_SIZE-1], _charArray.begin() ) );
}

TEST_F( MemoryTest, LoadSignWords )
{
    std::vector<short> _shortArray = loadSignWords( m_targetModule->getSymbolVa(L"shortArray"), TEST_ARRAY_SIZE);
    EXPECT_TRUE( std::equal( &shortArray[0], &shortArray[TEST_ARRAY_SIZE-1], _shortArray.begin() ) );
}

TEST_F( MemoryTest, LoadSignDWords )
{
    std::vector<long> _longArray = loadSignDWords( m_targetModule->getSymbolVa(L"longArray"), TEST_ARRAY_SIZE);
    EXPECT_TRUE( std::equal( &longArray[0], &longArray[TEST_ARRAY_SIZE-1], _longArray.begin() ) );
}

TEST_F( MemoryTest, LoadSignQWords )
{
    std::vector<long long> _longlongArray = loadSignQWords( m_targetModule->getSymbolVa(L"longlongArray"), TEST_ARRAY_SIZE);
    EXPECT_TRUE( std::equal( &longlongArray[0], &longlongArray[TEST_ARRAY_SIZE-1], _longlongArray.begin() ) );
}

TEST_F( MemoryTest, LoadFloats )
{
    std::vector<float> _floatArray = loadFloats( m_targetModule->getSymbolVa(L"floatArray"), TEST_ARRAY_SIZE);
    EXPECT_TRUE( std::equal( &floatArray[0], &floatArray[TEST_ARRAY_SIZE-1], _floatArray.begin() ) );
}

TEST_F( MemoryTest, LoadDoubles )
{
    std::vector<double> _doubleArray = loadDoubles( m_targetModule->getSymbolVa(L"doubleArray"), TEST_ARRAY_SIZE);
    EXPECT_TRUE( std::equal( &doubleArray[0], &doubleArray[TEST_ARRAY_SIZE-1], _doubleArray.begin() ) );
}

TEST_F( MemoryTest, ReadPtr )
{
    EXPECT_EQ( (unsigned char)bigValue, ptrByte( m_targetModule->getSymbolVa(L"bigValue") ) );
    EXPECT_EQ( (unsigned short)bigValue, ptrWord( m_targetModule->getSymbolVa(L"bigValue") ) );
    EXPECT_EQ( (unsigned long)bigValue, ptrDWord( m_targetModule->getSymbolVa(L"bigValue") ) );
    EXPECT_EQ( (unsigned long long)bigValue, ptrQWord( m_targetModule->getSymbolVa(L"bigValue") ) );
    EXPECT_EQ( (char)bigValue, ptrSignByte( m_targetModule->getSymbolVa(L"bigValue") ) );
    EXPECT_EQ( (short)bigValue, ptrSignWord( m_targetModule->getSymbolVa(L"bigValue") ) );
    EXPECT_EQ( (long)bigValue, ptrSignDWord( m_targetModule->getSymbolVa(L"bigValue") ) );
    EXPECT_EQ( (long long)bigValue, ptrSignQWord( m_targetModule->getSymbolVa(L"bigValue") ) );
    EXPECT_EQ( (unsigned long long)bigValue, ptrQWord( ptrPtr(m_targetModule->getSymbolVa(L"pbigValue") ) ) );
}

TEST_F( MemoryTest, LoadChars )
{
    std::string  _helloStr(helloStr);
    std::wstring  _helloWStr(helloWStr);

    EXPECT_EQ( _helloStr, loadChars( m_targetModule->getSymbolVa(L"helloStr"), _helloStr.size() ) );
    EXPECT_EQ( _helloWStr, loadWChars( m_targetModule->getSymbolVa(L"helloWStr"), _helloWStr.size() ) );
}

TEST_F( MemoryTest, loadCStr )
{
    EXPECT_EQ( "Hello", loadCStr( m_targetModule->getSymbolVa(L"helloStr") ) );
    EXPECT_EQ( L"Hello", loadWStr( m_targetModule->getSymbolVa(L"helloWStr") ) );
}

TEST_F( MemoryTest, testBigCStr )
{
    EXPECT_EQ( strlen(bigCStr), loadCStr( m_targetModule->getSymbolVa(L"bigCStr") ).length() );
    EXPECT_EQ( wcslen(bigWStr), loadWStr( m_targetModule->getSymbolVa(L"bigWStr") ).length() );
}

TEST_F(MemoryTest, InvalidBigRegion)
{
    MEMOFFSET_64  offset = m_targetModule->getSymbolVa(L"bigValue");

    EXPECT_THROW(loadBytes(offset, 0xF0000000), MemoryException);
    EXPECT_THROW(loadWords(offset, 0xF0000000), MemoryException);
    EXPECT_THROW(loadDWords(offset, 0xF0000000), MemoryException);
    EXPECT_THROW(loadQWords(offset, 0xF0000000), MemoryException);
    EXPECT_THROW(loadSignBytes(offset, 0xF0000000), MemoryException);
    EXPECT_THROW(loadSignWords(offset, 0xF0000000), MemoryException);
    EXPECT_THROW(loadSignDWords(offset, 0xF0000000), MemoryException);
    EXPECT_THROW(loadSignQWords(offset, 0xF0000000), MemoryException);
    EXPECT_THROW(loadFloats(offset, 0xF0000000), MemoryException);
    EXPECT_THROW(loadDoubles(offset, 0xF0000000), MemoryException);
    EXPECT_THROW(loadChars(offset, 0xF0000000), MemoryException);
    EXPECT_THROW(loadWChars(offset, 0xF0000000), MemoryException);
}


