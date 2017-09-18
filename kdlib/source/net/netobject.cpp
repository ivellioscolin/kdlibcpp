#include "stdafx.h"

#include "kdlib/exceptions.h"
#include "kdlib/memaccess.h"

#include "net/netobject.h"
#include "net/net.h"
#include "net/nettype.h"

namespace kdlib {


///////////////////////////////////////////////////////////////////////////////

TypedVarPtr getManagedVar(MEMOFFSET_64 addr)
{

    COR_TYPEID  typeId;
    HRESULT hres = g_netMgr->targetProcess5()->GetTypeID(addr, &typeId);
    if (FAILED(hres))
        throw DbgException("Failed ICorDebugProcess5::GetTypeID");

    TypeInfoPtr  typeInfo = getNetTypeById(typeId);

    return loadTypedVar(typeInfo, addr);
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr NetObjectClass::getElement( const std::wstring& fieldName )
{
    TypeInfoPtr fieldType = m_typeInfo->getElement( fieldName );

    MEMOFFSET_32   fieldOffset = m_typeInfo->getElementOffset(fieldName);

    return  loadTypedVar( fieldType,  m_dataAccessor->copy(fieldOffset, fieldType->getSize()) );
}

///////////////////////////////////////////////////////////////////////////////


NetObjectArray::NetObjectArray(const TypeInfoPtr&  typeInfo, MEMOFFSET_64 addr, const  COR_ARRAY_LAYOUT& arrayLayout, std::vector<size_t> indices ) :
    m_typeInfo(typeInfo),
    m_arrayLayout(arrayLayout),
    m_arrayObjAddr(addr)
{
    m_elementSize = m_arrayLayout.elementSize;

    m_elementCount = ptrDWord(addr + m_arrayLayout.countOffset);

    m_indices = indices;

    if ( arrayLayout.numRanks == 1 )
    {
        m_ranks.push_back(m_elementCount);
    }
    else
    {
        for ( auto rank = 0; rank < arrayLayout.numRanks; rank++)
        {
            switch( arrayLayout.rankSize )
            {
            case 1:
                m_ranks.push_back( ptrByte(addr + m_arrayLayout.rankOffset + arrayLayout.rankSize * m_ranks.size() ) );
                break;

            case 2:
                m_ranks.push_back( ptrWord(addr + m_arrayLayout.rankOffset + arrayLayout.rankSize * m_ranks.size() ) );
                break;

            case 4:
                m_ranks.push_back( ptrDWord(addr + m_arrayLayout.rankOffset + arrayLayout.rankSize * m_ranks.size() ) );
                break;

            case 8:
                m_ranks.push_back( ptrQWord(addr + m_arrayLayout.rankOffset + arrayLayout.rankSize * m_ranks.size() ) );
                break;

            default:

                throw DbgException("Unsupported array rank size");
            }
        }
    }

    MEMOFFSET_64  dataBegin = addr + m_arrayLayout.firstElementOffset;
    
    m_dataAccessor = getMemoryAccessor(dataBegin, m_elementCount*m_elementSize);
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr NetObjectArray::getElement( size_t index )
{
    if ( index >= m_ranks[ m_indices.size() ]  )
        throw IndexException( index );

    auto  indices = m_indices;
    indices.push_back(index);

    if ( m_indices.size() + 1 < m_ranks.size() )
    {
        return TypedVarPtr( new NetObjectArray(m_typeInfo, m_arrayObjAddr, m_arrayLayout, indices) );
    }

    TypeInfoPtr  elementType;

    switch ( m_arrayLayout.componentType )
    {
    case ELEMENT_TYPE_CHAR:
       elementType = loadType(L"Char");
       break;

    case ELEMENT_TYPE_I1:
        elementType = loadType(L"Int1B");
        break;

    case ELEMENT_TYPE_U1:
        elementType = loadType(L"UInt1B");
        break;

    case ELEMENT_TYPE_I2:
        elementType = loadType(L"Int2B");
        break;

    case ELEMENT_TYPE_U2:
        elementType = loadType(L"UInt1B");
        break;

    case ELEMENT_TYPE_I4:
        elementType = loadType(L"Int4B");
        break;

    case ELEMENT_TYPE_U4:
        elementType = loadType(L"UInt4B");
        break;

    case ELEMENT_TYPE_I8:
        elementType = loadType(L"Int8B");
        break;

    case ELEMENT_TYPE_U8:
        elementType = loadType(L"UInt8B");
        break;

    case ELEMENT_TYPE_R4:
        elementType = loadType(L"Float");
        break;

    case ELEMENT_TYPE_R8:
        elementType = loadType(L"Double");
        break;

    case ELEMENT_TYPE_CLASS:
        elementType = getNetTypeById(m_arrayLayout.componentID);
        break;

    default:
        NOT_IMPLEMENTED();
    }

    size_t  rawIndex = 0;
    size_t  elements = m_elementCount;

    for ( size_t i = 0; i < m_ranks.size(); ++i)
    {
        elements =  elements / m_ranks[i];
        rawIndex += elements * indices[i];
    }

    return loadTypedVar( elementType,  m_dataAccessor->copy(m_elementSize*rawIndex, m_elementSize) );
}

///////////////////////////////////////////////////////////////////////////////

size_t NetObjectArray::getElementCount()
{
    return m_ranks.back();
}

///////////////////////////////////////////////////////////////////////////////

}

