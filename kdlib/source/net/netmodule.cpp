#include "stdafx.h"


#include "netmodule.h"
#include "net.h"
#include "nettype.h"

#include <cor.h>

//
//#include <boost/regex.hpp>
//
/////////////////////////////////////////////////////////////////////////////////
//
//namespace {
//
/////////////////////////////////////////////////////////////////////////////////
//
//const boost::wregex  r1(L"\\?");
//const boost::wregex  r2(L"\\*");
//const boost::wregex  r3(L"\\.");
//
//bool fnmatch( const std::wstring& pattern, const std::wstring& str)
//{
//    std::wstring mask = pattern;
//    mask = boost::regex_replace(mask, r1, L".");
//    mask = boost::regex_replace(mask, r2, L".*");
//    mask = boost::regex_replace(mask, r3, L"\\.");
//
//    boost::wsmatch  matchResult;
//    boost::wregex  regEx(mask);
//    return boost::regex_match(str, matchResult, regEx);
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//}
//
///////////////////////////////////////////////////////////////////////////////

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

ModulePtr loadNetModule( MEMOFFSET_64 offset )
{
    return ModulePtr( new NetModule(offset) );
}

///////////////////////////////////////////////////////////////////////////////

void NetModule::initCorDebugModule()
{
    if (m_corDebugModule)
        return;

    CComPtr<ICorDebugAppDomainEnum>  appDomainsEnum;
    HRESULT  hres = g_netMgr->targetProcess()->EnumerateAppDomains(&appDomainsEnum);
    if (FAILED(hres) )
        throw DbgException("Failed ICorDebugProcess::EnumerateAppDomains");

    while(true)
    {
        CComPtr<ICorDebugAppDomain>  appDomain;
        ULONG fetchedAppDomain;
        hres = appDomainsEnum->Next(1, &appDomain, &fetchedAppDomain);

        if (FAILED(hres))
            throw DbgException("Failed ICorDebugAppDomainEnum::Next");

        if (S_OK != hres)
            break;

        CComPtr<ICorDebugAssemblyEnum>  assemblyEnum;
        hres = appDomain->EnumerateAssemblies(&assemblyEnum);

        if (FAILED(hres))
            throw DbgException("Failed ICorDebugAppDomain::EnumerateAssemblies");

        while(true)
        {
            CComPtr<ICorDebugAssembly>  assembly;
            ULONG  fetchedAssembly;
            hres = assemblyEnum->Next(1, &assembly, &fetchedAssembly);

            if (FAILED(hres))
                throw DbgException("Failed ICorDebugAssemblyEnum::Next");

            if (S_OK != hres)
                break;

            std::vector<wchar_t>  assemblyNameBuf(0x100);
            ULONG32  assemblyNameLen;
            assembly->GetName(assemblyNameBuf.size(), &assemblyNameLen, &assemblyNameBuf.front());

            CComPtr<ICorDebugModuleEnum>  moduleEnum;
            hres = assembly->EnumerateModules(&moduleEnum);

            if (FAILED(hres))
                throw DbgException("Failed ICorDebugAssembly::EnumerateModules");

            while(true)
            {
                CComPtr<ICorDebugModule>  module;
                ULONG  fetchedModule;
                hres = moduleEnum->Next(1, &module, &fetchedModule);

                if (FAILED(hres))
                    throw DbgException("Failed ICorDebugModuleEnum::Next");

                if (S_OK != hres)
                    break;

                CORDB_ADDRESS  baseAddr;
                hres = module->GetBaseAddress(&baseAddr);
                if (FAILED(hres))
                    throw DbgException("Failed ICorDebugModule::GetBaseAddress");

                if ( baseAddr == getBase() )
                {
                    m_corDebugModule = module;
                    m_metaDataProvider = getMetaDataProvider(module);
                    return;
                }
            }
        }
    }

    throw DbgException("Failed to find managed module");
}

///////////////////////////////////////////////////////////////////////////////

TypeNameList NetModule::enumTypes(const std::wstring& mask)
{
    initCorDebugModule();

    return m_metaDataProvider->getTypeNames(mask);
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 NetModule::getImageBase(ICorDebugModule* module)
{
    CORDB_ADDRESS  address;
    HRESULT  hres = module->GetBaseAddress(&address);
    if ( FAILED(hres) )
        throw DbgException("Failed ICorDebugModule::GetBaseAddress");

    return address;
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr NetModule::getTypeByName( const std::wstring &typeName )
{
    //initCorDebugModule();

    throw TypeException(typeName, L"not found");

    //auto typeToken = m_metaDataProvider->getTypeTokenByName(typeName);
    //return m_metaDataProvider->getTypeByName(type);
}

///////////////////////////////////////////////////////////////////////////////

}





















//
////IXCLRDataModule*  NetModule::get()
////{
////    if ( m_xclrDataModule )
////        return m_xclrDataModule;
////
////    HRESULT  hres = g_clrDataTarget.get()->GetModuleByAddress(m_base, &m_xclrDataModule);
////
////    if ( S_OK != hres )
////        throw DbgException("failed IXCLRDataProcess::GetModuleByAddress");
////
////    return m_xclrDataModule;
////}
//
/////////////////////////////////////////////////////////////////////////////////
//
//TypeNameList NetModule::enumTypes(const std::wstring& mask)
//{
//
// /*   TypeNameList  typeNameList;
//
//    CLRDATA_ENUM  enumAppDomainHandle;
//
//    HRESULT hres = g_clrDataTarget.get()->StartEnumAppDomains(&enumAppDomainHandle);
//    if ( FAILED(hres) )
//        throw DbgException("failed IXCLRDataModule::StartEnumAppDomains");
//
//    auto  enumAppDomainDeleter = [=](CLRDATA_ENUM*){g_clrDataTarget.get()->EndEnumAppDomains(enumAppDomainHandle); };
//    std::unique_ptr<CLRDATA_ENUM, decltype(enumAppDomainDeleter)>  scopedAppDomainEnum(&enumAppDomainHandle, enumAppDomainDeleter);
//
//    do {
//
//        CComPtr<IXCLRDataAppDomain>  appDomain;
//        hres = g_clrDataTarget.get()->EnumAppDomain(&enumAppDomainHandle, &appDomain);
//
//        if ( hres != S_OK )
//            break;
//
//        CLRDATA_ENUM  enumTypeHandle;
//
//        hres = get()->StartEnumTypeInstances(appDomain, &enumTypeHandle);
//        if ( FAILED(hres) )
//            throw DbgException("failed IXCLRDataModule::StartEnumTypeInstances");
//
//
//        auto  enumTypeDeleter = [=](CLRDATA_ENUM*){get()->EndEnumTypeInstances(enumTypeHandle);};
//        std::unique_ptr<CLRDATA_ENUM, decltype(enumTypeDeleter)>  scopedTypeEnum(&enumTypeHandle, enumTypeDeleter);
//
//        do {
//
//            CComPtr<IXCLRDataTypeInstance>  typeInstance;
//            hres = get()->EnumTypeInstance(&enumTypeHandle, &typeInstance);
//
//            if ( hres != S_OK )
//                break;
//
//            std::wstring  typeName;
//            hres = clrGetName(static_cast<IXCLRDataTypeInstance*>(typeInstance), &IXCLRDataTypeInstance::GetName, typeName);
//
//            if ( SUCCEEDED(hres) && fnmatch(mask, typeName) )
//            {
//                typeNameList.push_back(typeName);
//            }
//
//        } while(true);
//
//    } while(true);
//
//    return typeNameList;*/
//
//    /*CLRDATA_ENUM   typeEnum;
//
//    enumAssemblies();
//    enumAppDomain();
//
//    HRESULT hres = get()->StartEnumTypeDefinitions(&typeEnum);
//    if ( FAILED(hres) )
//        throw DbgException("failed IXCLRDataModule::StartEnumTypeDefinitions");
//
//    auto del = [=](CLRDATA_ENUM*){get()->EndEnumTypeDefinitions(typeEnum); };
//    std::unique_ptr<CLRDATA_ENUM, decltype(del)>  scopedTypeEnum(&typeEnum, del);
//
//    TypeNameList  typeNameList;
//
//    do {
//
//        CComPtr<IXCLRDataTypeDefinition>  typeDef;
//        hres = get()->EnumTypeDefinition(&typeEnum, &typeDef);
//        if (S_OK != hres )
//            break;
//
//        std::wstring  typeName;
//        hres = clrGetName(static_cast<IXCLRDataTypeDefinition*>(typeDef), &IXCLRDataTypeDefinition::GetName, typeName);
//        if ( SUCCEEDED(hres) && fnmatch(mask, typeName) )
//        {
//            typeNameList.push_back(typeName);
//        }
//
//    } while(true);
//
//    return typeNameList;
//    */
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
///*
//void NetModule::enumAssemblies()
//{
//    CLRDATA_ENUM  enumHandle;
//
//    HRESULT hres = get()->StartEnumAssemblies(&enumHandle);
//
//    if ( FAILED(hres) )
//        throw DbgException("failed IXCLRDataModule::StartEnumAssemblies");
//
//    auto  enumDeleter = [=](CLRDATA_ENUM*){get()->EndEnumTypeDefinitions(enumHandle); };
//    std::unique_ptr<CLRDATA_ENUM, decltype(enumDeleter)>  scopedTypeEnum(&enumHandle, enumDeleter);
//
//    do {
//
//        CComPtr<IXCLRDataAssembly>  dataAssembly;
//        hres = get()->EnumAssembly(&enumHandle, &dataAssembly);
//
//        if (S_OK != hres )
//            break;
//
//        std::wstring  name;
//        hres = clrGetName(static_cast<IXCLRDataAssembly*>(dataAssembly), &IXCLRDataAssembly::GetName, name);
//
//        std::wstring  displayName;
//        hres = clrGetName(static_cast<IXCLRDataAssembly*>(dataAssembly), &IXCLRDataAssembly::GetDisplayName, displayName);
//
//        std::wstring  fileName;
//        hres = clrGetName(static_cast<IXCLRDataAssembly*>(dataAssembly), &IXCLRDataAssembly::GetFileName, fileName);
//
//
//        auto a = 0;
//
//    } while(true);
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void NetModule::enumAppDomain()
//{
//    CLRDATA_ENUM  enumHandle;
//
//    HRESULT hres = g_clrDataTarget.get()->StartEnumAppDomains(&enumHandle);
//
//    if ( FAILED(hres) )
//        throw DbgException("failed IXCLRDataModule::StartEnumAppDomains");
//
//    auto  enumDeleter = [=](CLRDATA_ENUM*){get()->EndEnumAppDomains(enumHandle); };
//    std::unique_ptr<CLRDATA_ENUM, decltype(enumDeleter)>  scopedTypeEnum(&enumHandle, enumDeleter);
//
//    do {
//
//        CComPtr<IXCLRDataAppDomain>  appDomain;
//        hres = g_clrDataTarget.get()->EnumAppDomain(&enumHandle, &appDomain);
//
//        if ( hres != S_OK )
//            break;
//
//        std::wstring  name;
//        clrGetName(static_cast<IXCLRDataAppDomain*>(appDomain), &IXCLRDataAppDomain::GetName, name);
//
//        auto a = 0;
//
//    } while(true);
//}
//
/////////////////////////////////////////////////////////////////////////////////*/
//
//}
