#include "stdafx.h"

#include "kdlib/dbgengine.h"
#include "kdlib/exceptions.h"

#include "cpucontextimpl.h"
#include "dbgmgr.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

CPUContextPtr CPUContext::loadCPUCurrentContext()
{
    return CPUContextPtr( new CPUContextImpl() );
}

///////////////////////////////////////////////////////////////////////////////

//CPUContextPtr CPUContext::loadCPUContextByIndex( unsigned long index )
//{
//    THREAD_DEBUG_ID  threadId = getThreadIdByIndex(index);
//
//    SwitchThreadContext  threadContext( threadId );
//
//    return CPUContextPtr( new CPUContextImpl() );
//}

///////////////////////////////////////////////////////////////////////////////

//CPUContextPtr CPUContext::loadCPUContextImplicit( MEMOFFSET_64 offset)
//{
//    SwitchThreadContext  threadContext(-1);
//
//    setImplicitThread(offset);
//
//    return CPUContextPtr( new CPUContextImpl() );
//}

///////////////////////////////////////////////////////////////////////////////

CPUContextImpl::CPUContextImpl()
{
    HRESULT  hres;

    hres = g_dbgMgr->control->GetActualProcessorType( &m_processorType );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetActualProcessorType", hres );

    if ( m_processorType == IMAGE_FILE_MACHINE_I386 )
    {
        m_ctx_x86.ContextFlags = WOW64_CONTEXT_FULL;

        hres = g_dbgMgr->advanced->GetThreadContext( &m_ctx_wow64, sizeof(m_ctx_wow64) );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugAdvanced::GetThreadContext", hres );

    }
    else
    if ( m_processorType == IMAGE_FILE_MACHINE_AMD64 )
    {
        ULONG  effproc;

        hres = g_dbgMgr->control->GetEffectiveProcessorType( &effproc );
        if ( FAILED( hres ) )
            throw DbgEngException( L"IDebugControl::GetEffectiveProcessorType", hres );

        if ( effproc == IMAGE_FILE_MACHINE_I386 )
        {
            m_wow64 = true;

            ReadWow64Context();
        }
        else
        {
            hres = g_dbgMgr->advanced->GetThreadContext( &m_ctx_amd64, sizeof(m_ctx_amd64) );
            if ( FAILED( hres ) )    
                throw DbgEngException( L"IDebugAdvanced::GetThreadContext", hres );
        }
    }
    else
    {
        throw DbgException( "unknown processor type");
    }
}

///////////////////////////////////////////////////////////////////////////////

void CPUContextImpl::ReadWow64Context()
{
    // 
    //  *** undoc ***
    // !wow64exts.r
    // http://www.woodmann.com/forum/archive/index.php/t-11162.html
    // http://www.nynaeve.net/Code/GetThreadWow64Context.cpp
    // 

    HRESULT     hres;
    ULONG       debugClass, debugQualifier;
    
    hres = g_dbgMgr->control->GetDebuggeeType( &debugClass, &debugQualifier );
    
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetDebuggeeType", hres );   
         
    ULONG64 teb64Address;

    if ( debugClass == DEBUG_CLASS_KERNEL )
    {
        DEBUG_VALUE  debugValue = {};
        ULONG        remainderIndex = 0;

        hres = g_dbgMgr->control->EvaluateWide( 
            L"@@C++(#FIELD_OFFSET(nt!_KTHREAD, Teb))",
            DEBUG_VALUE_INT64,
            &debugValue,
            &remainderIndex );
            
        if ( FAILED( hres ) )
            throw  DbgEngException( L"IDebugControl::Evaluate", hres );
            
        ULONG64 tebOffset = debugValue.I64;

        hres = g_dbgMgr->system->GetImplicitThreadDataOffset(&teb64Address);
        if (FAILED(hres) )
            throw DbgEngException( L"IDebugSystemObjects::GetImplicitThreadDataOffset", hres);

        ULONG readedBytes;

        hres = g_dbgMgr->dataspace->ReadVirtual( 
            teb64Address + tebOffset,
            &teb64Address,
            sizeof(teb64Address),
            &readedBytes);

            if (FAILED(hres) )
            throw MemoryException(teb64Address + tebOffset);
    }
    else
    {
        hres = g_dbgMgr->system->GetImplicitThreadDataOffset(&teb64Address);
        if (S_OK != hres)
            throw DbgEngException( L"IDebugSystemObjects::GetImplicitThreadDataOffset", hres);
    }

    // ? @@C++(#FIELD_OFFSET(nt!_TEB64, TlsSlots))
    // hardcoded in !wow64exts.r (6.2.8250.0)
    static const ULONG teb64ToTlsOffset = 0x01480;
    static const ULONG WOW64_TLS_CPURESERVED = 1;
    ULONG64 cpuAreaAddress;
    ULONG readedBytes;

    hres = g_dbgMgr->dataspace->ReadVirtual( 
            teb64Address + teb64ToTlsOffset + (sizeof(ULONG64) * WOW64_TLS_CPURESERVED),
            &cpuAreaAddress,
            sizeof(cpuAreaAddress),
            &readedBytes);

    if (FAILED(hres) )
        throw MemoryException(teb64Address + teb64ToTlsOffset + (sizeof(ULONG64) * WOW64_TLS_CPURESERVED));

    // CPU Area is:
    // +00 unknown ULONG
    // +04 WOW64_CONTEXT struct
    static const ULONG cpuAreaToWow64ContextOffset = sizeof(ULONG);

    hres = g_dbgMgr->dataspace->ReadVirtual(
            cpuAreaAddress + cpuAreaToWow64ContextOffset,
            &m_ctx_wow64,
            sizeof(m_ctx_wow64),
            &readedBytes);

    if (FAILED(hres) )
        throw MemoryException(cpuAreaAddress + cpuAreaToWow64ContextOffset);
}

///////////////////////////////////////////////////////////////////////////////

void* CPUContextImpl::getContext()
{
    if ( m_processorType == IMAGE_FILE_MACHINE_I386 )
    {
        return &m_ctx_x86;
    }

    if ( m_processorType == IMAGE_FILE_MACHINE_AMD64 )
    {
        return m_wow64 ? (void*)&m_ctx_wow64 : (void*)&m_ctx_amd64;
    }

    throw DbgException( "unknown processor type");
}

///////////////////////////////////////////////////////////////////////////////

unsigned long CPUContextImpl::getContextSize() 
{
    if ( m_processorType == IMAGE_FILE_MACHINE_I386 )
    {
        return sizeof(m_ctx_x86);
    }

    if ( m_processorType == IMAGE_FILE_MACHINE_AMD64 )
    {
        return m_wow64 ? sizeof(m_ctx_wow64) : sizeof(m_ctx_amd64);
    }

    throw DbgException( "unknown processor type");
}

///////////////////////////////////////////////////////////////////////////////

unsigned long CPUContextImpl::getStackLength()
{
    HRESULT  hres;

    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME> dbgFrames(filledFrames);

    unsigned long contextSize = getContextSize();

    hres = g_dbgMgr->control->GetContextStackTrace( 
        getContext(),
        contextSize,
        &dbgFrames[0],
        filledFrames,
        NULL, 
        filledFrames*contextSize,
        contextSize,
        &filledFrames );

    if (S_OK != hres)
        throw DbgEngException( L"IDebugControl4::GetContextStackTrace", hres );

    return filledFrames;
}

///////////////////////////////////////////////////////////////////////////////

void CPUContextImpl::getStackFrame( unsigned long frameIndex, MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp )
{
    HRESULT  hres;

    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME> dbgFrames(filledFrames);

    unsigned long contextSize = getContextSize();

    hres = g_dbgMgr->control->GetContextStackTrace( 
        getContext(),
        contextSize,
        &dbgFrames[0],
        filledFrames,
        NULL, 
        filledFrames*contextSize,
        contextSize,
        &filledFrames );

    if (S_OK != hres)
        throw DbgEngException( L"IDebugControl4::GetContextStackTrace", hres );


    if ( frameIndex >= filledFrames )
        throw IndexException( frameIndex );

    ip = dbgFrames[frameIndex].InstructionOffset;
    ret = dbgFrames[frameIndex].ReturnOffset;
    fp = dbgFrames[frameIndex].FrameOffset;
    sp = dbgFrames[frameIndex].StackOffset;
}

///////////////////////////////////////////////////////////////////////////////









































//
///////////////////////////////////////////////////////////////
//
//CPUContext::CPUContext( unsigned long index )
//{
//    if ( index == -1 )
//    {
//        index = getCurrentThreadId();
//    }
//    else
//    {
//        if ( index >= getNumberThreads() )
//            throw IndexException(index);
//    }
//
//    m_contextIndex = index;
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//MEMOFFSET_64 CPUContext::getIP()
//{
//    return  getInstructionOffset( m_contextIndex );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//MEMOFFSET_64 CPUContext::getSP()
//{
//     return  getStackOffset( m_contextIndex );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//MEMOFFSET_64 CPUContext::getFP()
//{
//    return  getFrameOffset( m_contextIndex );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//NumVariant CPUContext::getRegisterByName( const std::wstring &name)
//{
//    unsigned long index = getRegsiterIndex( m_contextIndex, name );
//    return getRegisterByIndex( index );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//std::wstring CPUContext::getRegisterName( unsigned long index )
//{
//    return kdlib::getRegisterName( m_contextIndex, index );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//NumVariant CPUContext::getRegisterByIndex( unsigned long index )
//{
//    CPURegType  regType = getRegisterType(m_contextIndex, index);
//
//    switch( regType )
//    {
//    case  RegInt8:
//        {
//            unsigned char  val;
//            getRegisterValue( m_contextIndex, index, &val, sizeof(val) );
//            return NumVariant(val);
//        }
//        break;
//
//    case  RegInt16:
//        {
//            unsigned short  val;
//            getRegisterValue( m_contextIndex, index, &val, sizeof(val) );
//            return NumVariant(val);
//        }
//        break;
//
//    case  RegInt32:
//        {
//            unsigned long  val;
//            getRegisterValue( m_contextIndex, index, &val, sizeof(val) );
//            return NumVariant(val);
//        }
//        break;
//
//
//    case  RegInt64:
//        {
//            unsigned long long  val;
//            getRegisterValue( m_contextIndex, index, &val, sizeof(val) );
//            return NumVariant(val);
//        }
//        break;
//
//    case  RegFloat32:
//        {
//            float  val;
//            getRegisterValue( m_contextIndex, index, &val, sizeof(val) );
//            return NumVariant(val);
//        }
//        break;
//
//
//    case  RegFloat64:
//        {
//            double  val;
//            getRegisterValue( m_contextIndex, index, &val, sizeof(val) );
//            return NumVariant(val);
//        }
//        break;
//
//    case RegFloat80:
//    case RegFloat128:
//    case RegVector64:
//    case RegVector128:
//        return NumVariant();
//    }
//
//    throw DbgException( "unsupported registry type");
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//unsigned long long CPUContext::loadMSR( unsigned long msrIndex )
//{
//    return kdlib::loadMSR(m_contextIndex, msrIndex );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void CPUContext::setMSR( unsigned long msrIndex, unsigned long long value )
//{
//    return kdlib::setMSR( m_contextIndex, msrIndex, value );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//CPUType CPUContext::getCPUType()
//{
//    return kdlib::getCPUType( m_contextIndex );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//CPUType CPUContext::getCPUMode()
//{
//    return kdlib::getCPUMode( m_contextIndex );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void CPUContext::setCPUMode( CPUType mode )
//{
//    kdlib::setCPUMode( m_contextIndex, mode );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void CPUContext::switchCPUMode()
//{
//    switch ( getCPUMode() )
//    {
//    case CPU_I386:
//        setCPUMode(CPU_AMD64);
//        return;
//    
//    case CPU_AMD64:
//        setCPUMode(CPU_I386);
//        return;
//    }
//
//    DbgException( "can not switch CPU mode");
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//unsigned long CPUContext::getStackLength()
//{
//    return kdlib::getNumberFrames( m_contextIndex );
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void CPUContext::getStackFrame( unsigned long frameIndex, MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp )
//{
//    kdlib::getStackFrame( m_contextIndex, frameIndex, ip, ret, fp, sp );
//}
//
/////////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end
