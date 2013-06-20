#pragma once

#include <string>
#include <sstream>
#include <exception>

#include "kdlib/dbgtypedef.h"
#include "kdlib/dbgengine.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class DbgException : public std::exception
{
public:

    DbgException( const std::wstring  &desc ) :
        m_desc(desc),
        std::exception( "KDLIBCPP Exception" )
        {}

    const  std::wstring& getDesc() const {
        return m_desc;
    }

private:

    std::wstring    m_desc;

};

///////////////////////////////////////////////////////////////////////////////

class MemoryException : public DbgException
{
public:

    MemoryException( MEMOFFSET_64 targetAddr, bool phyAddr = false ) :
        m_targetAddress( targetAddr ),
        DbgException( buildDesc( targetAddr, phyAddr ) )
        {}    
    
    MEMOFFSET_64
    getAddress() const {
        return m_targetAddress;
    }
    
private:    
        
    MEMOFFSET_64     m_targetAddress;

    static std::wstring buildDesc( MEMOFFSET_64 addr, bool phyAddr )
    {
        std::wstringstream   sstr;
        if ( phyAddr )
            sstr << L"Memory exception at 0x" << std::hex << addr << L" target physical address";
        else
            sstr << L"Memory exception at 0x" << std::hex << addr << L" target virtual address";
       
        return sstr.str();
    }   
};

/////////////////////////////////////////////////////////////////////////////////

class SymbolException : public DbgException 
{
public:

    SymbolException( const std::wstring  &desc ) :
        DbgException( desc )
        {}    

};

/////////////////////////////////////////////////////////////////////////////////

class TypeException : public SymbolException
{
public:

    TypeException( const std::wstring &typeName, const std::wstring  &errorStr )
        : SymbolException( buildDesc( typeName, errorStr ) )
        {}

private:

    static std::wstring buildDesc( const std::wstring &typeName, const std::wstring  &errorStr )
    {
        std::wstringstream   sstr;
        sstr << typeName << L" : " << errorStr;
        return sstr.str();
    }
};

/////////////////////////////////////////////////////////////////////////////////


class IndexException : public std::exception
{
public:

    IndexException( size_t index ) {}

/*private:

    static std::wstring buildDesc( size_t index )
    {
        std::wstringstream   sstr;
        sstr << L"index is out of range";
        return sstr.str();
    }
*/
};

/////////////////////////////////////////////////////////////////////////////////

class ImplementException : public DbgException
{
public:

    ImplementException( const std::wstring &file, int line, const std::wstring &msg ) :
         DbgException( buildDesc(file,line, msg) )    
         {}

private:

    static std::wstring buildDesc( const std::wstring &file, int line, const std::wstring &msg )
    {
        std::wstringstream sstream;
        sstream << L"File: " << file << L" Line: " << line << L"  " << msg;
        return sstream.str();
    }

};

//#define WIDEN2(x)
#define WIDEN(x) L#x
#define __WFILE__ WIDEN(__FILE__)

#define NOT_IMPLEMENTED()   throw ImplementException( __WFILE__, __LINE__, L"Not Implemented" );
#define TODO(x) throw ImplementException( __WFILE__, __LINE__, WIDEN(x) );

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end
