#include "stdafx.h"

#include <metahost.h>  

#include <boost/thread/recursive_mutex.hpp>

#include "kdlib/exceptions.h"
#include "kdlib/process.h"
#include "kdlib/eventhandler.h"

#include "win/dbgmgr.h"

#include "net.h"

#pragma comment(lib, "mscoree.lib") 
#pragma comment(lib, "corguids.lib")

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class ClrDebugManagerImpl : 
    public ICorDebugDataTarget,
    public ICLRDebuggingLibraryProvider,
    public ClrDebugManager,
    public EventHandler

{
public: 

    ClrDebugManagerImpl();
   
    ICorDebugProcess* targetProcess();

    //void initCLRDebugging();

    //ICorDebugModule*  getModule(MEMOFFSET_64  offset);

    //CComPtr<ICLRDebugging>  m_debugging;

    //CComPtr<ICorDebugProcess>  m_targetProcess;
    //CComQIPtr<ICorDebugProcess5>  m_targetProcess5;

private:

    STDMETHOD_(ULONG, AddRef)() { return 1; }

    STDMETHOD_(ULONG, Release)() { return 1; }

    STDMETHOD(QueryInterface)(
        _In_ REFIID InterfaceId,
        _Out_ PVOID* Interface
        )
    {
        *Interface = NULL;

        if ( IsEqualIID(InterfaceId, IID_IUnknown) )
        {
            AddRef();
            *Interface = static_cast<ICorDebugDataTarget*>(this);
            return S_OK;
        }

        if ( IsEqualIID(InterfaceId, __uuidof(ICorDebugDataTarget) ) )
        {
            AddRef();
            *Interface = static_cast<ICorDebugDataTarget*>(this);
            return S_OK;
        }

        if ( IsEqualIID(InterfaceId, __uuidof(ICLRDebuggingLibraryProvider) ) )
        {
            AddRef();
            *Interface = static_cast<ICLRDebuggingLibraryProvider*>(this);
            return S_OK;
        }

        *Interface = NULL;

        return E_NOINTERFACE;
    }

private:

     STDMETHOD (GetPlatform)(
            /* [out] */ CorDebugPlatform *pTargetPlatform
            );
        
     STDMETHOD (ReadVirtual)(
            /* [in] */ CORDB_ADDRESS address,
            /* [length_is][size_is][out] */ BYTE *pBuffer,
            /* [in] */ ULONG32 bytesRequested,
            /* [out] */ ULONG32 *pBytesRead
            );
        
    STDMETHOD (GetThreadContext)(
            /* [in] */ DWORD dwThreadID,
            /* [in] */ ULONG32 contextFlags,
            /* [in] */ ULONG32 contextSize,
            /* [size_is][out] */ BYTE *pContext
            )
     {
          return E_NOTIMPL;
     }


private:

    STDMETHOD ( ProvideLibrary)(
            /* [in] */ const WCHAR *pwszFileName,
            /* [in] */ DWORD dwTimestamp,
            /* [in] */ DWORD dwSizeOfImage,
            /* [out] */ HMODULE *phModule
            );


private:

    virtual DebugCallbackResult onProcessStart(PROCESS_DEBUG_ID processid) ;

    virtual DebugCallbackResult onProcessExit( PROCESS_DEBUG_ID processid, ProcessExitReason  reason, unsigned long exitCode );

private:

    ICorDebugProcess*  getNetProcess();

    CComPtr<ICLRDebugging>  m_debugging;

    typedef  std::map<PROCESS_ID, CComPtr<ICorDebugProcess> >   ProcessMap;

    boost::recursive_mutex  m_processLock;
    ProcessMap  m_processMap;
};

///////////////////////////////////////////////////////////////////////////////

ClrDebugManager*  g_netMgr = 0;

void  ClrDebugManager::init()
{
    if ( g_netMgr )
        throw DbgException("Net debug manager is already initialized");

    g_netMgr = new ClrDebugManagerImpl();
}

void ClrDebugManager::deinit()
{
    if ( !g_netMgr )
        throw DbgException("Net debug manager is not initialized yet");

    delete g_netMgr;
    g_netMgr = 0;
}

///////////////////////////////////////////////////////////////////////////////

ClrDebugManagerImpl::ClrDebugManagerImpl()
{
    HRESULT  hres = CLRCreateInstance(CLSID_CLRDebugging, IID_ICLRDebugging, (LPVOID*)&m_debugging);

    if ( hres != S_OK )
        throw DbgException("CLRCreateInstance failed");
}

///////////////////////////////////////////////////////////////////////////////

ICorDebugProcess* ClrDebugManagerImpl::targetProcess()
{
    boost::recursive_mutex::scoped_lock  lock(m_processLock);

    PROCESS_ID  pid = TargetProcess::getCurrent()->getSystemId();

    auto  findIt = m_processMap.find(pid);
    if ( findIt  == m_processMap.end() )
        return NULL;

    if (!findIt->second)
        findIt->second = getNetProcess();

    return findIt->second;
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ClrDebugManagerImpl::onProcessStart(PROCESS_DEBUG_ID processid)
{
    boost::recursive_mutex::scoped_lock  lock(m_processLock);

    PROCESS_ID  pid = TargetProcess::getById(processid)->getSystemId();

    m_processMap[pid] = 0;

    return DebugCallbackNoChange;
}

///////////////////////////////////////////////////////////////////////////////

DebugCallbackResult ClrDebugManagerImpl::onProcessExit( PROCESS_DEBUG_ID processid, ProcessExitReason  reason, unsigned long exitCode )
{
    PROCESS_ID  pid = TargetProcess::getById(processid)->getSystemId();

    m_processMap.erase(pid);

    return DebugCallbackNoChange;
}

///////////////////////////////////////////////////////////////////////////////

ICorDebugProcess*  ClrDebugManagerImpl::getNetProcess()
{
    MEMOFFSET_64 clrBase = TargetProcess::getCurrent()->getModuleByName(L"clr")->getBase();

    CLR_DEBUGGING_VERSION  maxVersion = {4, 6, 0x7fff, 0x7fff};
    CLR_DEBUGGING_VERSION  currentVersion = {};
    CLR_DEBUGGING_PROCESS_FLAGS  flags;

    CComPtr<ICorDebugProcess>  targetNetProcess;

    HRESULT hres = m_debugging->OpenVirtualProcess(
        clrBase,
        (ICorDebugDataTarget*)this,
        (ICLRDebuggingLibraryProvider*)this,
        &maxVersion,
        IID_ICorDebugProcess,
        (IUnknown**)&targetNetProcess,
        &currentVersion,
        &flags
        );

    if ( hres != S_OK )
        throw DbgException("ICLRDebugging::OpenVirtualProcess failed");

    return targetNetProcess;
}

//ICorDebugModule* ClrDebugManagerImpl::getModule(MEMOFFSET_64  offset)
//{
//    //if ( !m_debugging )
//    //    initCLRDebugging();
//
//    //CComPtr<ICorDebugAppDomainEnum>  appDomainsEnum;
//    //HRESULT  hres = m_targetProcess->EnumerateAppDomains(&appDomainsEnum);
//    //if (FAILED(hres) )
//    //    throw DbgException("Failed ICorDebugProcess::EnumerateAppDomains");
//
//    NOT_IMPLEMENTED();
//}

///////////////////////////////////////////////////////////////////////////////

/*
void ClrDebugManagerImpl::initCLRDebugging()
{   
    //assert(!m_debugging);

    //HRESULT  hres = CLRCreateInstance(CLSID_CLRDebugging, IID_ICLRDebugging, (LPVOID*)&m_debugging);

    //if ( hres != S_OK )
    //    throw DbgException("CLRCreateInstance failed");

    //MEMOFFSET_64 clrBase = findModuleBase(L"clr");

    //CLR_DEBUGGING_VERSION  maxVersion = {4, 6, 0x7fff, 0x7fff};
    //CLR_DEBUGGING_VERSION  currentVersion = {};
    //CLR_DEBUGGING_PROCESS_FLAGS  flags;

    //hres = m_debugging->OpenVirtualProcess(
    //    clrBase,
    //    (ICLRDataTarget*)this,
    //    this,
    //    &maxVersion,
    //    IID_ICorDebugProcess,
    //    (IUnknown**)&m_targetProcess,
    //    &currentVersion,
    //    &flags
    //    );

    //m_targetProcess5 = m_targetProcess;

    //if ( hres != S_OK )
    //    throw DbgException("ICLRDebugging::OpenVirtualProcess failed");
}
*/


///////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE ClrDebugManagerImpl::ReadVirtual(
            /* [in] */ CORDB_ADDRESS address,
            /* [length_is][size_is][out] */ BYTE *pBuffer,
            /* [in] */ ULONG32 bytesRequested,
            /* [out] */ ULONG32 *pBytesRead
            )
{
    return g_dbgMgr->dataspace->ReadVirtual(
        address, 
        pBuffer, 
        bytesRequested,
        reinterpret_cast<PULONG>(pBytesRead) 
        );
}

///////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE ClrDebugManagerImpl::GetPlatform(
            /* [out] */ CorDebugPlatform *pTargetPlatform
            )
{
    ULONG  machineType;

    HRESULT   hres = g_dbgMgr->control->GetEffectiveProcessorType(&machineType);

    if ( FAILED(hres) )
        return hres;

    switch( machineType )
    {

    case IMAGE_FILE_MACHINE_I386:
        *pTargetPlatform =  CORDB_PLATFORM_WINDOWS_X86;
        return S_OK;

    case IMAGE_FILE_MACHINE_AMD64:
        *pTargetPlatform =  CORDB_PLATFORM_WINDOWS_AMD64;
        return S_OK;
    }

    return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE ClrDebugManagerImpl::ProvideLibrary(
        /* [in] */ const WCHAR *pwszFileName,
        /* [in] */ DWORD dwTimestamp,
        /* [in] */ DWORD dwSizeOfImage,
        /* [out] */ HMODULE *phModule
        )
{
    MEMOFFSET_64 clrBase = findModuleBase(L"clr");

    std::wstring clrImageName = getModuleImageName(clrBase);

    std::wstring clrDir = clrImageName.substr(0, clrImageName.find_last_of(L'\\'));

    std::wstring path = clrDir + L'\\';
    path += pwszFileName;

    *phModule = LoadLibraryW(path.c_str());

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

}
































//#include <memory>
//
//#include <Windows.h>
//
//#include <clrdata.h>
//#include <atlbase.h>
//
//#include "kdlib/exceptions.h"
//#include "kdlib/dbgengine.h"
//#include "kdlib/memaccess.h"
//
//#include "net.h"
//#include "win/dbgmgr.h"
//#include "netobject.h"
//
//
//
//struct DacpGcHeapData
//{
//    BOOL bServerMode;
//    BOOL bGcStructuresValid;
//    UINT HeapCount;
//    UINT g_max_generation;
//};
//
//
//struct DacpGenerationData
//{    
//    CLRDATA_ADDRESS start_segment;
//    CLRDATA_ADDRESS allocation_start;
//
//    // These are examined only for generation 0, otherwise NULL
//    CLRDATA_ADDRESS allocContextPtr;
//    CLRDATA_ADDRESS allocContextLimit;
//};
//
//#define max_generation 4
//#define DAC_NUMBERGENERATIONS 4
//
//struct DacpGcHeapDetails
//{
//    CLRDATA_ADDRESS heapAddr; // Only filled in in server mode, otherwise NULL
//    CLRDATA_ADDRESS alloc_allocated;
//
//    CLRDATA_ADDRESS mark_array;
//    CLRDATA_ADDRESS current_c_gc_state;
//    CLRDATA_ADDRESS next_sweep_obj;
//    CLRDATA_ADDRESS saved_sweep_ephemeral_seg;
//    CLRDATA_ADDRESS saved_sweep_ephemeral_start;
//    CLRDATA_ADDRESS background_saved_lowest_address;
//    CLRDATA_ADDRESS background_saved_highest_address;
//
//    DacpGenerationData generation_table [DAC_NUMBERGENERATIONS]; 
//    CLRDATA_ADDRESS ephemeral_heap_segment;        
//    CLRDATA_ADDRESS finalization_fill_pointers [DAC_NUMBERGENERATIONS + 3];
//    CLRDATA_ADDRESS lowest_address;
//    CLRDATA_ADDRESS highest_address;
//    CLRDATA_ADDRESS card_table;
//};
//
//struct DacpHeapSegmentData
//{
//    CLRDATA_ADDRESS segmentAddr;
//    CLRDATA_ADDRESS allocated;
//    CLRDATA_ADDRESS committed;
//    CLRDATA_ADDRESS reserved;
//    CLRDATA_ADDRESS used;
//    CLRDATA_ADDRESS mem;
//    // pass this to request if non-null to get the next segments.
//    CLRDATA_ADDRESS next;
//    CLRDATA_ADDRESS gc_heap; // only filled in in server mode, otherwise NULL
//    // computed field: if this is the ephemeral segment highMark includes the ephemeral generation
//    CLRDATA_ADDRESS highAllocMark;
//
//    size_t flags;
//    CLRDATA_ADDRESS background_allocated;
//};
//
///*
//
//    HRESULT Request(ISOSDacInterface *sos, CLRDATA_ADDRESS addr, const DacpGcHeapDetails& heap)
//    {
//        HRESULT hr = sos->GetHeapSegmentData(addr, this);
//
//        // if this is the start segment, set highAllocMark too.
//        if (SUCCEEDED(hr))
//        {
//            // TODO:  This needs to be put on the Dac side.
//            if (this->segmentAddr == heap.generation_table[0].start_segment)
//                highAllocMark = heap.alloc_allocated;
//            else
//                highAllocMark = allocated;
//        }    
//        return hr;
//    }
//    */
//
//namespace kdlib {
//
/////////////////////////////////////////////////////////////////////////////////
//
//CLRDataTarget  g_clrDataTarget;
//
//
//CLRDataTarget::~CLRDataTarget()
//{
//    if ( m_xclrDataProcess )
//    {
//        m_xclrDataProcess->Release();
//    }
//
//    if ( m_mscordacMod )
//    {
//        FreeModule(m_mscordacMod);
//    }
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//IXCLRDataProcess* CLRDataTarget::get()
//{
//    if ( m_xclrDataProcess )
//    {
//        m_xclrDataProcess->AddRef();
//        return m_xclrDataProcess;
//    }
//
//    MEMOFFSET_64 clrBase = findModuleBase(L"clr");
//
//    std::wstring clrImageName = getModuleImageName(clrBase);
//
//    std::wstring clrDir = clrImageName.substr(0, clrImageName.find_last_of(L'\\'));
//
//    std::wstring mscordac = clrDir + L"\\mscordacwks.dll";
//
//    m_mscordacMod = LoadLibraryW(mscordac.c_str() );
//    if ( !m_mscordacMod )
//        throw DbgException("failed to load mscordacwks.dll");
//
//    PFN_CLRDataCreateInstance  pfnCLRDataCreateInstance = reinterpret_cast<PFN_CLRDataCreateInstance>(GetProcAddress(m_mscordacMod, "CLRDataCreateInstance"));
//
//    if ( !pfnCLRDataCreateInstance )
//        throw DbgException("failed to get CLRDataCreateInstance address");
//
//    HRESULT  hres = pfnCLRDataCreateInstance( __uuidof(IXCLRDataProcess), this, (void**)&m_xclrDataProcess);
//    if ( FAILED(hres) )
//        throw DbgException("failed CLRDataCreateInstance");
//
//
//    CComQIPtr<ISOSDacInterface>  sosDac = m_xclrDataProcess;
//
//    DacpGcHeapData  heapData;
//    hres = sosDac->GetGCHeapData(&heapData);
//    
//    unsigned int   needed = 0;
//    std::vector<CLRDATA_ADDRESS>  heaps(heapData.HeapCount);
//    hres = sosDac->GetGCHeapList(heaps.size(), &heaps.front(), &needed);
//
//    DacpGcHeapDetails  heapDetail = {0};
//    hres = sosDac->GetGCHeapStaticData(&heapDetail);
//
//    CLRDATA_ADDRESS currSegment = heapDetail.generation_table[2].start_segment;
//
//    BOOL visitedLOHSegment = FALSE;
//    while (currSegment != NULL)
//    {
//        DacpHeapSegmentData  heapSegment = {0};
//        hres = sosDac->GetHeapSegmentData(currSegment, &heapSegment);
//
//        std::cout << "Segment: " << std::hex << currSegment << std::endl;
//        std::cout << "=========================================" << std::endl;
//
//        //currSegment = heapSegment.next;
//        //if (currSegment == NULL && !visitedLOHSegment)
//        //{
//        //    currSegment = heapDetail.generation_table[3].start_segment;
//        //    visitedLOHSegment = TRUE;
//        //}
//
//         CLRDATA_ADDRESS  currObj = heapSegment.mem;
//
//
//
//    }
//
//    return m_xclrDataProcess;
//}
//
//
//
/////////////////////////////////////////////////////////////////////////////////
//
//HRESULT STDMETHODCALLTYPE CLRDataTarget::GetMachineType(
//        /* [out] */ ULONG32 *machineType
//        )
//{
//    return  g_dbgMgr->control->GetEffectiveProcessorType(reinterpret_cast<PULONG>(machineType));
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//HRESULT STDMETHODCALLTYPE CLRDataTarget::GetPointerSize(
//        /* [out] */ ULONG32 *pointerSize
//        )
//{
//    HRESULT  hres = g_dbgMgr->control->IsPointer64Bit();
//
//    if (FAILED(hres) )
//        return hres;
//
//    *pointerSize = hres == S_OK ? 8 : 4;
//
//    return S_OK;
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//HRESULT STDMETHODCALLTYPE CLRDataTarget::GetImageBase(
//        /* [string][in] */ LPCWSTR imagePath,
//        /* [out] */ CLRDATA_ADDRESS *baseAddress
//        )
//{
//    HRESULT  hres = S_OK;
//
//    *baseAddress = 0;
//
//    for ( ULONG index = 0; hres == S_OK; index++)
//    {
//        ULONG64  base = 0;
//        hres = g_dbgMgr->symbols->GetModuleByIndex(index, &base);
//        if (FAILED(hres))
//            return hres;
//
//        std::vector<wchar_t> buffer(1000);
//
//        hres = g_dbgMgr->symbols->GetModuleNameStringWide(
//            DEBUG_MODNAME_IMAGE,
//            index,
//            0,
//            &buffer[0],
//            static_cast<ULONG>(buffer.size()),
//            NULL );
//
//        if (FAILED(hres))
//            return hres;
//
//        std::wstring  fullImageName( &buffer[0] );
//        std::wstring  shortImageName;
//
//        auto  slashPos = fullImageName.find_last_of(L'\\');
//        if (slashPos == std::wstring::npos || slashPos == fullImageName.size() - 1)
//        {
//            shortImageName = fullImageName;
//        }
//        else
//        {
//            shortImageName = fullImageName.substr(slashPos + 1, fullImageName.size() - slashPos - 1);
//        }
//
//        if ( shortImageName == std::wstring(imagePath) )
//            return g_dbgMgr->symbols->GetModuleByIndex(index, baseAddress);
//    }
//
//    return E_FAIL;
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//HRESULT STDMETHODCALLTYPE CLRDataTarget::ReadVirtual(
//            /* [in] */ CLRDATA_ADDRESS address,
//            /* [length_is][size_is][out] */ BYTE *buffer,
//            /* [in] */ ULONG32 bytesRequested,
//            /* [out] */ ULONG32 *bytesRead
//            )
//{
//    return g_dbgMgr->dataspace->ReadVirtual(
//        address, 
//        buffer, 
//        bytesRequested,
//        reinterpret_cast<PULONG>(bytesRead) 
//        );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//TypedVarPtr  getNetAppDomain()
//{
//    CLRDATA_ENUM  enumHandle;
//
//    HRESULT hres = g_clrDataTarget.get()->StartEnumAppDomains(&enumHandle);
//
//    if ( FAILED(hres) )
//        throw DbgException("failed IXCLRDataProcess::StartEnumAppDomains");
//
//    auto  enumDeleter = [=](CLRDATA_ENUM*){g_clrDataTarget.get()->EndEnumAppDomains(enumHandle); };
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
//        CComPtr<IXCLRDataValue>  managedObj;
//        hres = appDomain->GetManagedObject(&managedObj);
//        if (FAILED(hres))
//            throw DbgException("failed IXCLRDataAppDomain::GetManagedObject");
//
//        return TypedVarPtr( new NetObject(managedObj) );
//
//    } while(true);
//
//    throw DbgException("failed to get application domain");
//}
//
//
/////////////////////////////////////////////////////////////////////////////////

