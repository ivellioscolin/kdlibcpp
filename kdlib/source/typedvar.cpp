#include "stdafx.h"

#include <boost/regex.hpp>

#include "kdlib/typedvar.h"
#include "kdlib/module.h"

#include "typedvarimp.h"


namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr loadTypedVar( const std::wstring &varName )
{
    std::wstring     moduleName;
    std::wstring     symName;

    splitSymName( varName, moduleName, symName );

    ModulePtr  module;

    if ( moduleName.empty() )
    {
        MEMOFFSET_64 moduleOffset = findModuleBySymbol( symName );
        module = loadModule( moduleOffset );
    }
    else
    {
        module = loadModule( moduleName );
    }

    SymbolPtr  symVar = module->getSymbolScope()->getChildByName( symName );

    TypeInfoPtr varType = loadType( symVar );

    if ( LocIsConstant != symVar->getLocType() )
    {
        MEMOFFSET_64 offset = module->getBase() + symVar->getRva();

        return getTypedVar( varType, VarDataProviderPtr( new VarDataMemoryProvider(offset) ) );
    }

    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr loadTypedVar( const std::wstring &typeName, MEMOFFSET_64 offset )
{
    TypeInfoPtr varType = loadType( typeName );

    return getTypedVar( varType, VarDataProviderPtr( new VarDataMemoryProvider(offset) ) );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr loadTypedVar( const TypeInfoPtr &varType, MEMOFFSET_64 offset )
{
    if ( !varType )
        throw DbgException( L"type info is null");

    return getTypedVar( varType, VarDataProviderPtr( new VarDataMemoryProvider(offset) ) );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr getTypedVar( const TypeInfoPtr& typeInfo, VarDataProviderPtr &varData )
{
    if ( typeInfo->isBase() )
        return TypedVarPtr( new TypedVarBase( typeInfo, varData ) );

    if ( typeInfo->isUserDefined() )
        return TypedVarPtr( new TypedVarUdt( typeInfo, varData ) );

    if ( typeInfo->isPointer() )
        return TypedVarPtr( new  TypedVarPointer( typeInfo, varData ) );

    if ( typeInfo->isArray() )
        return TypedVarPtr( new TypedVarArray( typeInfo, varData ) );

    if ( typeInfo->isBitField() )
        return TypedVarPtr( new TypedVarBitField( typeInfo, varData ) );

    if ( typeInfo->isEnum() )
        return TypedVarPtr( new TypedVarEnum( typeInfo, varData ) );

    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////

NumVariant TypedVarBase::getValue() const
{
    if ( m_typeInfo->getName() == L"Char" )
        return NumVariant( m_varData->readSignByte() );

    if ( m_typeInfo->getName() == L"WChar" )
        return NumVariant( m_varData->readSignByte() );

    if ( m_typeInfo->getName() == L"Int1B" )
        return NumVariant( m_varData->readByte() );

    if ( m_typeInfo->getName() == L"UInt1B" )
        return NumVariant( m_varData->readByte() );

    if ( m_typeInfo->getName() == L"Int2B" )
        return NumVariant( m_varData->readSignWord() );

    if ( m_typeInfo->getName() == L"UInt2B" )
        return NumVariant( m_varData->readWord() );

    if ( m_typeInfo->getName() == L"Int4B" )
        return NumVariant( m_varData->readSignDWord() );

    if ( m_typeInfo->getName() == L"UInt4B" )
        return NumVariant( m_varData->readDWord() );

    if ( m_typeInfo->getName() == L"Int8B" )
        return NumVariant( m_varData->readSignQWord() );

    if ( m_typeInfo->getName() == L"UInt8B" )
        return NumVariant( m_varData->readQWord() );

    if ( m_typeInfo->getName() == L"Float" )
        return NumVariant( m_varData->readFloat() );

    if ( m_typeInfo->getName() == L"Double" )
        return NumVariant( m_varData->readDouble() );

    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr TypedVarUdt::getElement( const std::wstring& fieldName )
{
    TypeInfoPtr fieldType = m_typeInfo->getElement( fieldName );

    //if ( m_typeInfo->isStaticMember(fieldName) )
    //{
    //    ULONG64  staticOffset = m_typeInfo->getStaticOffsetByName( fieldName );

    //    if ( staticOffset == 0 )
    //        throw ImplementException( __FILE__, __LINE__, "Fix ME");

    //    return  TypedVar::getTypedVar( fieldType, VarDataMemory::factory(staticOffset) );
    //}

    MEMOFFSET_32   fieldOffset = m_typeInfo->getElementOffset(fieldName);

    //if ( m_typeInfo->isVirtualMember( fieldName ) )
    //{
    //    fieldOffset += getVirtualBaseDisplacement( fieldName );
    //}

    return  loadTypedVar( fieldType, m_varData->getAddress() + fieldOffset );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr TypedVarUdt::getElement( size_t index )
{
    TypeInfoPtr fieldType = m_typeInfo->getElement( index );

    MEMOFFSET_32   fieldOffset = m_typeInfo->getElementOffset(index);

    return  loadTypedVar( fieldType, m_varData->getAddress() + fieldOffset );
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr TypedVarArray::getElement( size_t index )
{
    if ( index >= m_typeInfo->getElementCount() )
        throw TypeException( m_typeInfo->getName(),  L"index is out of the elements range" );

    TypeInfoPtr     elementType = m_typeInfo->getElement(0);

    return loadTypedVar( elementType, m_varData->getAddress() + elementType->getSize()*index );
}

///////////////////////////////////////////////////////////////////////////////

NumVariant TypedVarBitField::getValue() const
{
    NumVariant  var = *loadTypedVar( m_typeInfo->getBitType(), m_varData->getAddress() );

    var >>=  m_typeInfo->getBitOffset();

    if ( var.isSigned() )
    {
        BITOFFSET width = m_typeInfo->getBitWidth();

        if ( ( var & ( 1ULL << ( width -1 ) ) ) != 0 )
        {
            var |= ~( ( 1ULL << width ) - 1 );
        }
        else
        {
            var &= ( 1ULL << width ) - 1;
        }
    }
    else
    {     

        var &= ( 1 << m_typeInfo->getBitWidth() ) - 1;
    }

    return var;
}

///////////////////////////////////////////////////////////////////////////////

} // end kdlib namesapce
