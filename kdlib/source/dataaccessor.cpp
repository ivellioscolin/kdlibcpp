#include "stdafx.h"

#include "dataaccessorimpl.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

DataAccessorPtr  getMemoryAccessor( MEMOFFSET_64  offset, size_t length) 
{
    return DataAccessorPtr( new MemoryAccessor(offset, length) );
}

///////////////////////////////////////////////////////////////////////////////

DataAccessorPtr  getEmptyAccessor()
{
    return DataAccessorPtr( new EmptyAccessor() );
}

///////////////////////////////////////////////////////////////////////////////

DataAccessorPtr  getRegisterAccessor(const std::wstring& registerName)
{
    return DataAccessorPtr(new RegisterAccessor(registerName));
}

///////////////////////////////////////////////////////////////////////////////

DataAccessorPtr getCacheAccessor(size_t bufferSize)
{
    return DataAccessorPtr(new CacheAccessor(bufferSize) );
}

///////////////////////////////////////////////////////////////////////////////

DataAccessorPtr getCacheAccessor(const std::vector<char>& buffer)
{
    return DataAccessorPtr(new CacheAccessor(buffer) );
}
///////////////////////////////////////////////////////////////////////////////

DataAccessorPtr getCacheAccessor(const NumVariant& var)
{
    return DataAccessorPtr(new CacheAccessor(var));
}

///////////////////////////////////////////////////////////////////////////////

}
