#pragma once

#include <boost/noncopyable.hpp>

#include "kdlib/dataaccessor.h"
#include "kdlib/memaccess.h"
#include "kdlib/exceptions.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class EmptyAccessor : public DataAccessor
{
private:

    virtual size_t getLength() const
    {
        throw DbgException("data accessor no data");
    }

    virtual unsigned char readByte(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeByte(unsigned char value, size_t pos=0)
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeByte(size_t pos=0) 
    {
        throw DbgException("data accessor no data");
    }

    virtual char readSignByte(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeSignByte(char value, size_t pos=0) 
    {
        throw DbgException("data accessor no data");
    }

    virtual unsigned short readWord(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeWord(unsigned short value, size_t pos=0) 
    {
        throw DbgException("data accessor no data");
    }

    virtual short readSignWord(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeSignWord(short value, size_t pos=0)
    {
        throw DbgException("data accessor no data");
    }

    virtual unsigned long readDWord(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeDWord(unsigned long value, size_t pos)
    {
        throw DbgException("data accessor no data");
    }

    virtual long readSignDWord(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeSignDWord(long value, size_t pos=0) 
    {
        throw DbgException("data accessor no data");
    }

    virtual unsigned long long readQWord(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeQWord(unsigned long long value, size_t pos=0) 
    {
        throw DbgException("data accessor no data");
    }

    virtual long long readSignQWord(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeSignQWord(long long value, size_t pos=0) 
    {
        throw DbgException("data accessor no data");
    }

    virtual float readFloat(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeFloat(float value, size_t pos=0)
    {
        throw DbgException("data accessor no data");
    }

    virtual double readDouble(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeDouble(double value, size_t pos=0) 
    {
        throw DbgException("data accessor no data");
    }

    virtual void readBytes(std::vector<unsigned char>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeBytes( const std::vector<unsigned char>&  dataRange, size_t pos=0)
    {
        throw DbgException("data accessor no data");
    }

    virtual void readWords(std::vector<unsigned short>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeWords( const std::vector<unsigned short>&  dataRange, size_t pos=0)
    {
        throw DbgException("data accessor no data");
    }

    virtual void readDWords(std::vector<unsigned long>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeDWords( const std::vector<unsigned long>&  dataRange, size_t  pos=0) 
    {
        throw DbgException("data accessor no data");
    }

    virtual void readQWords(std::vector<unsigned long long>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeQWords( const std::vector<unsigned long long>&  dataRange, size_t  pos=0) 
    {
        throw DbgException("data accessor no data");
    }

    virtual void readSignBytes(std::vector<char>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeSignBytes( const std::vector<char>&  dataRange, size_t  pos=0)
    {
        throw DbgException("data accessor no data");
    }

    virtual void readSignWords(std::vector<short>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeSignWords( const std::vector<short>&  dataRange, size_t  pos=0) 
    {
        throw DbgException("data accessor no data");
    }

    virtual void readSignDWords(std::vector<long>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeSignDWords( const std::vector<long>&  dataRange, size_t  pos=0)
    {
        throw DbgException("data accessor no data");
    }

    virtual void readSignQWords(std::vector<long long>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeSignQWords( const std::vector<long long>&  dataRange, size_t  pos=0) 
    {
        throw DbgException("data accessor no data");
    }

    virtual void readFloats(std::vector<float>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeFloats( const std::vector<float>&  dataRange, size_t  pos=0) 
    {
        throw DbgException("data accessor no data");
    }

    virtual void readDoubles(std::vector<double>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void writeDoubles( const std::vector<double>&  dataRange, size_t  pos=0) 
    {
        throw DbgException("data accessor no data");
    }

    virtual MEMOFFSET_64 getAddress() const
    {
        throw DbgException("data accessor no data");
    }

    virtual VarStorage getStorageType() const
    {
        return UnknownVar;
    }

    virtual std::wstring getRegisterName() const
    {
        throw DbgException("data accessor no data");
    }
};

///////////////////////////////////////////////////////////////////////////////

class MemoryAccessor : public EmptyAccessor
{
public:

    MemoryAccessor(MEMOFFSET_64 offset, size_t length) :
        m_begin(addr64(offset)),
        m_length(length)
    {}

private:

    virtual size_t getLength() const
    {
        return m_length;
    }

    virtual unsigned char readByte(size_t pos = 0) const
    {
        if (pos >= m_length)
            throw DbgException("memory accessor range error");

        return ptrByte(m_begin + pos);
    }

    virtual void writeByte(unsigned char value, size_t pos=0) 
    {
        if (pos >= m_length)
            throw DbgException("memory accessor range error");

        setByte(m_begin + pos, value);
    }

    virtual char readSignByte(size_t pos = 0) const
    {
        if (pos >= m_length)
            throw DbgException("memory accessor range error");
        return ptrSignByte(m_begin + pos);
    }

    virtual void writeSignByte(char value, size_t pos=0)
    {
        if (pos >= m_length)
            throw DbgException("memory accessor range error");
        setSignByte(m_begin + pos, value);
    }

    virtual unsigned short readWord(size_t pos = 0) const
    {
        if (pos >= m_length / sizeof(unsigned short) )
            throw DbgException("memory accessor range error");
        return ptrWord(m_begin + pos * sizeof(unsigned short));
    }

    virtual void writeWord(unsigned short value, size_t pos=0)
    {
        if (pos >= m_length / sizeof(unsigned short) )
            throw DbgException("memory accessor range error");
        setWord(m_begin + pos * sizeof(unsigned short), value);
    }

    virtual short readSignWord(size_t pos = 0) const
    {
        if (pos  >= m_length / sizeof(short) )
            throw DbgException("memory accessor range error");
        return ptrSignWord(m_begin + pos * sizeof(short));
    }

    virtual void writeSignWord(short value, size_t pos=0)
    {
        if (pos >= m_length / sizeof(short) )
            throw DbgException("memory accessor range error");
        setSignWord(m_begin + pos * sizeof(short), value);
    }

    virtual unsigned long readDWord(size_t pos = 0) const
    {
        if (pos >= m_length / sizeof(unsigned long) )
            throw DbgException("memory accessor range error");
        return ptrDWord(m_begin + pos * sizeof(unsigned long));
    }

    virtual void writeDWord(unsigned long value, size_t pos)
    {
        if (pos >= m_length / sizeof(unsigned long) )
            throw DbgException("memory accessor range error");
        setDWord(m_begin + pos * sizeof(unsigned long), value);
    }

    virtual long readSignDWord(size_t pos = 0) const
    {
        if (pos >= m_length /sizeof(long) )
            throw DbgException("memory accessor range error");
        return ptrSignDWord(m_begin + pos * sizeof(long));
    }

    virtual void writeSignDWord(long value, size_t pos=0)
    {
        if (pos >= m_length /sizeof(long) )
            throw DbgException("memory accessor range error");
        setSignDWord(m_begin + pos * sizeof(long), value);
    }

    virtual unsigned long long readQWord(size_t pos = 0) const
    {
        if (pos >= m_length / sizeof(unsigned long long) )
            throw DbgException("memory accessor range error");
        return ptrQWord(m_begin + pos * sizeof(unsigned long long));
    }

    virtual void writeQWord(unsigned long long value, size_t pos=0)
    {
        if (pos >= m_length / sizeof(unsigned long long) )
            throw DbgException("memory accessor range error");
        setQWord(m_begin + pos * sizeof(unsigned long long), value);
    }

    virtual long long readSignQWord(size_t pos = 0) const
    {
        if ( pos >= m_length / sizeof(long long) )
            throw DbgException("memory accessor range error");
        return ptrSignQWord(m_begin + pos * sizeof(long long));
    }

    virtual void writeSignQWord(long long value, size_t pos=0) 
    {
        if ( pos >= m_length / sizeof(long long) )
            throw DbgException("memory accessor range error");
        setSignQWord(m_begin + pos * sizeof(long long), value);
    }

    virtual float readFloat(size_t pos = 0) const
    {
        if (pos >= m_length / sizeof(float) )
            throw DbgException("memory accessor range error");
        return ptrSingleFloat(m_begin + pos * sizeof(float));
    }

    virtual void writeFloat(float value, size_t pos=0) 
    {
        if (pos >= m_length / sizeof(float) )
            throw DbgException("memory accessor range error");

        setSingleFloat(m_begin + pos * sizeof(float), value);
    }

    virtual double readDouble(size_t pos = 0) const
    {
        if (pos >= m_length / sizeof(double) )
            throw DbgException("memory accessor range error");
        return ptrDoubleFloat(m_begin + pos * sizeof(double));
    }

    virtual void writeDouble(double value, size_t pos=0)
    {
        if (pos >= m_length / sizeof(double) )
            throw DbgException("memory accessor range error");
        
        setDoubleFloat(m_begin + pos * sizeof(double), value);
    }

    virtual void readBytes(std::vector<unsigned char>& dataRange, size_t count, size_t pos = 0) const
    {
        if ( count > m_length - pos )
            throw DbgException("memory accessor range error");

        dataRange = loadBytes(m_begin + pos, count);
    }

    virtual void writeBytes( const std::vector<unsigned char>&  dataRange, size_t pos=0) 
    {
        if ( dataRange.size() > m_length - pos )
            throw DbgException("memory accessor range error");
        kdlib::writeBytes(m_begin + pos, dataRange);
    }

    virtual void readWords(std::vector<unsigned short>& dataRange, size_t count, size_t pos = 0) const
    {
        if ( count > m_length / sizeof(unsigned short) - pos )
            throw DbgException("memory accessor range error");

        dataRange = loadWords(m_begin + pos* sizeof(unsigned short), count);
    }

    virtual void writeWords( const std::vector<unsigned short>&  dataRange, size_t pos=0) 
    {
        if ( dataRange.size() > m_length / sizeof(unsigned short) - pos )
            throw DbgException("memory accessor range error");

        kdlib::writeWords(m_begin + pos* sizeof(unsigned short), dataRange);
    }

    virtual void readDWords(std::vector<unsigned long>& dataRange, size_t count, size_t pos = 0) const
    {
        if ( count > m_length / sizeof(unsigned long) - pos )
            throw DbgException("memory accessor range error");

        dataRange = loadDWords(m_begin + pos* sizeof(unsigned long), count);
    }

    virtual void writeDWords( const std::vector<unsigned long>&  dataRange, size_t  pos=0)
    {
        if ( dataRange.size() > m_length / sizeof(unsigned long) - pos )
            throw DbgException("memory accessor range error");

        kdlib::writeDWords(m_begin + pos* sizeof(unsigned long), dataRange);
    }

    virtual void readQWords(std::vector<unsigned long long>& dataRange, size_t count, size_t pos = 0) const
    {
        if ( count > m_length / sizeof(unsigned long long) - pos )
            throw DbgException("memory accessor range error");

        dataRange = loadQWords(m_begin + pos* sizeof(unsigned long long), count);
    }

    virtual void writeQWords( const std::vector<unsigned long long>&  dataRange, size_t  pos=0)
    {
        if ( dataRange.size() > m_length / sizeof(unsigned long long) - pos )
            throw DbgException("memory accessor range error");

        kdlib::writeQWords(m_begin + pos * sizeof(unsigned long long), dataRange);
    }

    virtual void readSignBytes(std::vector<char>& dataRange, size_t count, size_t pos = 0) const
    {
        if ( count > m_length - pos )
            throw DbgException("memory accessor range error");

        dataRange = loadSignBytes(m_begin + pos* sizeof(char), count);
    }

    virtual void writeSignBytes( const std::vector<char>&  dataRange, size_t  pos=0)
    {
        if ( dataRange.size() > m_length  - pos )
            throw DbgException("memory accessor range error");

        kdlib::writeSignBytes(m_begin + pos, dataRange);
    }

    virtual void readSignWords(std::vector<short>& dataRange, size_t count, size_t pos = 0) const
    {
        if ( count > m_length / sizeof(short) - pos )
            throw DbgException("memory accessor range error");

        dataRange = loadSignWords(m_begin + pos* sizeof(short), count);
    }

    virtual void writeSignWords( const std::vector<short>&  dataRange, size_t  pos=0)
    {
        if ( dataRange.size() > m_length / sizeof(short) - pos )
            throw DbgException("memory accessor range error");

        kdlib::writeSignWords(m_begin + pos * sizeof(short), dataRange);
    }

    virtual void readSignDWords(std::vector<long>& dataRange, size_t count, size_t pos = 0) const
    {
        if ( count > m_length / sizeof(long) - pos )
            throw DbgException("memory accessor range error");

        dataRange = loadSignDWords(m_begin + pos* sizeof(long), count);
    }

    virtual void writeSignDWords( const std::vector<long>&  dataRange, size_t  pos=0)
    {
        if ( dataRange.size() > m_length / sizeof(long) - pos )
            throw DbgException("memory accessor range error");

        kdlib::writeSignDWords(m_begin + pos * sizeof(long), dataRange);
    }

    virtual void readSignQWords(std::vector<long long>& dataRange, size_t count, size_t pos = 0) const
    {
        if ( count > m_length / sizeof(long long) - pos )
            throw DbgException("memory accessor range error");

        dataRange = loadSignQWords(m_begin + pos* sizeof(long long), count);
    }

    virtual void writeSignQWords( const std::vector<long long>&  dataRange, size_t  pos=0)
    {
        if ( dataRange.size() > m_length / sizeof(long long) - pos )
            throw DbgException("memory accessor range error");

        kdlib::writeSignQWords(m_begin + pos * sizeof(long long), dataRange);
    }

    virtual void readFloats(std::vector<float>& dataRange, size_t count, size_t pos = 0) const
    {
        if ( count > m_length / sizeof(float) - pos )
            throw DbgException("memory accessor range error");

        dataRange = loadFloats(m_begin + pos* sizeof(float), count);
    }

    virtual void writeFloats( const std::vector<float>&  dataRange, size_t  pos=0) 
    {
        if ( dataRange.size() > m_length / sizeof(float) - pos )
            throw DbgException("memory accessor range error");

        kdlib::writeFloats(m_begin + pos * sizeof(float), dataRange);
    }

    virtual void readDoubles(std::vector<double>& dataRange, size_t count, size_t pos = 0) const
    {
        if ( count > m_length / sizeof(double) || pos > m_length / sizeof(double) - count )
            throw DbgException("memory accessor range error");

        dataRange = loadDoubles(m_begin + pos* sizeof(double), count);
    }

    virtual void writeDoubles( const std::vector<double>&  dataRange, size_t  pos=0) 
    {
        if ( dataRange.size() > m_length / sizeof(double) - pos )
            throw DbgException("memory accessor range error");

        kdlib::writeDoubles(m_begin + pos * sizeof(double), dataRange);
    }

    virtual MEMOFFSET_64 getAddress() const {
        return m_begin;
    }

    virtual VarStorage getStorageType() const
    {
        return MemoryVar;
    }


private:

    MEMOFFSET_64  m_begin;
    size_t  m_length;
};


///////////////////////////////////////////////////////////////////////////////

class CacheAccessor : public EmptyAccessor
{
public:

    CacheAccessor(const std::vector<char>& buffer) :
        m_buffer(buffer)
    {}

    virtual size_t getLength() const
    {
        return m_buffer.size();
    }

private:

    std::vector<char>  m_buffer;
};

///////////////////////////////////////////////////////////////////////////////

class VariantAccessor : public EmptyAccessor
{
public:
    VariantAccessor(const NumVariant& var) :
        m_variant(var)
        {}

    virtual MEMOFFSET_64 getAddress() const
    {
        return ~0;
    }

    virtual unsigned char readByte(size_t pos = 0) const
    {
        return m_variant.asUChar();
    }

    virtual char readSignByte(size_t pos = 0) const
    {
        return m_variant.asChar();
    }

    virtual unsigned short readWord(size_t pos = 0) const
    {
        return m_variant.asUShort();
    }

    virtual short readSignWord(size_t pos = 0) const
    {
        return m_variant.asShort();
    }

    virtual unsigned long readDWord(size_t pos = 0) const
    {
        return m_variant.asULong();
    }

    virtual long readSignDWord(size_t pos = 0) const
    {
        return m_variant.asLong();
    }

    virtual unsigned long long readQWord(size_t pos = 0) const
    {
        return m_variant.asULongLong();
    }

    virtual long long readSignQWord(size_t pos = 0) const
    {
        return m_variant.asLongLong();
    }

private:
    NumVariant m_variant;
};

///////////////////////////////////////////////////////////////////////////////

class RegisterAccessor : public VariantAccessor
{
public:

    RegisterAccessor(const NumVariant& registerValue, const std::wstring& registerName) :
        VariantAccessor(registerValue),
        m_regName(registerName)
    {}


    virtual VarStorage getStorageType() const
    {
        return RegisterVar;
    }

    virtual std::wstring getRegisterName() const
    {
        return m_regName;
    }

private:

    std::wstring  m_regName;
};

///////////////////////////////////////////////////////////////////////////////

}
