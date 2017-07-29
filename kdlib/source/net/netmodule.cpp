#include "stdafx.h"

#include "netmodule.h"


namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

ModulePtr loadNetModule( MEMOFFSET_64 offset )
{
    return ModulePtr( new NetModule(offset) );
}

///////////////////////////////////////////////////////////////////////////////

IXCLRDataModule*  NetModule::get()
{
    if ( m_xclrDataModule )
        return m_xclrDataModule;

    HRESULT  hres = g_clrDataTarget.get()->GetModuleByAddress(m_base, &m_xclrDataModule);

    if ( FAILED(hres) )
        throw DbgException("failed IXCLRDataProcess::GetModuleByAddress");

    return m_xclrDataModule;
}

///////////////////////////////////////////////////////////////////////////////

TypeNameList NetModule::enumTypes(const std::wstring& mask)
{
    CLRDATA_ENUM   typeEnum;

    HRESULT hres = get()->StartEnumTypeDefinitions(&typeEnum);
    if ( FAILED(hres) )
        throw DbgException("failed IXCLRDataModule::StartEnumTypeDefinitions");

    auto del = [=](CLRDATA_ENUM*){get()->EndEnumTypeDefinitions(typeEnum); };
    std::unique_ptr<CLRDATA_ENUM, decltype(del)>  scopedTypeEnum(&typeEnum, del);

    TypeNameList  typeNameList;

    do {

        CComPtr<IXCLRDataTypeDefinition>  typeDef;
        hres = get()->EnumTypeDefinition(&typeEnum, &typeDef);

        if (S_OK != hres )
            break;

        std::wstring  typeName;
        hres = clrGetName<IXCLRDataTypeDefinition>(typeDef, typeName);
        if ( SUCCEEDED(hres) )
            typeNameList.push_back(typeName);

    } while(true);

    return typeNameList;
}

///////////////////////////////////////////////////////////////////////////////

}
