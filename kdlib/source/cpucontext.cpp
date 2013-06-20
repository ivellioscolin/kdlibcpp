#include "stdafx.h"

#include "kdlib/cpucontext.h"
#include "kdlib/dbgengine.h"
#include "kdlib/exceptions.h"

namespace kdlib {

/////////////////////////////////////////////////////////////

CPUContext::CPUContext( size_t index )
{
    if ( index == -1 )
    {
        index = getCurrentThreadId();
    }
    else
    {
        if ( index >= getNumberThreads() )
            throw IndexException(index);
    }

    m_contextIndex = index;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 CPUContext::getIP()
{
    return  getInstructionOffset( m_contextIndex );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 CPUContext::getSP()
{
     return  getStackOffset( m_contextIndex );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 CPUContext::getFP()
{
    return  getFrameOffset( m_contextIndex );
}

///////////////////////////////////////////////////////////////////////////////

NumVariant CPUContext::getRegisterByName( const std::wstring &name)
{
    size_t index = getRegsiterIndex( name );
    return getRegisterByIndex( index );
}

///////////////////////////////////////////////////////////////////////////////

NumVariant CPUContext::getRegisterByIndex( size_t index )
{
    CPURegType  regType = getRegisterType(index);

    switch( regType )
    {
    case  RegInt8:
        {
            char  val;
            getRegisterValue( index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;

    case  RegInt16:
        {
            short  val;
            getRegisterValue( index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;

    case  RegInt32:
        {
            long  val;
            getRegisterValue( index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;


    case  RegInt64:
        {
            long long  val;
            getRegisterValue( index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;

    case  RegFloat32:
        {
            float  val;
            getRegisterValue( index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;


    case  RegFloat64:
        {
            double  val;
            getRegisterValue( index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;
    }

    throw DbgException( L"unsupported registry type");
}

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end
