#pragma once

#include <vector>

#include <boost/shared_ptr.hpp>

#include <kdlib/dbgtypedef.h>
#include <kdlib/variant.h>

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class DataAccessor;
typedef boost::shared_ptr<DataAccessor> DataAccessorPtr;

class DataAccessor
{
public:

    virtual size_t getLength() const = 0;
    virtual unsigned char readByte(size_t pos=0) const = 0;
    virtual char readSignByte(size_t pos=0) const = 0;
    virtual unsigned short readWord(size_t pos=0) const = 0;
    virtual short readSignWord(size_t pos=0) const = 0;
    virtual unsigned long readDWord(size_t pos=0) const = 0;
    virtual long readSignDWord(size_t pos=0) const = 0;
    virtual unsigned long long readQWord(size_t pos=0) const = 0;
    virtual long long readSignQWord(size_t pos=0) const = 0;
    virtual float readFloat(size_t pos=0) const = 0;
    virtual double readDouble(size_t pos=0) const = 0;

    virtual void readBytes( std::vector<unsigned char>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void readWords( std::vector<unsigned short>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void readDWords( std::vector<unsigned long>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void readQWords( std::vector<unsigned long long>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void readSignBytes( std::vector<char>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void readSignWords( std::vector<short>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void readSignDWords( std::vector<long>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void readSignQWords( std::vector<long long>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void readFloats( std::vector<float>&  dataRange, size_t count, size_t  pos=0) const = 0;
    virtual void readDoubles( std::vector<double>&  dataRange, size_t count, size_t  pos=0) const = 0;

    virtual DataAccessorPtr clone(size_t count, size_t  pos=0) = 0;
    virtual MEMOFFSET_64 getAddress() const = 0;
    virtual VarStorage getStorageType() const = 0;
    virtual std::wstring getRegisterName() const = 0;
};

///////////////////////////////////////////////////////////////////////////////

DataAccessorPtr getMemoryAccessor( MEMOFFSET_64  offset, size_t length);
DataAccessorPtr getCacheAccessor(const std::vector<char>& buffer);
DataAccessorPtr getVariantAccessor(const NumVariant& var);
DataAccessorPtr  getRegisterAccessor(const NumVariant& registerValue, const std::wstring& registerName);
DataAccessorPtr  getEmptyAccessor();

///////////////////////////////////////////////////////////////////////////////

}
