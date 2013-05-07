#pragma once

#include <sstream>

#include <Windows.h>

#include "kdlib/exceptions.h"

namespace  kdlib {

class DbgEngException : public DbgException
{
public:

    DbgEngException( const std::wstring  &methodName, HRESULT hres ) :
        DbgException( buildHresultDesc( methodName, hres ).c_str() )
        {}

private:

    std::wstring buildHresultDesc( const std::wstring  &methodName, HRESULT hres ) 
    {
        std::wstringstream sstr;
        sstr << L"Call " << methodName << L" failed\n";
        sstr << L"HRESULT 0x" << std::hex << hres;
        return sstr.str();
    }

};


} // kdlib namespace end 


