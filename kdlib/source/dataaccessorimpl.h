#pragma once

#include <boost/noncopyable.hpp>

#include "kdlib/dataaccessor.h"
#include "kdlib/memaccess.h"
#include "kdlib/exceptions.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class EmptyAccessor : public DataAccessor, boost::noncopyable
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

    virtual char readSignByte(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual unsigned short readWord(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual short readSignWord(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual unsigned long readDWord(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual long readSignDWord(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual unsigned long long readQWord(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual long long readSignQWord(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual float readFloat(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual double readDouble(size_t pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void readBytes(std::vector<unsigned char>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void readWords(std::vector<unsigned short>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void readDWords(std::vector<unsigned long>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void readQWords(std::vector<unsigned long long>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void readSignBytes(std::vector<char>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void readSignWords(std::vector<short>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void readSignDWords(std::vector<long>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void readSignQWords(std::vector<long long>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void readFloats(std::vector<float>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual void readDoubles(std::vector<double>&  dataRange, size_t count, size_t  pos = 0) const
    {
        throw DbgException("data accessor no data");
    }

    virtual DataAccessorPtr clone(size_t count, size_t  pos = 0)
    {
        throw DbgException("data accessor no data");
    }

    virtual MEMOFFSET_64 getAddress() const
    {
        throw DbgException("data accessor no data");
    }
};

///////////////////////////////////////////////////////////////////////////////

class MemoryAccessor : public DataAccessor, boost::noncopyable
{
public:

    MemoryAccessor(MEMOFFSET_64 offset, size_t length) :
        m_begin(offset),
        m_length(length)
    {}

private:
    virtual size_t getLength() const
    {
        return m_length;
    }

    virtual unsigned char readByte(size_t pos = 0) const
    {
        if (pos >= m_begin + m_length)
            throw DbgException("memory accessor range error");
        return ptrByte(m_begin + pos);
    }

    virtual char readSignByte(size_t pos = 0) const
    {
        if (pos >= m_begin + m_length)
            throw DbgException("memory accessor range error");
        return ptrSignByte(m_begin + pos);
    }

    virtual unsigned short readWord(size_t pos = 0) const
    {
        if (pos * sizeof(unsigned short) >= m_begin + m_length - (sizeof(unsigned short) - 1))
            throw DbgException("memory accessor range error");
        return ptrWord(m_begin + pos * sizeof(unsigned short));
    }

    virtual short readSignWord(size_t pos = 0) const
    {
        if (pos * sizeof(short) >= m_begin + m_length - (sizeof(short) - 1))
            throw DbgException("memory accessor range error");
        return ptrSignWord(m_begin + pos * sizeof(short));
    }

    virtual unsigned long readDWord(size_t pos = 0) const
    {
        if (pos * sizeof(unsigned long) >= m_begin + m_length - (sizeof(unsigned long) - 1))
            throw DbgException("memory accessor range error");
        return ptrDWord(m_begin + pos * sizeof(unsigned long));
    }

    virtual long readSignDWord(size_t pos = 0) const
    {
        if (pos * sizeof(long) >= m_begin + m_length - (sizeof(long) - 1))
            throw DbgException("memory accessor range error");
        return ptrSignDWord(m_begin + pos * sizeof(long));
    }

    virtual unsigned long long readQWord(size_t pos = 0) const
    {
        if (pos * sizeof(unsigned long long) >= m_begin + m_length - (sizeof(unsigned long long) - 1))
            throw DbgException("memory accessor range error");
        return ptrQWord(m_begin + pos * sizeof(unsigned long long));
    }

    virtual long long readSignQWord(size_t pos = 0) const
    {
        if (pos * sizeof(long long) >= m_begin + m_length - (sizeof(long long) - 1))
            throw DbgException("memory accessor range error");
        return ptrSignQWord(m_begin + pos * sizeof(long long));
    }

    virtual float readFloat(size_t pos = 0) const
    {
        if (pos * sizeof(float) >= m_begin + m_length - (sizeof(float) - 1))
            throw DbgException("memory accessor range error");
        return ptrSingleFloat(m_begin + pos * sizeof(float));
    }

    virtual double readDouble(size_t pos = 0) const
    {
        if (pos * sizeof(double) >= m_begin + m_length - (sizeof(double) - 1))
            throw DbgException("memory accessor range error");
        return ptrDoubleFloat(m_begin + pos * sizeof(double));
    }

    virtual void readBytes(std::vector<unsigned char>&  dataRange, size_t count, size_t pos = 0) const
    {
        if (pos + count >= m_begin + m_length)
            throw DbgException("memory accessor range error");
        dataRange = loadBytes(m_begin + pos, count);
    }

    virtual void readWords(std::vector<unsigned short>& dataRange, size_t count, size_t pos = 0) const
    {
        if ((pos + count + 1) * sizeof(unsigned short) > m_begin + m_length)
            throw DbgException("memory accessor range error");
        dataRange = loadWords(m_begin + pos* sizeof(unsigned short), count);
    }

    virtual void readDWords(std::vector<unsigned long>& dataRange, size_t count, size_t pos = 0) const
    {
        if ((pos + count + 1) * sizeof(unsigned long) > m_begin + m_length)
            throw DbgException("memory accessor range error");
        dataRange = loadDWords(m_begin + pos* sizeof(unsigned long), count);
    }

    virtual void readQWords(std::vector<unsigned long long>& dataRange, size_t count, size_t pos = 0) const
    {
        if ((pos + count + 1) * sizeof(unsigned long long) > m_begin + m_length)
            throw DbgException("memory accessor range error");
        dataRange = loadQWords(m_begin + pos* sizeof(unsigned long long), count);
    }

    virtual void readSignBytes(std::vector<char>& dataRange, size_t count, size_t pos = 0) const
    {
        if ((pos + count + 1) * sizeof(char) > m_begin + m_length)
            throw DbgException("memory accessor range error");
        dataRange = loadSignBytes(m_begin + pos* sizeof(char), count);
    }

    virtual void readSignWords(std::vector<short>& dataRange, size_t count, size_t pos = 0) const
    {
        if ((pos + count + 1) * sizeof(short) > m_begin + m_length)
            throw DbgException("memory accessor range error");
        dataRange = loadSignWords(m_begin + pos* sizeof(short), count);
    }

    virtual void readSignDWords(std::vector<long>& dataRange, size_t count, size_t pos = 0) const
    {
        if ((pos + count + 1) * sizeof(long) > m_begin + m_length)
            throw DbgException("memory accessor range error");
        dataRange = loadSignDWords(m_begin + pos* sizeof(long), count);
    }

    virtual void readSignQWords(std::vector<long long>& dataRange, size_t count, size_t pos = 0) const
    {
        if ((pos + count + 1) * sizeof(long long) > m_begin + m_length)
            throw DbgException("memory accessor range error");
        dataRange = loadSignQWords(m_begin + pos* sizeof(long long), count);
    }

    virtual void readFloats(std::vector<float>& dataRange, size_t count, size_t pos = 0) const
    {
        if ((pos + count + 1) * sizeof(float) > m_begin + m_length)
            throw DbgException("memory accessor range error");
        dataRange = loadFloats(m_begin + pos* sizeof(float), count);
    }

    virtual void readDoubles(std::vector<double>& dataRange, size_t count, size_t pos = 0) const
    {
        if ((pos + count + 1) * sizeof(double) > m_begin + m_length)
            throw DbgException("memory accessor range error");
        dataRange = loadDoubles(m_begin + pos* sizeof(double), count);
    }

    virtual DataAccessorPtr clone(size_t count, size_t pos = 0)
    {
        if (pos + count >= m_begin + m_length)
            throw DbgException("memory accessor range error");

        return getMemoryAccessor(m_begin + pos, count);
    }

    virtual MEMOFFSET_64 getAddress() const {
        return m_begin;
    }


private:

    MEMOFFSET_64  m_begin;
    size_t  m_length;
};


///////////////////////////////////////////////////////////////////////////////

class RegisterAccessor : public DataAccessor, boost::noncopyable
{
public:

    RegisterAccessor(unsigned long  regId) :
        m_regId(regId)
    {}

private:

    virtual size_t getLength() const {
        CPURegType  regType = getRegisterType(m_regId);

        switch (regType)
        {
        case RegInt8: return 1;
        case RegInt16: return 2;
        case RegInt32: return 4;
        case RegInt64: return 8;
        case RegFloat32: return 4;
        case RegFloat64: return 8;
        case RegFloat80: return 10;
        case RegFloat128: return 16;
        case RegVector64: return 8;
        case RegVector128: return 16;
        }
        throw DbgException("unknown register type");
    }

    template <typename T>
    T getValue() const
    {
        T  v;
        getRegisterValue(m_regId, &v, sizeof(v));
        return v;
    }

    virtual unsigned char readByte(size_t pos) const
    {
        return getValue<unsigned char>();
    }

    virtual char readSignByte(size_t pos) const
    {
        return getValue<char>();
    }

    virtual unsigned short readWord(size_t pos) const
    {
        return getValue<unsigned short>();
    }

    virtual short readSignWord(size_t pos) const
    {
        return getValue<short>();
    }

    virtual unsigned long readDWord(size_t pos) const
    {
        return getValue<unsigned long>();
    }

    virtual long readSignDWord(size_t pos) const
    {
        return getValue<long>();
    }

    virtual unsigned long long readQWord(size_t pos) const
    {
        return getValue<unsigned long long>();
    }

    virtual long long readSignQWord(size_t pos) const
    {
        return getValue<long long>();
    }


    virtual float readFloat(size_t pos) const
    {
        return getValue<float>();
    }

    virtual double readDouble(size_t pos = 0) const
    {
        return getValue<double>();
    }

    virtual void readBytes(std::vector<unsigned char>&  dataRange, size_t count, size_t  pos) const
    {
        NOT_IMPLEMENTED();
    }

    virtual void readWords(std::vector<unsigned short>&  dataRange, size_t count, size_t  pos) const
    {
        NOT_IMPLEMENTED();
    }

    virtual void readDWords(std::vector<unsigned long>&  dataRange, size_t count, size_t  pos) const
    {
        NOT_IMPLEMENTED();
    }

    virtual void readQWords(std::vector<unsigned long long>&  dataRange, size_t count, size_t  pos) const
    {
        NOT_IMPLEMENTED();
    }

    virtual void readSignBytes(std::vector<char>&  dataRange, size_t count, size_t  pos) const
    {
        NOT_IMPLEMENTED();
    }

    virtual void readSignWords(std::vector<short>&  dataRange, size_t count, size_t  pos) const
    {
        NOT_IMPLEMENTED();
    }
    virtual void readSignDWords(std::vector<long>&  dataRange, size_t count, size_t  pos) const
    {
        NOT_IMPLEMENTED();
    }
    virtual void readSignQWords(std::vector<long long>&  dataRange, size_t count, size_t  pos) const
    {
        NOT_IMPLEMENTED();
    }
    virtual void readFloats(std::vector<float>&  dataRange, size_t count, size_t  pos) const
    {
        NOT_IMPLEMENTED();
    }
    virtual void readDoubles(std::vector<double>&  dataRange, size_t count, size_t  pos) const
    {
        NOT_IMPLEMENTED();
    }

    virtual DataAccessorPtr clone(size_t count, size_t  pos)
    {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getAddress() const
    {
        return ~0;
    }

private:

    unsigned long m_regId;
};

}
