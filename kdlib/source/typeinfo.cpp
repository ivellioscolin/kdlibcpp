#include "stdafx.h"
#include <sstream>
#include <iomanip>
#include <regex>

#include "kdlib/exceptions.h"
#include "kdlib/dbgengine.h"
#include "kdlib/module.h"
#include "kdlib/typeinfo.h"


#include "typeinfoimp.h"
#include "typedvarimp.h"
#include "processmon.h"
#include "fnmatch.h"

namespace {

///////////////////////////////////////////////////////////////////////////////

static const std::wregex complexSymMatch(L"^([\\*]*)([^\\(\\)\\*\\[\\]]*)([\\(\\)\\*\\[\\]\\d]*)$");

std::wstring getTypeNameFromComplex( const std::wstring &fullTypeName )
{
    std::wsmatch    matchResult;

    if ( !std::regex_match( fullTypeName, matchResult, complexSymMatch ) )
        return L"";

    return std::wstring( matchResult[2].first, matchResult[2].second );
}

std::wstring getTypeSuffixFromComplex( const std::wstring &fullTypeName )
{
    std::wsmatch    matchResult;

    if ( !std::regex_match( fullTypeName, matchResult, complexSymMatch ) )
        return L"";

    return std::wstring( matchResult[3].first, matchResult[3].second );
}


///////////////////////////////////////////////////////////////////////////////

static const std::wregex bracketMatch(L"^([^\\(]*)\\((.*)\\)([^\\)]*)$"); 

void getBracketExpression( std::wstring &suffix, std::wstring &bracketExpression )
{
    std::wsmatch    matchResult;

    if ( std::regex_match( suffix, matchResult, bracketMatch  ) )
    {
        bracketExpression = std::wstring( matchResult[2].first, matchResult[2].second );
        
        std::wstring  newSuffix;

        if ( matchResult[1].matched )
            newSuffix += std::wstring( matchResult[1].first, matchResult[1].second );

        if ( matchResult[3].matched )
            newSuffix += std::wstring( matchResult[3].first, matchResult[3].second );

        suffix = newSuffix;
    }
}

///////////////////////////////////////////////////////////////////////////////

static const std::wregex ptrMatch(L"^\\*(.*)$");

bool getPtrExpression( std::wstring &suffix )
{
    std::wsmatch    matchResult;

    if ( std::regex_match( suffix, matchResult, ptrMatch  ) )
    {
        suffix = std::wstring(matchResult[1].first, matchResult[1].second );

        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

static const std::wregex arrayMatch(L"^(.*)\\[(\\d+)\\]$");

bool getArrayExpression( std::wstring &suffix, size_t &arraySize )
{
    std::wsmatch    matchResult;

    if ( std::regex_match( suffix, matchResult, arrayMatch  ) )
    {
        std::wstringstream  sstr;
        sstr << std::wstring(matchResult[2].first, matchResult[2].second );
        sstr >> arraySize;

        suffix = std::wstring(matchResult[1].first, matchResult[1].second );

        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

} // end nameless namespace

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

size_t getPointerSizeByMachine(const SymbolPtr &symbol)
{
    const auto machineType = symbol->getMachineType();
    switch (machineType)
    {
    case machine_I386:
    case machine_ARM:
        return 4;

    case machine_AMD64:
    case machine_ARM64:
        return 8;

    default:
        throw TypeException(L"unknown machine type");
    }
}

///////////////////////////////////////////////////////////////////////////////

std::wstring printStructType(TypeInfoPtr& structType)
{
    std::wstringstream  sstr;

    sstr << "class/struct : " << structType->getName() << " Size: 0x" << std::hex << structType->getSize() << " (" << std::dec << structType->getSize() << ")" << std::endl;
    
    size_t  fieldCount = structType->getElementCount();

    for ( size_t i = 0; i < fieldCount; ++i )
    {
        if (structType->isConstMember(i) )
        {
            continue;
        }
        else
        if ( structType->isStaticMember(i) )
        {
            sstr << L"   =" << std::right << std::setw(10) << std::setfill(L'0') << std::hex << structType->getElementVa(i);
            sstr << L" " << std::left << std::setw(18) << std::setfill(L' ') << structType->getElementName(i) << L':';
        }
        else
        {
            if ( structType->isVirtualMember(i) )
            {
                sstr << L"   virtual base ";
                sstr << L" +" << std::right << std::setw(4) << std::setfill(L'0') << std::hex << structType->getElementOffset(i);
                sstr << L" " << structType->getElementName(i) << L':';
            }
            else
            {
                sstr << L"   +" << std::right << std::setw(4) << std::setfill(L'0') << std::hex << structType->getElementOffset(i);
                sstr << L" " << std::left << std::setw(24) << std::setfill(L' ') << structType->getElementName(i) << L':';
            }
        }

        sstr << L" " << std::left << structType->getElement(i)->getName();
        sstr << std::endl;
    }

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

std::wstring printPointerType(TypeInfoPtr&  ptrType)
{
    return L"ptr to " + ptrType->deref()->getName();
}

///////////////////////////////////////////////////////////////////////////////

std::wstring printEnumType(TypeInfoPtr& enumType)
{
    std::wstringstream  sstr;

    sstr << L"enum: " << enumType->getName() << std::endl;

    size_t  fieldCount = enumType->getElementCount();

    for ( size_t i = 0; i < fieldCount; ++i )
    {
        sstr << L"   " << enumType->getElementName(i);
        sstr << L" = " << enumType->getElement(i)->getValue().asStr();
        sstr << std::endl;
    }

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

size_t getSymbolSize( const std::wstring &fullName )
{
    if ( TypeInfo::isBaseType( fullName ) )
        return TypeInfo::getBaseTypeInfo( fullName, ptrSize() )->getSize();

    std::wstring     moduleName;
    std::wstring     symName;

    splitSymName( fullName, moduleName, symName );

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

    return module->getSymbolSize( symName );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getSymbolOffset( const std::wstring &fullName )
{
    std::wstring     moduleName;
    std::wstring     symName;

    splitSymName( fullName, moduleName, symName );

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

    return module->getSymbolVa( symName );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring findSymbol( MEMOFFSET_64 offset, MEMDISPLACEMENT &displacement )
{
    ModulePtr  module = loadModule(offset);
    return module->findSymbol( offset, displacement );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring findSymbol( MEMOFFSET_64 offset)
{
    MEMDISPLACEMENT  displacement;
    return findSymbol( offset, displacement );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr loadType( const std::wstring &typeName )
{
    std::wstring     moduleName;
    std::wstring     symName;

    if ( typeName.empty() )
        throw TypeException(L"type name is empty");

    if ( TypeInfo::isBaseType( typeName ) )
        return TypeInfo::getBaseTypeInfo( typeName );

    splitSymName( typeName, moduleName, symName );

    if ( moduleName.empty() )
    {
        return TypeInfo::getTypeInfoFromCache(symName);
    }

    ModulePtr  module = loadModule(moduleName);

    SymbolPtr  symbolScope = module->getSymbolScope();

    return loadType(symbolScope, symName );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr loadType( const SymbolPtr &symbolScope, const std::wstring &symbolName )
{
    SymbolPtr  symbol;

    if ( symbolName.empty() )
        throw SymbolException(L"symbol name is empty");

    try {
        symbol  = symbolScope->getChildByName( symbolName );
        return loadType(symbol);
    } 
    catch(SymbolException& )
    {}

    if ( TypeInfo::isComplexType( symbolName ) )
        return TypeInfo::getComplexTypeInfo( symbolName, symbolScope );

    std::wstringstream  sstr;
    sstr << L'\'' << symbolName << L'\'' << L" - symbol not found";
    throw SymbolException(sstr.str());
}


///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr loadType( const SymbolPtr &symbol )
{
    unsigned long symTag = symbol->getSymTag();
    TypeInfoPtr  ptr;

    switch( symTag )
    {
    case SymTagData:

        if ( symbol->getLocType() == LocIsBitField )
        {
            ptr = TypeInfoPtr( new TypeInfoSymbolBitField(symbol) );
            break;
        }

        if ( symbol->getDataKind() == DataIsConstant )
        {
            NumVariant     constVal;
            symbol->getValue( constVal );  
            
            ptr = loadType( symbol->getType() );

            dynamic_cast<TypeInfoImp*>( ptr.get() )->setConstant( constVal );

            break;
        }

       return loadType( symbol->getType() );

    case SymTagBaseType:
        return TypeInfo::getBaseTypeInfo( symbol );

    case SymTagUDT:
    case SymTagBaseClass:
       return TypeInfoPtr( new TypeInfoUdt( symbol ) );

    case SymTagArrayType:
       return TypeInfoPtr( new TypeInfoSymbolArray( symbol ) );

    case SymTagPointerType:
        return TypeInfoPtr( new TypeInfoSymbolPointer( symbol ) );

    case SymTagVTable:
        ptr = TypeInfoPtr( new TypeInfoSymbolPointer( symbol->getType() ) );
        break;

    case SymTagVTableShape:
        ptr = TypeInfoPtr( new TypeInfoVtbl( symbol ) );
        break;

    case SymTagEnum:
        ptr = TypeInfoPtr( new TypeInfoEnum( symbol ) );
        break;

    case SymTagFunction:
        ptr = TypeInfoPtr( new TypeInfoSymbolFunction( symbol ) );
        break;

    case SymTagFunctionType:
        ptr = TypeInfoPtr( new TypeInfoSymbolFunctionPrototype( symbol ) );
        break;

    case SymTagTypedef:
        ptr = loadType( symbol->getType() );
        break;

    case SymTagFunctionArgType:
        ptr = loadType( symbol->getType() );
        break;

    case SymTagInlineSite:
        ptr = loadType(symbol->getType());
        break;

    default:
        throw TypeException( L"",  L"this type is not supported" );
    }

    //if ( ptr )
    //    ptr->m_ptrSize = getTypePointerSize(typeSym);

    return ptr;
}

///////////////////////////////////////////////////////////////////////////////

std::wstring  callingConventionAsStr(kdlib::CallingConventionType  calltype)
{
    using namespace kdlib;

    switch (calltype)
    {
    case CallConv_NearC:
        return L"__cdecl";
        break;

    case CallConv_NearFast:
        return L"__fastcall";
        break;

    case CallConv_NearStd:
        return  L"__stdcall";
        break;

    case CallConv_ThisCall:
        return L"__thiscall";
        break;

    case CallConv_ClrCall:
        return L"__clrcall";
        break;

    }

    throw TypeException(L"unknown calling convention");
}

///////////////////////////////////////////////////////////////////////////////

std::wstring  getMethodPrototype( kdlib::TypeInfoPtr&  methodType )
{
    using namespace kdlib;

    std::wstringstream  sstr;

    std::wstring  returnType = methodType->getReturnType()->getName();
  
    CallingConventionType ccType = methodType->getCallingConvention();
    if (ccType == CallConv_NearC && methodType->hasThis())
        ccType = CallConv_ThisCall;

    std::wstring  callType = callingConventionAsStr(ccType);

    std::wstringstream  sargs;

    for ( size_t i = CallConv_ThisCall == ccType && methodType->hasThis() ? 1 : 0; i < methodType->getElementCount(); ++i )
    {
        if ( !sargs.str().empty() )
           sargs << L',';

        TypeInfoPtr argType = methodType->getElement(i);
        if (argType->isNoType())
        {
            // Variadic function
            sargs << "...";
        }
        else
        {
            sargs << argType->getName();
        }
    }

    sstr << returnType << L'(' << callType << ')' << L'(' << sargs.str() << L')';

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

static const std::wregex  prototypeMatch1(L"(\\w+)\\s*\\(\\s*(\\w+)\\s*\\)\\s*\\((.*)\\)");
static const std::wregex  prototypeMatch2(L"(\\w+)\\s*\\(\\s*(\\w+)\\s+(\\w+)::\\s*\\)\\s*\\((.*)\\)");

bool isPrototypeMatch(TypeInfoPtr&  methodType, const std::wstring& methodPrototype)
{
    std::wsmatch    matchResult;

    std::wstring  returnType;
    std::wstring  callConversion;
    std::wstring  className;
    std::wstring  args;

    if ( std::regex_match( methodPrototype, matchResult, prototypeMatch1  ) )
    {
        returnType = std::wstring(matchResult[1].first, matchResult[1].second);
        callConversion = std::wstring(matchResult[2].first, matchResult[2].second);
        args = std::wstring(matchResult[3].first, matchResult[3].second);
    }
    else
    if ( std::regex_match( methodPrototype, matchResult, prototypeMatch2  ) )
    {
        returnType = std::wstring(matchResult[1].first, matchResult[1].second);
        callConversion = std::wstring(matchResult[2].first, matchResult[2].second);
        className = std::wstring(matchResult[3].first, matchResult[3].second);
        args= std::wstring (matchResult[4].first, matchResult[4].second);
    }
    else
        return false;
 
    if ( returnType != methodType->getReturnType()->getName() )
        return false;

    CallingConventionType ccType = methodType->getCallingConvention();
    if (ccType == CallConv_NearC && methodType->hasThis())
        ccType = CallConv_ThisCall;

    if ( callConversion != callingConventionAsStr(ccType) )
        return false;

    if ( !className.empty() && className != methodType->getClassParent()->getName() )
        return false;

    args.erase(std::remove_if(args.begin(), args.end(), [](const auto& c) { return std::isspace(c, std::locale()); }), args.end());

    std::wstringstream  sargs;

    for ( size_t i = CallConv_ThisCall == ccType && methodType->hasThis() ? 1 : 0; i < methodType->getElementCount(); ++i )
    {
        if ( !sargs.str().empty() )
            sargs << L',';

        TypeInfoPtr argType = methodType->getElement(i);
        if (argType->isNoType())
        {
            // Variadic function
            sargs << "...";
        }
        else
        {
            sargs << argType->getName();
        }
    }

    return args == sargs.str();
}

///////////////////////////////////////////////////////////////////////////////

std::vector<TypeFieldPtr> enumFields(
    const SymbolPtr &rootSym,
    const SymbolPtr &parentSym,
    MEMOFFSET_32 startOffset = 0,
    MEMOFFSET_32 virtualBasePtr = 0,
    size_t virtualDispIndex = 0,
    size_t virtualDispSize = 0)
{
    std::vector<TypeFieldPtr>  fields;

    bool  firstVtbl = true;

    size_t   childCount = parentSym->getChildCount();

    for (unsigned long i = 0; i < childCount; ++i)
    {
        SymbolPtr  childSym = parentSym->getChildByIndex(i);
        SymTags  symTag = childSym->getSymTag();

        if (symTag == SymTagBaseClass)
        {
            if (childSym->isVirtualBaseClass() )
            {
                if (rootSym != parentSym)
                    continue;

                auto childFields = enumFields(
                    rootSym,
                    childSym, 
                    0,
                    childSym->getVirtualBasePointerOffset(),
                    childSym->getVirtualBaseDispIndex(),
                    childSym->getVirtualBaseDispSize());

                for (const auto& childField : childFields)
                {
                    fields.push_back(childField);
                }
            }
            else
            {
                auto childFields = enumFields(
                    rootSym, 
                    childSym,
                    startOffset + childSym->getOffset(), 
                    virtualBasePtr, 
                    virtualDispIndex,
                    virtualDispSize);
            
                for (const auto& childField : childFields)
                {
                    if (childField->isVtbl() && firstVtbl)
                    {
                        firstVtbl = false;
                        auto  vtblField = SymbolUdtField::getVtblField(
                            parentSym->getVirtualTableShape(),
                            L"__VFN_table",
                            startOffset + childSym->getOffset(),
                            virtualBasePtr,
                            virtualDispIndex,
                            virtualDispSize);

                        fields.push_back(vtblField);
                    }
                    else
                    {
                        fields.push_back(childField);
                    }
                }
            }
        }
        else
        if (symTag == SymTagData)
        {
            TypeFieldPtr  fieldPtr;

            auto dataKind = childSym->getDataKind();

            switch (dataKind)
            {

            case DataIsMember:
                fieldPtr = SymbolUdtField::getField(
                    childSym,
                    childSym->getName(),
                    startOffset + childSym->getOffset(), 
                    virtualBasePtr, 
                    virtualDispIndex,
                    virtualDispSize);
                break;

            case DataIsStaticMember:
                {
                    MEMOFFSET_64  staticOffset = 0L;
                    try
                    {
                        staticOffset = childSym->getVa();
                    }
                    catch (SymbolException&)
                    {
                    }
                    fieldPtr = SymbolUdtField::getStaticField(childSym, childSym->getName(), staticOffset);
                }
                break;

            case DataIsConstant:
                fieldPtr = SymbolUdtField::getConstField(childSym, childSym->getName());
                break;

            default:
                continue;
            }

            if (rootSym != parentSym)
                fieldPtr->setMemberInherited();

            fields.push_back(fieldPtr);
        }
        else
        if (symTag == SymTagVTable)
        {
            TypeFieldPtr  fieldPtr;

            fieldPtr = SymbolUdtField::getVtblField(
                parentSym->getVirtualTableShape(), 
                L"__VFN_table", 
                startOffset + childSym->getOffset(), 
                virtualBasePtr,
                virtualDispIndex, 
                virtualDispSize);

            fields.push_back( fieldPtr );
        }
        else
        if (symTag == SymTagEnum)
        {
            auto childFields = enumFields(rootSym, childSym, startOffset);
            for (const auto& childField : childFields)
                fields.push_back(childField);
        }
    }

    return fields;
}

///////////////////////////////////////////////////////////////////////////////

static const std::wregex baseMatch(L"^(Char)|(WChar)|(Int1B)|(UInt1B)|(Int2B)|(UInt2B)|(Int4B)|(UInt4B)|(Int8B)|(UInt8B)|(Long)|(ULong)|(Float)|(Bool)|(Double)|(Void)|(Hresult)|(NoType)$" );

bool TypeInfo::isBaseType( const std::wstring &typeName )
{
    std::wstring  name = typeName;

    if ( isComplexType(name) )
    {
        name = getTypeNameFromComplex(name);
        if (name.empty())
            return false;
    }

    std::wsmatch    baseMatchResult;
    return std::regex_match( name, baseMatchResult, baseMatch );
}

///////////////////////////////////////////////////////////////////////////////

bool TypeInfo::isComplexType( const std::wstring &typeName )
{
    return typeName.find_first_of( L"*[" ) !=  std::string::npos;
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfo::getBaseTypeInfo( const std::wstring &typeName, size_t ptrSize )
{
    if ( isComplexType( typeName ) )
        return getComplexTypeInfo( typeName, SymbolPtr() );

    std::wsmatch    baseMatchResult;

    if ( std::regex_match( typeName, baseMatchResult, baseMatch ) )
    {
        if ( baseMatchResult[1].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<char>(L"Char", ptrSize) );

        if ( baseMatchResult[2].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<wchar_t>(L"WChar", ptrSize) );

        if ( baseMatchResult[3].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<char>(L"Int1B", ptrSize ) );

        if ( baseMatchResult[4].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<unsigned char>(L"UInt1B", ptrSize ) );

        if ( baseMatchResult[5].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<short>(L"Int2B", ptrSize) );

        if ( baseMatchResult[6].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<unsigned short>(L"UInt2B", ptrSize) );

        if ( baseMatchResult[7].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<long>(L"Int4B", ptrSize) );

        if ( baseMatchResult[8].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<unsigned long>(L"UInt4B", ptrSize) );

        if ( baseMatchResult[9].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<__int64>(L"Int8B", ptrSize) );

        if ( baseMatchResult[10].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<unsigned __int64>(L"UInt8B", ptrSize) );

        if ( baseMatchResult[11].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<long>(L"Long", ptrSize) );

        if ( baseMatchResult[12].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<unsigned long>(L"ULong", ptrSize) );

        if ( baseMatchResult[13].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<float>(L"Float", ptrSize) );

        if ( baseMatchResult[14].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<bool>(L"Bool", ptrSize) );

        if ( baseMatchResult[15].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<double>(L"Double", ptrSize) );

        if ( baseMatchResult[16].matched )
            return TypeInfoPtr( new TypeInfoVoid( ptrSize ) );

        if ( baseMatchResult[17].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<unsigned long>(L"Hresult", ptrSize) );

        if ( baseMatchResult[18].matched )
            return TypeInfoPtr( new TypeInfoNoType() );
    }

    NOT_IMPLEMENTED();

    //return TypeInfoPtr();
}

/////////////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfo::getBaseTypeInfo( const SymbolPtr &symbol )  
{
    std::wstring  symName = getBasicTypeName( symbol->getBaseType() );

    if ( symName == L"Int" || symName == L"UInt" ) 
    {
        std::wstringstream   sstr;
        sstr << symName << symbol->getSize() << L"B";
        symName = sstr.str();
    }
    else if ( symName == L"Long" )
    {
        symName = L"Int4B";
    }
    else if ( symName == L"ULong" )
    {
        symName = L"UInt4B";
    }
    else if ( symName == L"Float" && symbol->getSize() == 8  )
    {
        symName = L"Double";
    }
    else if (symName == L"Char16")
    {
        symName = L"UInt2B";
    }
    else if (symName == L"Char32")
    {
        symName = L"UInt4B";
    }

    return getBaseTypeInfo( symName, getPtrSizeBySymbol(symbol) );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfo::getComplexTypeInfo( const std::wstring &typeName, const SymbolPtr &symbolScope )
{
    std::wstring  innerSymName = getTypeNameFromComplex(typeName);
    if ( innerSymName.empty() )
        throw TypeException( typeName, L"invalid type name" );

    std::wstring symSuffix = getTypeSuffixFromComplex(typeName);
    if ( symSuffix.empty() )
        throw TypeException( typeName, L"invalid type name" );

    size_t  pointerSize;
    if ( symbolScope )
        pointerSize = getPointerSizeByMachine(symbolScope);
    else
        pointerSize = ptrSize();

    if ( isBaseType(innerSymName) )
    {
        TypeInfoPtr    basePtr = getBaseTypeInfo( innerSymName, pointerSize );
        return getRecursiveComplexType( typeName, basePtr, symSuffix, pointerSize );
    }

    SymbolPtr lowestSymbol = symbolScope->getChildByName( innerSymName );

    if ( lowestSymbol->getSymTag() == SymTagData )
    {
        throw TypeException( typeName, L"symbol name can not be an expresion" );
    }

    TypeInfoPtr lowestType = loadType( lowestSymbol );
   
    return getRecursiveComplexType( typeName, lowestType, symSuffix, pointerSize );

}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfo::getRecursiveComplexType( const std::wstring &typeName, TypeInfoPtr &lowestType, std::wstring &suffix,  size_t ptrSize  )
{
    std::wstring bracketExpr;
    getBracketExpression( suffix, bracketExpr );

    while( !suffix.empty() )
    {
        if ( getPtrExpression( suffix ) )
        {
            lowestType = TypeInfoPtr( new TypeInfoPointer( lowestType, ptrSize ) );
            continue;
        }

        size_t arraySize;
        if ( getArrayExpression( suffix, arraySize ) )
        {
            lowestType = TypeInfoPtr( new TypeInfoArray( lowestType, arraySize ) );
            continue;
        }

        throw TypeException( typeName, L"symbol name can not be an expresion" );
    }

    if ( !bracketExpr.empty() )
        return getRecursiveComplexType( typeName, lowestType, bracketExpr, ptrSize );

    return lowestType;
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfo::getTypeInfoFromCache(const std::wstring &typeName)
{
    TypeInfoPtr  cachedType = ProcessMonitor::getTypeInfo(typeName);
    if (cachedType)
        return cachedType;

    MEMOFFSET_64 moduleOffset = findModuleBySymbol(typeName);

    TypeInfoPtr  typeInfo = loadModule(moduleOffset)->getTypeByName(typeName);;

    ProcessMonitor::insertTypeInfo(typeInfo);

    return typeInfo;
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoImp::ptrTo( size_t ptrSize )
{
    return TypeInfoPtr( new TypeInfoPointer( shared_from_this(), ptrSize ? ptrSize : getPtrSize() ) );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoImp::arrayOf( size_t size )
{
    if (isIncomplete())
        throw TypeException(getName(), L"can not make array of incomplete type");
    return TypeInfoPtr( new TypeInfoArray( shared_from_this(), size ) );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoImp::arrayOf()
{
    if (isIncomplete())
        throw TypeException(getName(), L"can not make array of incomplete type");
    return TypeInfoPtr(new TypeInfoIncompleteArray(shared_from_this()));
}

///////////////////////////////////////////////////////////////////////////////

TypedVarPtr TypeInfoImp::getVar(const DataAccessorPtr &dataSource)
{
    if ( isBase() )
        return TypedVarPtr( new TypedVarBase(shared_from_this(), dataSource) );

    if ( isUserDefined() )
        return TypedVarPtr( new TypedVarUdt( shared_from_this(), dataSource ) );

    if ( isPointer() )
        return TypedVarPtr( new TypedVarPointer( shared_from_this(), dataSource) );

    if ( isArray() )
        return TypedVarPtr( new TypedVarArray( shared_from_this(), dataSource ) );

    if ( isBitField() )
        return TypedVarPtr( new TypedVarBitField( shared_from_this(), dataSource) );

    if ( isEnum() )
        return TypedVarPtr( new TypedVarEnum( shared_from_this(), dataSource) );

    if ( isFunction() )
        return TypedVarPtr( new TypedVarFunction( shared_from_this(), dataSource) );

    if ( isVtbl() )
        return TypedVarPtr( new TypedVarVtbl( shared_from_this(), dataSource) );

    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////

bool TypeInfoImp::isTemplate()
{
    return getName().find_first_of(L'<') != std::wstring::npos;
}

///////////////////////////////////////////////////////////////////////////////

size_t TypeInfoImp::getTemplateArgsCount()
{
    return getTempalteArgs().size();
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypeInfoImp::getTemplateArg(size_t index)
{
    const auto& templateArgs = getTempalteArgs();

    if (index >= templateArgs.size())
        throw IndexException(index);

    return *std::next(templateArgs.begin(), index);
}
///////////////////////////////////////////////////////////////////////////////

namespace {

std::pair<std::wstring::const_iterator, std::wstring::const_iterator> removeSpaces(const std::wstring::const_iterator& it1, const std::wstring::const_iterator& it2)
{
    auto result = std::make_pair(it1, it2);

    while (result.first != result.second && *result.first == L' ')
    {
        result.first = std::next(result.first);
    }

    while (result.first != result.second)
    {
        auto prev = std::prev(result.second);
        if (*prev != L' ')
            break;
        result.second = prev;
    }

    return result;
}

static const std::wregex templateMatch(L"^([\\w:]*)<(.*)>$");

}

std::list<std::wstring> TypeInfoImp::getTempalteArgs()
{
    std::wsmatch    matchResult;
    const auto&  typeName = getName();
    if ( !std::regex_match(typeName, matchResult, templateMatch))
        throw TypeException(getName(), L"type is not a template");
    
    std::list<std::wstring>   argList;

    auto  current = matchResult[2].first;
    auto  argStart = current;
    auto  end = matchResult[2].second;

    int  nestedLevel = 0;

    while (true)
    {
        if (nestedLevel == 0)
        {
            if (current == end)
            {
                auto arg = removeSpaces(argStart, current);
                argList.push_back(std::wstring(arg.first, arg.second));
                break;
            }
            else
            if (*current == ',')
            {
                argList.push_back(std::wstring(argStart, current));
                current = std::next(current);
                argStart = current;
            }
            else
            if (*current == '<')
            {
                nestedLevel++;
                current = std::next(current);
            }
            else
            {
                current = std::next(current);
            }
        }
        else
        {
            if (current == end)
                break;
            else if (*current == '<')
                nestedLevel++;
            else if (*current == '>')
                nestedLevel--;
            current = std::next(current);
        }
    }  

    return argList;
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypeInfoReference::getName()
{
    std::wstring       name;
    TypeInfoPtr        typeInfo = shared_from_this();

    std::pair<std::wstring, std::wstring> tiName;
    do {

        if ( typeInfo->isArray() )
        {
            if (!name.empty())
            {
                name.insert(0, 1, L'(');
                name.insert(name.size(), 1, L')');
            }

            do
            {
                if (typeInfo->isIncomplete())
                {
                    name += L"[]";
                }
                else
                {
                    std::wstringstream  sstr;
                    sstr << L'[' << typeInfo->getElementCount() << L']';
                    name += sstr.str();
                }

                typeInfo = typeInfo->deref();

                if (!typeInfo->isArray())
                    break;

            } while (true);

            continue;
        }
        else
        if ( typeInfo->isPointer() )
        {
            name.insert( 0, 1, L'*' );

            typeInfo = typeInfo->deref();

            continue;
        }

        tiName = typeInfo->splitName();
        break;

    } while ( true );

    name.insert( 0, tiName.first );
    name += tiName.second;

    return name;
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypeInfoFields::print()
{
    std::wstringstream  sstr;

    sstr << getName() << " Size: 0x" << std::hex << getSize() << " (" << std::dec << getSize() << ")" << std::endl;
    
    size_t  fieldCount = getElementCount();

    for ( size_t i = 0; i < fieldCount; ++i )
    {
        TypeFieldPtr   udtField = m_fields.lookup(i);

        if (udtField->isConstMember())
        {
            continue;
        }
        else
        if ( udtField->isStaticMember() )
        {
            sstr << L"   =" << std::right << std::setw(10) << std::setfill(L'0') << std::hex << udtField->getStaticOffset();
            sstr << L" " << std::left << std::setw(18) << std::setfill(L' ') << udtField->getName() << L':';
        }
        else
        {
            if ( udtField->isVirtualMember() )
            {
                sstr << L"   virtual base " << udtField->getVirtualBaseClassName();
                sstr << L" +" << std::right << std::setw(4) << std::setfill(L'0') << std::hex << udtField->getOffset();
                sstr << L" " << udtField->getName() << L':';
            }
            else
            {
                sstr << L"   +" << std::right << std::setw(4) << std::setfill(L'0') << std::hex << udtField->getOffset();
                sstr << L" " << std::left << std::setw(24) << std::setfill(L' ') << udtField->getName() << L':';
            }
        }

        sstr << L" " << std::left << udtField->getTypeInfo()->getName();
        sstr << std::endl;
    }

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoFields::getElement( const std::wstring &name )
{
    checkFields();

    size_t  pos = name.find_first_of( L'.');

    TypeInfoPtr  fieldType = m_fields.lookup( std::wstring( name, 0, pos) )->getTypeInfo();

    if (pos == std::wstring::npos)
    {
        if (fieldType->isVtbl())
        {
            return fieldType->ptrTo(fieldType->getPtrSize());
        }
        return fieldType;
    }

    return fieldType->getElement( std::wstring( name, pos + 1 ) );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoFields::getElement(size_t index )
{
    checkFields();

    auto typeInfo = m_fields.lookup(index)->getTypeInfo();
    if (typeInfo->isVtbl())
    {
        return typeInfo->ptrTo(typeInfo->getPtrSize());
    }
    return typeInfo;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_REL TypeInfoFields::getElementOffset( const std::wstring &name )
{
    checkFields();

    size_t  pos = name.find_first_of( L'.');

    MEMOFFSET_REL  offset = m_fields.lookup( std::wstring( name, 0, pos) )->getOffset();

    if ( pos != std::wstring::npos )
    {
        TypeInfoPtr  fieldType = getElement( std::wstring( name, 0, pos ) );

        offset += fieldType->getElementOffset( std::wstring( name, pos + 1 ) );
    }

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_REL TypeInfoFields::getElementOffset( size_t index )
{
    checkFields();

    return m_fields.lookup( index )->getOffset();
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypeInfoFields::getElementName( size_t index )
{
    checkFields();

    return m_fields.lookup( index )->getName();
}

///////////////////////////////////////////////////////////////////////////////

size_t TypeInfoFields::getElementIndex( const std::wstring &name )
{
    checkFields();

    return m_fields.getIndex(name);
}

///////////////////////////////////////////////////////////////////////////////

size_t TypeInfoFields::getElementCount()
{
    checkFields();

    return m_fields.count();
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 TypeInfoFields::getElementVa( const std::wstring &name )
{
    checkFields();

    size_t  pos = name.find_first_of( L'.');

    TypeFieldPtr  fieldPtr = m_fields.lookup( std::wstring( name, 0, pos) );

    if ( pos == std::wstring::npos )
        return fieldPtr->getStaticOffset();

    TypeInfoPtr  fieldType = fieldPtr->getTypeInfo();

    return fieldType->getElementVa( std::wstring( name, pos + 1 ) );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 TypeInfoFields::getElementVa( size_t index )
{
   checkFields();

   return m_fields.lookup( index )->getStaticOffset();
}

///////////////////////////////////////////////////////////////////////////////

bool TypeInfoFields::isStaticMember( const std::wstring &name )
{
    checkFields();

    size_t  pos = name.find_first_of( L'.');

    TypeFieldPtr  fieldPtr = m_fields.lookup( std::wstring( name, 0, pos) );

    if ( pos == std::wstring::npos )
        return fieldPtr->isStaticMember();

    TypeInfoPtr  fieldType = fieldPtr->getTypeInfo();

    return fieldType->isStaticMember( std::wstring( name, pos + 1 ) );
}

///////////////////////////////////////////////////////////////////////////////

bool TypeInfoFields::isStaticMember( size_t index )
{
    checkFields();

    return m_fields.lookup( index )->isStaticMember();
}

///////////////////////////////////////////////////////////////////////////////

bool TypeInfoFields::isVirtualMember( const std::wstring &name )
{
    checkFields();

    size_t  pos = name.find_first_of( L'.');

    TypeFieldPtr  fieldPtr = m_fields.lookup( std::wstring( name, 0, pos) );

    if ( pos == std::wstring::npos )
        return fieldPtr->isVirtualMember();

    TypeInfoPtr  fieldType = fieldPtr->getTypeInfo();

    return fieldType->isVirtualMember( std::wstring( name, pos + 1 ) );
}

///////////////////////////////////////////////////////////////////////////////

bool TypeInfoFields::isVirtualMember( size_t index )
{ 
    checkFields();

    return m_fields.lookup( index )->isVirtualMember();
}

///////////////////////////////////////////////////////////////////////////////

bool TypeInfoFields::isConstMember(const std::wstring &name)
{
    checkFields();

    size_t  pos = name.find_first_of(L'.');

    TypeFieldPtr  fieldPtr = m_fields.lookup(std::wstring(name, 0, pos));

    if (pos == std::wstring::npos)
        return fieldPtr->isConstMember();

    TypeInfoPtr  fieldType = fieldPtr->getTypeInfo();

    return fieldType->isConstMember(std::wstring(name, pos + 1));
}

///////////////////////////////////////////////////////////////////////////////

bool TypeInfoFields::isConstMember(size_t index)
{
    checkFields();

    return m_fields.lookup(index)->isConstMember();
}

///////////////////////////////////////////////////////////////////////////////

bool TypeInfoFields::isInheritedMember(const std::wstring &name)
{
    checkFields();

    size_t  pos = name.find_first_of(L'.');

    TypeFieldPtr  fieldPtr = m_fields.lookup(std::wstring(name, 0, pos));

    if (pos == std::wstring::npos)
        return fieldPtr->isInheritedMember();

    TypeInfoPtr  fieldType = fieldPtr->getTypeInfo();

    return fieldType->isInheritedMember(std::wstring(name, pos + 1));
}
///////////////////////////////////////////////////////////////////////////////

bool TypeInfoFields::isInheritedMember(size_t index)
{
    checkFields();

    return m_fields.lookup(index)->isInheritedMember();
}

///////////////////////////////////////////////////////////////////////////////

size_t TypeInfoFields::getAlignReq()
{
    size_t alignReq = 1;
    for (size_t i = 0; i < getElementCount(); ++i)
    {
        const size_t fieldAlignReq = getElement(i)->getAlignReq();
        alignReq = (fieldAlignReq > alignReq) ? fieldAlignReq : alignReq;
    }
    return alignReq;
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoUdt::getVirtualDisplacement( const std::wstring& fieldName, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize )
{
    TypeFieldPtr  fieldPtr = m_fields.lookup( fieldName );

    if ( !fieldPtr->isVirtualMember() )
        throw TypeException( getName(), L"field is not a virtual member" );

    fieldPtr->getVirtualDisplacement( virtualBasePtr, virtualDispIndex, virtualDispSize );
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoUdt::getVirtualDisplacement( size_t fieldIndex, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize )
{
    TypeFieldPtr  fieldPtr = m_fields.lookup( fieldIndex );

    if ( !fieldPtr->isVirtualMember() )
        throw TypeException( getName(), L"field is not a virtual member" );

    fieldPtr->getVirtualDisplacement( virtualBasePtr, virtualDispIndex, virtualDispSize );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypeInfoUdt::str()
{
    TypeInfoPtr  selfPtr = shared_from_this();
    return printStructType(selfPtr);

    //return std::wstring(L"class/struct : ") + TypeInfoFields::print();
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoUdt::getClassParent()
{
    return loadType(m_symbol->getClassParent());
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr  TypeInfoUdt::getMethod( const std::wstring &name, const std::wstring&  prototype)
{

    SymbolPtrList methods = m_symbol->findChildren(SymTagFunction);

    for ( SymbolPtrList::iterator  it = methods.begin(); it != methods.end(); ++it )
    {
        SymbolPtr  method = *it;

        std::wstring  methodName = method->getName();

        if ( methodName == name )
        {
            TypeInfoPtr  methodType = loadType(method);
            if ( prototype.empty() || isPrototypeMatch( methodType, prototype ) )
                return methodType;
        }
    }

    std::wstringstream  sstr;
    sstr << getName() << " has no this method :" << name;
    throw TypeException(sstr.str() );
}

TypeInfoPtr TypeInfoUdt::getMethod(size_t index)
{
    SymbolPtrList methods = m_symbol->findChildren(SymTagFunction);

    if ( index >= methods.size() )
        throw IndexException(index);

    SymbolPtrList::iterator  it = methods.begin();
    std::advance(it, index);
    return loadType(*it);
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypeInfoUdt::getMethodName(size_t index)
{
    SymbolPtrList methods = m_symbol->findChildren(SymTagFunction);

    if (index >= methods.size())
        throw IndexException(index);

    SymbolPtrList::iterator  it = methods.begin();
    std::advance(it, index);

    return (*it)->getName();
}

///////////////////////////////////////////////////////////////////////////////

size_t TypeInfoUdt::getMethodsCount()
{
     SymbolPtrList methods = m_symbol->findChildren(SymTagFunction);
     return methods.size();
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoUdt::getBaseClass( const std::wstring& className)
{
    SymbolPtrList baseClasses = m_symbol->findChildren(SymTagBaseClass);

    for ( SymbolPtrList::iterator  it = baseClasses.begin(); it != baseClasses.end(); ++it )
    {
        SymbolPtr  baseClass = *it;

        std::wstring  baseClassName = baseClass->getName();

        if ( baseClassName == className )
            return loadType(baseClass);
    }

    std::wstringstream  sstr;
    sstr << getName() << " has no this base class : " << className;
    throw TypeException(sstr.str() );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoUdt::getBaseClass( size_t index )
{
    SymbolPtrList baseClasses = m_symbol->findChildren(SymTagBaseClass);

    if (index >= baseClasses.size() )
        throw IndexException(index);

    SymbolPtrList::iterator  it = baseClasses.begin();
    std::advance(it, index);

    return loadType(*it);
}

///////////////////////////////////////////////////////////////////////////////

size_t TypeInfoUdt::getBaseClassesCount()
{
    return m_symbol->getChildCount(SymTagBaseClass);
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_REL TypeInfoUdt::getBaseClassOffset( const std::wstring &className )
{
    SymbolPtrList baseClasses = m_symbol->findChildren(SymTagBaseClass);

    for ( SymbolPtrList::iterator  it = baseClasses.begin(); it != baseClasses.end(); ++it )
    {
        SymbolPtr  baseClass = *it;

        std::wstring  baseClassName = baseClass->getName();

        if ( baseClassName == className )
        {
            return baseClass->getOffset();
        }
    }

    std::wstringstream  sstr;
    sstr << getName() << " has no this base class : " << className;
    throw TypeException(sstr.str() );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_REL TypeInfoUdt::getBaseClassOffset( size_t index )
{
    SymbolPtrList baseClasses = m_symbol->findChildren(SymTagBaseClass);

    if (index >= baseClasses.size() )
        throw IndexException(index);

    SymbolPtrList::iterator  it = baseClasses.begin();
    std::advance(it, index);

    return (*it)->getOffset();
}

///////////////////////////////////////////////////////////////////////////////

bool TypeInfoUdt::isBaseClassVirtual( const std::wstring &className )
{
    SymbolPtrList baseClasses = m_symbol->findChildren(SymTagBaseClass);

    for ( SymbolPtrList::iterator  it = baseClasses.begin(); it != baseClasses.end(); ++it )
    {
        SymbolPtr  baseClass = *it;

        std::wstring  baseClassName = baseClass->getName();

        if ( baseClassName == className )
        {
            return baseClass->isVirtualBaseClass();
        }
    }

    std::wstringstream  sstr;
    sstr << getName() << " has no this base class : " << className;
    throw TypeException(sstr.str() );
}

///////////////////////////////////////////////////////////////////////////////

bool TypeInfoUdt::isBaseClassVirtual( size_t index )
{
    SymbolPtrList baseClasses = m_symbol->findChildren(SymTagBaseClass);

    if (index >= baseClasses.size() )
        throw IndexException(index);

    SymbolPtrList::iterator  it = baseClasses.begin();
    std::advance(it, index);

    return (*it)->isVirtualBaseClass();
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoUdt::getBaseClassVirtualDisplacement( const std::wstring &className, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize )
{
    SymbolPtrList baseClasses = m_symbol->findChildren(SymTagBaseClass);

    for ( SymbolPtrList::iterator  it = baseClasses.begin(); it != baseClasses.end(); ++it )
    {
        SymbolPtr  baseClass = *it;

        std::wstring  baseClassName = baseClass->getName();

        if ( baseClassName == className )
        {
            virtualBasePtr = baseClass->getVirtualBasePointerOffset();
            virtualDispIndex = baseClass->getVirtualBaseDispIndex();
            virtualDispSize = baseClass->getVirtualBaseDispSize();
            return;
        }
    }

    std::wstringstream  sstr;
    sstr << getName() << " has no this base class : " << className;
    throw TypeException(sstr.str() );
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoUdt::getBaseClassVirtualDisplacement( size_t index, MEMOFFSET_32 &virtualBasePtr, size_t &virtualDispIndex, size_t &virtualDispSize )
{
    SymbolPtrList baseClasses = m_symbol->findChildren(SymTagBaseClass);

    if (index >= baseClasses.size() )
        throw IndexException(index);

    SymbolPtrList::iterator  it = baseClasses.begin();
    std::advance(it, index);

    std::wstring  name = (*it)->getName();

    virtualBasePtr = (*it)->getVirtualBasePointerOffset();
    virtualDispIndex = (*it)->getVirtualBaseDispIndex();
    virtualDispSize = (*it)->getVirtualBaseDispSize();
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoUdt::getVTBL()
{
    return loadType( m_symbol->getVirtualTableShape() );
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoUdt::getFields()
{
    auto fields = kdlib::enumFields(m_symbol, m_symbol);

    for (auto& field : fields)
        m_fields.push_back(field);
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypeInfoEnum::str()
{
    std::wstringstream  sstr;

    sstr << L"enum: " << getName() << std::endl;

    size_t  fieldCount = getElementCount();

    for ( size_t i = 0; i < fieldCount; ++i )
    {
        TypeFieldPtr   enumField = m_fields.lookup(i);
        sstr << L"   " << enumField->getName();
        sstr << L" = " << enumField->getValue().asStr();
        sstr << std::endl;
    }

    return sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoEnum::getClassParent()
{
    return loadType(m_symbol->getClassParent());
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoEnum::getFields()
{
    size_t   childCount = m_symbol->getChildCount();

    for ( unsigned long  i = 0; i < childCount; ++i )
    {
        SymbolPtr  childSym = m_symbol->getChildByIndex( i );

        TypeFieldPtr  fieldPtr = TypeFieldPtr( new EnumField( childSym ) );

        m_fields.push_back( fieldPtr );
    }
}


///////////////////////////////////////////////////////////////////////////////

TypeInfoSymbolFunctionPrototype::TypeInfoSymbolFunctionPrototype( const SymbolPtr& symbol ) :
    m_symbol(symbol)
{
    // add this
    try
    {
        m_args.push_back( loadType(m_symbol->getObjectPointerType()) );
        m_hasThis = true;
    }
    catch(const SymbolException &)
    {
    }

    // add args
    SymbolPtrList lstArgs = m_symbol->findChildren(SymTagFunctionArgType);

    for ( SymbolPtrList::iterator it = lstArgs.begin(); it != lstArgs.end(); ++it)
        m_args.push_back(loadType(*it));

    m_returnType = loadType(m_symbol->getType());
    m_callconv = static_cast< CallingConventionType >( m_symbol->getCallingConvention() );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypeInfoFunctionPrototype::getName()
{
    std::pair<std::wstring, std::wstring> splittedName = splitName();
    return splittedName.first + splittedName.second;
}

///////////////////////////////////////////////////////////////////////////////

std::pair<std::wstring, std::wstring> TypeInfoFunctionPrototype::splitName()
{
    std::pair<std::wstring, std::wstring> splittedName;

    std::wstringstream sstr;

    sstr << getReturnType()->getName();
    sstr << L"(";

    CallingConventionType ccType = getCallingConvention();
    if (ccType == CallConv_NearC && hasThis())
        ccType = CallConv_ThisCall;

    switch (ccType)
    {
    case CallConv_NearC:
        sstr << L"__cdecl";
        break;

    case CallConv_NearFast:
        sstr << L"__fastcall";
        break;

    case CallConv_NearStd:
        sstr << L"__stdcall";
        break;

    case CallConv_ThisCall:
        sstr << L"__thiscall";
        break;

    case CallConv_ClrCall:
        sstr << L"__clrcall";
        break;

    default:
        sstr << L"__userpurge";
        break;
    }

    TypeInfoPtr classParent = getClassParent();
    if (classParent)
    {
        sstr << L" " << classParent->getName() << "::";
    }

    splittedName.first = sstr.str();

    std::wstringstream().swap(sstr);

    sstr << L")(";

    Args::iterator itArg = m_args.begin();
    if (CallConv_ThisCall == ccType && hasThis())
        ++itArg;

    bool bIsFirstArg = true;
    for (; itArg != m_args.end(); ++itArg)
    {
        if (bIsFirstArg)
            bIsFirstArg = false;
        else
            sstr << L", ";

        TypeInfoPtr argType = *itArg ;
        if (argType->isNoType())
        {
            // Variadic function
            sstr << "...";
            assert((itArg + 1) == m_args.end());
        }
        else
        {
            sstr << (*itArg)->getName();
        }
    }

    sstr << L")";
    splittedName.second = sstr.str();

    return splittedName;
}



///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoSymbolFunctionPrototype::getClassParent()
{
    try 
    {
        return loadType(m_symbol->getClassParent());
    }
    catch(SymbolException&)
    {}

    return TypeInfoPtr();
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoSymbolBitField::TypeInfoSymbolBitField( const SymbolPtr &symbol ) :
    TypeInfoBitField(
        TypeInfo::getBaseTypeInfo( symbol->getType() ), 
        symbol->getBitPosition(),
        static_cast<BITOFFSET>(symbol->getSize()) ),
    m_symbol(symbol)
{}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypeInfoBitField::getName() 
{
    std::wstringstream   sstr;

    sstr << m_bitType->getName()  << L":" <<  m_bitWidth;
    return  sstr.str();
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoSymbolBitField::getClassParent()
{
    return loadType(m_symbol->getClassParent());
}

///////////////////////////////////////////////////////////////////////////////

size_t TypeInfoVtbl::getElementCount()
{
    return m_symbol->getCount();
}


///////////////////////////////////////////////////////////////////////////////

size_t TypeInfoVtbl::getSize()
{
    return getPointerSizeByMachine(m_symbol) * getElementCount();
}

///////////////////////////////////////////////////////////////////////////////

//TypeInfoPtr TypeInfoPointer::getClassParent()
//{
//    if (m_symbol)
//        return loadType(m_symbol->getClassParent());
//
//    return TypeInfoImp::getClassParent();
//}

///////////////////////////////////////////////////////////////////////////////

//TypeInfoPtr TypeInfoArray::getClassParent()
//{
//    if (m_symbol)
//        return loadType(m_symbol->getClassParent());
//
//    return TypeInfoImp::getClassParent();
//}

///////////////////////////////////////////////////////////////////////////////

TypeInfoVoid::TypeInfoVoid(size_t size)
{
    m_ptrSize = size != 0 ? size : ptrSize(); 
}

///////////////////////////////////////////////////////////////////////////////


class TypeInfoBaseConst : public TypeInfoImp 
{
public:

    template<typename T>
    static TypeInfoPtr get(T& val) {
        return TypeInfoPtr( new  TypeInfoBaseConst(val) );
    }

protected:

    explicit TypeInfoBaseConst(char& val) : m_val(val), m_name(L"Int1B") {}
    explicit TypeInfoBaseConst(short& val) : m_val(val), m_name(L"Int2B"){}
    explicit TypeInfoBaseConst(long & val) : m_val(val), m_name(L"Int4B") {}
    explicit TypeInfoBaseConst(long long & val) : m_val(val), m_name(L"Int8B") {}
    explicit TypeInfoBaseConst(unsigned char & val) : m_val(val), m_name(L"UInt1B") {}
    explicit TypeInfoBaseConst(unsigned short & val) : m_val(val), m_name(L"UInt2B") {}
    explicit TypeInfoBaseConst(unsigned long & val) : m_val(val), m_name(L"UInt4B") {}
    explicit TypeInfoBaseConst(unsigned long long & val) : m_val(val), m_name(L"UInt8B") {}
    explicit TypeInfoBaseConst(int & val) : m_val(val), m_name(L"Int") {}
    explicit TypeInfoBaseConst(unsigned int & val) : m_val(val), m_name(L"UInt") {}
    explicit TypeInfoBaseConst(float & val) : m_val(val), m_name(L"Float") {}
    explicit TypeInfoBaseConst(double & val) : m_val(val), m_name(L"Double") {}
    explicit TypeInfoBaseConst(wchar_t & val) : m_val(val), m_name(L"WChar") {}
    explicit TypeInfoBaseConst(bool & val) : m_val(val), m_name(L"Bool") {}

    virtual std::wstring str() {
        return m_name;
    }

    std::wstring getName() {
        return m_name;
    }

    virtual size_t getSize() {
        return m_val.getSize();
    }

    virtual bool isBase() {
        return true;
    }

    virtual bool isConstant() {
        return true;
    }

    virtual size_t getPtrSize() {
        return getPtrSize();
    }

    virtual size_t getAlignReq() {
        return getSize();
    }

    virtual NumVariant getValue() const {
        return m_val;
    }
protected:

    NumVariant  m_val;
    std::wstring  m_name;
};


TypeInfoPtr makeCharConst(char val)
{
    return TypeInfoBaseConst::get(val);
}

TypeInfoPtr makeShortConst(short val)
{
    return TypeInfoBaseConst::get(val);
}

TypeInfoPtr makeLongConst(long val)
{
    return TypeInfoBaseConst::get(val);
}

TypeInfoPtr makeLongLongConst(long long val)
{
    return TypeInfoBaseConst::get(val);
}

TypeInfoPtr makeUCharConst(unsigned char val)
{
    return TypeInfoBaseConst::get(val);
}

TypeInfoPtr makeUShortConst(unsigned short val)
{
    return TypeInfoBaseConst::get(val);
}

TypeInfoPtr makeULongConst(unsigned long val)
{
    return TypeInfoBaseConst::get(val);
}

TypeInfoPtr makeULongLongConst(unsigned long long val)
{
    return TypeInfoBaseConst::get(val);
}

TypeInfoPtr makeIntConst(int val)
{
    return TypeInfoBaseConst::get(val);
}

TypeInfoPtr makeUIntConst(unsigned int val)
{
    return TypeInfoBaseConst::get(val);
}

TypeInfoPtr makeFloatConst(float val)
{
    return TypeInfoBaseConst::get(val);
}

TypeInfoPtr makeDoubleConst(double val)
{
    return TypeInfoBaseConst::get(val);
}

TypeInfoPtr makeWCharConst(wchar_t val)
{
    return TypeInfoBaseConst::get(val);
}

TypeInfoPtr makeBoolConst(bool val)
{
    return TypeInfoBaseConst::get(val);
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoProviderPtr  getTypeInfoProviderFromPdb( const std::wstring&  pdbFile, MEMOFFSET_64  loadBase )
{
    return TypeInfoProviderPtr( new TypeInfoSymbolProvider(pdbFile, loadBase) );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoProviderPtr  getDefaultTypeInfoProvider()
{
    return TypeInfoProviderPtr(new TypeInfoDefaultProvider());
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoSymbolProvider::TypeInfoSymbolProvider(const std::wstring&  pdbFile, MEMOFFSET_64  loadBase)
{
    m_symbolSession = loadSymbolFile(pdbFile, loadBase);
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoSymbolProvider::getTypeByName(const std::wstring& name)
{
   return loadType( m_symbolSession->getSymbolScope(), name);
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoEnumeratorPtr TypeInfoSymbolProvider::getTypeEnumerator(const std::wstring& mask)
{
    return TypeInfoEnumeratorPtr( new TypeInfoSymbolEnum(m_symbolSession, mask) );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypeInfoSymbolProvider::makeTypeName(const std::wstring& typeName, const std::wstring& typeQualifier, bool isConst)
{
    std::wstringstream  wstr;

    wstr << typeName;

    if (!typeQualifier.empty())
        wstr << L' ' << typeQualifier;

    if (isConst)
        wstr << L" const ";

    return wstr.str();
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypeInfoDefaultProvider::makeTypeName(const std::wstring& typeName, const std::wstring& typeQualifier, bool isConst)
{
    std::wstringstream  wstr;

    wstr << typeName;

    if (!typeQualifier.empty())
        wstr << L' ' << typeQualifier;

    if (isConst)
        wstr << L" const ";

    return wstr.str();
}

///////////////////////////////////////////////////////////////////////////////

inline
bool isTypeTag(SymTags tag) 
{
    switch(tag)
    {
    case SymTagUDT:
    case SymTagEnum:
    case SymTagTypedef:
        return true;
    }

    return false;
}

TypeInfoSymbolEnum::TypeInfoSymbolEnum(SymbolSessionPtr& symSession, const std::wstring& mask)
{
    m_index = 0;
    SymbolPtr  symScope = symSession->getSymbolScope();
    size_t  symCount = symScope->getChildCount();
    
    for (size_t index = 0; index < symCount; index++)
    {
        SymbolPtr sym = symScope->getChildByIndex(index);

        if (!isTypeTag(sym->getSymTag()))
            continue;

        std::wstring  symName = sym->getName();

        if (mask.empty() || fnmatch(mask, symName))
        {
            m_typeList.push_back(loadType(sym));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoSymbolEnum::Next()
{
    if ( m_index < m_typeList.size() )
        return m_typeList[m_index++];
    return TypeInfoPtr();
}

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end
