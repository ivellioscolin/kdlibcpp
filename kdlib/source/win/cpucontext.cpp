#include "stdafx.h"

#include "kdlib/dbgengine.h"
#include "kdlib/exceptions.h"

#include "stackimpl.h"
#include "cpucontextimpl.h"
#include "dbgmgr.h"


namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

NumVariant getRegisterByName(const std::wstring& regName)
{
    unsigned long index = getRegsiterIndex(regName);
    return getRegisterByIndex(index);
}

///////////////////////////////////////////////////////////////////////////////

NumVariant getRegisterByIndex(unsigned long index)
{
    CPURegType  regType = getRegisterType(index);

    switch( regType )
    {
    case  RegInt8:
        {
            unsigned char  val;
            getRegisterValue( index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;

    case  RegInt16:
        {
            unsigned short  val;
            getRegisterValue( index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;

    case  RegInt32:
        {
            unsigned long  val;
            getRegisterValue( index, &val, sizeof(val) );
            return NumVariant(val);
        }
        break;


    case  RegInt64:
        {
            unsigned long long  val;
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

    case RegFloat80:
    case RegFloat128:
    case RegVector64:
    case RegVector128:
        return NumVariant();
    }

    throw DbgException( "unsupported registry type");
}

///////////////////////////////////////////////////////////////////////////////

CPUContextPtr loadCPUContext()
{
    return CPUContextPtr( new CPUContextImpl() );
}

///////////////////////////////////////////////////////////////////////////////

CPUContextImpl::CPUContextImpl()
{
    HRESULT  hres;

    hres = g_dbgMgr->control->GetActualProcessorType( (PULONG)&m_cpuType );
    
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetActualProcessorType", hres );

    hres = g_dbgMgr->control->GetEffectiveProcessorType( (PULONG)&m_cpuMode );
    if ( FAILED( hres ) )
        throw DbgEngException( L"IDebugControl::GetEffectiveProcessorType", hres );


    unsigned long  registerNumber = kdlib::getRegisterNumber();

    m_values.reserve(registerNumber);

    for ( unsigned long  i = 0; i < registerNumber; ++i)
    {
        const std::wstring name = kdlib::getRegisterName(i);
        try
        {
            m_values.push_back( std::move( std::make_pair( name, kdlib::getRegisterByIndex(i) ) ) );
        }
        catch (const DbgException &)
        {
            // some registers values are not available (for crash dump)
        }
    }

    m_ip = getInstructionOffset();
    m_sp = getStackOffset();
    m_fp = getFrameOffset();
}

///////////////////////////////////////////////////////////////////////////////

NumVariant CPUContextImpl::getRegisterByName( const std::wstring &name)
{
    for ( unsigned long  i = 0; i < m_values.size(); ++i)
    {
        if (m_values[i].first == name )
            return m_values[i].second;
    }

    throw DbgException("register not found");
}

///////////////////////////////////////////////////////////////////////////////

NumVariant CPUContextImpl::getRegisterByIndex( unsigned long index )
{
    if ( index >= m_values.size() )
        throw IndexException(index);

    return m_values[index].second;
}

///////////////////////////////////////////////////////////////////////////////

std::wstring CPUContextImpl::getRegisterName( unsigned long index )
{
    if ( index >= m_values.size() )
        throw IndexException(index);

    return m_values[index].first;
}

///////////////////////////////////////////////////////////////////////////////

StackPtr getStackAmd64()
{
    HRESULT  hres;

    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME>  frames(filledFrames);
    std::vector<CONTEXT_X64>  contexts(filledFrames);

    g_dbgMgr->setQuietNotiification(true);

    hres = 
        g_dbgMgr->control->GetContextStackTrace(
            NULL,
            0,
            &frames[0],
            filledFrames,
            &contexts[0],
            filledFrames*sizeof(CONTEXT_X64),
            sizeof(CONTEXT_X64),
            &filledFrames 
            );

    g_dbgMgr->setQuietNotiification(false);

    if (S_OK != hres)
        throw DbgEngException( L"IDebugControl::GetStackTrace", hres );


    std::vector<StackFramePtr>  stackFrames;

    for (ULONG i = 0; i < filledFrames; ++i)
    {
        stackFrames.push_back(StackFramePtr(new StackFrameImpl(
            i,
            frames[i].InstructionOffset,
            frames[i].ReturnOffset,
            frames[i].FrameOffset,
            frames[i].StackOffset,
            CPUContextPtr(new CPUContextAmd64(contexts[i])))));
    }

    return StackPtr(new StackImpl(stackFrames));
}

///////////////////////////////////////////////////////////////////////////////

void ReadWow64Context(WOW64_CONTEXT& wow64Context)
{
    // 
    //  *** undoc ***
    // !wow64exts.r
    // http://www.woodmann.com/forum/archive/index.php/t-11162.html
    // http://www.nynaeve.net/Code/GetThreadWow64Context.cpp
    // 

    HRESULT     hres;

    ULONG64 teb64Address;

    ULONG debugClass, debugQualifier;
    hres = g_dbgMgr->control->GetDebuggeeType(&debugClass, &debugQualifier);

    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::GetDebuggeeType", hres);

    if (debugClass == DEBUG_CLASS_KERNEL)
    {
        DEBUG_VALUE  debugValue = {};
        ULONG        remainderIndex = 0;

        hres =
            g_dbgMgr->control->EvaluateWide(
            L"@@C++(#FIELD_OFFSET(nt!_KTHREAD, Teb))",
            DEBUG_VALUE_INT64,
            &debugValue,
            &remainderIndex);
        if (FAILED(hres))
            throw  DbgEngException(L"IDebugControl::Evaluate", hres);

        ULONG64 tebOffset = debugValue.I64;

        hres = g_dbgMgr->system->GetImplicitThreadDataOffset(&teb64Address);
        if (S_OK != hres)
            throw DbgEngException(L"IDebugSystemObjects::GetImplicitThreadDataOffset", hres);

        ULONG readedBytes;

        hres =
            g_dbgMgr->dataspace->ReadVirtual(
            teb64Address + tebOffset,
            &teb64Address,
            sizeof(teb64Address),
            &readedBytes);
        if (FAILED(hres))
            throw MemoryException(teb64Address + tebOffset);
    }
    else
    {
        hres = g_dbgMgr->system->GetImplicitThreadDataOffset(&teb64Address);
        if (S_OK != hres)
            throw DbgEngException(L"IDebugSystemObjects::GetImplicitThreadDataOffset", hres);
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

    if (S_OK != hres)
        throw MemoryException(teb64Address + teb64ToTlsOffset + (sizeof(ULONG64) * WOW64_TLS_CPURESERVED));

    // CPU Area is:
    // +00 unknown ULONG
    // +04 WOW64_CONTEXT struct
    static const ULONG cpuAreaToWow64ContextOffset = sizeof(ULONG);

    hres =
        g_dbgMgr->dataspace->ReadVirtual(
        cpuAreaAddress + cpuAreaToWow64ContextOffset,
        &wow64Context,
        sizeof(wow64Context),
        &readedBytes);

    if (S_OK != hres)
        throw MemoryException(cpuAreaAddress + cpuAreaToWow64ContextOffset);
}

///////////////////////////////////////////////////////////////////////////////

StackPtr getStackWow64()
{
    HRESULT  hres;

    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME>  frames(filledFrames);
    std::vector<WOW64_CONTEXT>  contexts(filledFrames);

    WOW64_CONTEXT  wow64Context;
    ReadWow64Context( wow64Context);

    g_dbgMgr->setQuietNotiification(true);

    hres =
        g_dbgMgr->control->GetContextStackTrace(
        &wow64Context,
        sizeof(WOW64_CONTEXT),
        &frames[0],
        filledFrames,
        &contexts[0],
        filledFrames*sizeof(WOW64_CONTEXT),
        sizeof(WOW64_CONTEXT),
        &filledFrames
        );

    g_dbgMgr->setQuietNotiification(false);

    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::GetStackTrace", hres);


    std::vector<StackFramePtr>  stackFrames;

    for (ULONG i = 0; i < filledFrames; ++i)
    {
        stackFrames.push_back(StackFramePtr(new StackFrameImpl(
            i,
            frames[i].InstructionOffset,
            frames[i].ReturnOffset,
            frames[i].FrameOffset,
            frames[i].StackOffset,
            CPUContextPtr(new CPUContextWOW64(contexts[i])))));
    }

    return StackPtr(new StackImpl(stackFrames));
}

///////////////////////////////////////////////////////////////////////////////

StackPtr getStackX86()
{
    HRESULT  hres;

    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME>  frames(filledFrames);
    std::vector<CONTEXT_X86>  contexts(filledFrames);

    g_dbgMgr->setQuietNotiification(true);

    hres =
        g_dbgMgr->control->GetContextStackTrace(
        NULL,
        0,
        &frames[0],
        filledFrames,
        &contexts[0],
        filledFrames*sizeof(CONTEXT_X86),
        sizeof(CONTEXT_X86),
        &filledFrames
        );

    g_dbgMgr->setQuietNotiification(false);

    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::GetStackTrace", hres);


    std::vector<StackFramePtr>  stackFrames;

    for (ULONG i = 0; i < filledFrames; ++i)
    {
        stackFrames.push_back(StackFramePtr(new StackFrameImpl(
            i,
            frames[i].InstructionOffset,
            frames[i].ReturnOffset,
            frames[i].FrameOffset,
            frames[i].StackOffset,
            CPUContextPtr(new CPUContextI386(contexts[i])))));
    }

    return StackPtr(new StackImpl(stackFrames));
}

///////////////////////////////////////////////////////////////////////////////


StackPtr getStack()
{
    HRESULT  hres;
    
    ULONG  cpuType;
    hres = g_dbgMgr->control->GetActualProcessorType((PULONG)&cpuType);

    if (FAILED(hres))
        throw DbgEngException(L"IDebugControl::GetActualProcessorType", hres);

    ULONG  cpuMode;
    hres = g_dbgMgr->control->GetEffectiveProcessorType((PULONG)&cpuMode);
    if (FAILED(hres))
        throw DbgEngException(L"IDebugControl::GetEffectiveProcessorType", hres);

    if (cpuType == IMAGE_FILE_MACHINE_AMD64 && cpuMode == IMAGE_FILE_MACHINE_AMD64)
        return getStackAmd64();

    if (cpuType == IMAGE_FILE_MACHINE_AMD64 && cpuMode == IMAGE_FILE_MACHINE_I386)
        return getStackWow64();

    if (cpuType == IMAGE_FILE_MACHINE_I386)
        return getStackX86(); 
    
    throw DbgException("Unknown CPU type");
}

///////////////////////////////////////////////////////////////////////////////

StackFramePtr getStackFrameAmd64()
{
    HRESULT  hres;
    DEBUG_STACK_FRAME  stackFrame = {};
    CONTEXT_X64  stackContext;
    
    g_dbgMgr->setQuietNotiification(true);
    
    hres = g_dbgMgr->symbols->GetScope(NULL, &stackFrame, &stackContext, sizeof(CONTEXT_X64));
    
    g_dbgMgr->setQuietNotiification(false);
    
    if ( FAILED(hres) )
        throw DbgEngException(L"IDebugSymbols::GetScope", hres);
    
    return StackFramePtr(new StackFrameImpl(
        stackFrame.FrameNumber,
        stackFrame.InstructionOffset,
        stackFrame.ReturnOffset,
        stackFrame.FrameOffset,
        stackFrame.StackOffset,
        CPUContextPtr(new CPUContextAmd64(stackContext))));
}

///////////////////////////////////////////////////////////////////////////////

StackFramePtr getStackFrameWow64()
{
    HRESULT  hres;
    DEBUG_STACK_FRAME  stackFrame = {};
    WOW64_CONTEXT  stackContext;

    g_dbgMgr->setQuietNotiification(true);

    hres = g_dbgMgr->symbols->GetScope(NULL, &stackFrame, &stackContext, sizeof(WOW64_CONTEXT));

    g_dbgMgr->setQuietNotiification(false);

    if (FAILED(hres))
        throw DbgEngException(L"IDebugSymbols::GetScope", hres);

    return StackFramePtr(new StackFrameImpl(
        stackFrame.FrameNumber,
        stackFrame.InstructionOffset,
        stackFrame.ReturnOffset,
        stackFrame.FrameOffset,
        stackFrame.StackOffset,
        CPUContextPtr(new CPUContextWOW64(stackContext))));
}

///////////////////////////////////////////////////////////////////////////////

StackFramePtr getStackFrameX86()
{
    HRESULT  hres;
    DEBUG_STACK_FRAME  stackFrame = {};
    CONTEXT_X86  stackContext;

    g_dbgMgr->setQuietNotiification(true);

    hres = g_dbgMgr->symbols->GetScope(NULL, &stackFrame, &stackContext, sizeof(CONTEXT_X86));

    g_dbgMgr->setQuietNotiification(false);

    if (FAILED(hres))
        throw DbgEngException(L"IDebugSymbols::GetScope", hres);

    return StackFramePtr(new StackFrameImpl(
        stackFrame.FrameNumber,
        stackFrame.InstructionOffset,
        stackFrame.ReturnOffset,
        stackFrame.FrameOffset,
        stackFrame.StackOffset,
        CPUContextPtr(new CPUContextI386(stackContext))));
}

///////////////////////////////////////////////////////////////////////////////

StackFramePtr getCurrentStackFrame()
{
    HRESULT  hres;

    ULONG  cpuType;
    hres = g_dbgMgr->control->GetActualProcessorType((PULONG)&cpuType);

    if (FAILED(hres))
        throw DbgEngException(L"IDebugControl::GetActualProcessorType", hres);

    ULONG  cpuMode;
    hres = g_dbgMgr->control->GetEffectiveProcessorType((PULONG)&cpuMode);
    if (FAILED(hres))
        throw DbgEngException(L"IDebugControl::GetEffectiveProcessorType", hres);

    if (cpuType == IMAGE_FILE_MACHINE_AMD64 && cpuMode == IMAGE_FILE_MACHINE_AMD64)
        return getStackFrameAmd64();

    if (cpuType == IMAGE_FILE_MACHINE_AMD64 && cpuMode == IMAGE_FILE_MACHINE_I386)
        return getStackFrameWow64();

    if (cpuType == IMAGE_FILE_MACHINE_I386)
        return getStackFrameX86();

    throw DbgException("Unknown CPU type");
}

///////////////////////////////////////////////////////////////////////////////

unsigned long getCurrentStackFrameNumber()
{
    HRESULT  hres;

    ULONG  frameIndex;
    hres = g_dbgMgr->symbols->GetCurrentScopeFrameIndex(&frameIndex);
    if (FAILED(hres))
        throw DbgEngException(L"IDebugSymbols::GetCurrentScopeFrameIndex", hres);

    return frameIndex;
}

///////////////////////////////////////////////////////////////////////////////

void setCurrentStackFrame(StackFramePtr& stackFrame)
{
    HRESULT  hres;
    
    DEBUG_STACK_FRAME  debugStackFrame = {};
    
    debugStackFrame.InstructionOffset = stackFrame->getIP();
    debugStackFrame.ReturnOffset = stackFrame->getRET();
    debugStackFrame.FrameOffset = stackFrame->getFP();
    debugStackFrame.StackOffset = stackFrame->getSP();
    debugStackFrame.FrameNumber = stackFrame->getNumber();
    
    hres = g_dbgMgr->symbols->SetScope(0, &debugStackFrame, NULL, 0);
    if ( FAILED(hres) )
        throw DbgEngException(L"IDebugSymbols::SetScope", hres);
}

///////////////////////////////////////////////////////////////////////////////

void setCurrentStackFrameByIndex(unsigned long frameIndex)
{
    HRESULT  hres;

    hres = g_dbgMgr->symbols->SetScopeFrameByIndex(frameIndex);
    if (FAILED(hres))
        throw DbgEngException(L"IDebugSymbols::SetScopeFrameByIndex", hres);
}

///////////////////////////////////////////////////////////////////////////////

void resetCurrentStackFrame()
{
    HRESULT  hres;
    
    hres = g_dbgMgr->symbols->ResetScope();
    if ( FAILED(hres) )
        throw DbgEngException(L"IDebugSymbols::ResetScope", hres);
}


///////////////////////////////////////////////////////////////////////////////
















//CPUContextPtr CPUContext::loadCPUCurrentContext()
//{
//    return CPUContextPtr( new CPUContextImpl() );
//}
//
/////////////////////////////////////////////////////////////////////////////////

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

//CPUContextImpl::CPUContextImpl()
//{
//    //HRESULT  hres;
//
//    //hres = g_dbgMgr->control->GetActualProcessorType( &m_processorType );
//    //if ( FAILED( hres ) )
//    //    throw DbgEngException( L"IDebugControl::GetActualProcessorType", hres );
//
//
//    //hres = g_dbgMgr->control->GetEffectiveProcessorType( &m_processorMode );
//    //if ( FAILED( hres ) )
//    //    throw DbgEngException( L"IdebugControl::GetEffectiveProcessorType", hres );
//
//
//
//
//
//    //if ( m_processorType == IMAGE_FILE_MACHINE_I386 )
//    //{
//    //    hres = g_dbgMgr->advanced->GetThreadContext( &m_ctx_x86, sizeof(m_ctx_x86) );
//    //    if ( FAILED( hres ) )
//    //        throw DbgEngException( L"IDebugAdvanced::GetThreadContext", hres );
//
//    //}
//    //else
//    //if ( m_processorType == IMAGE_FILE_MACHINE_AMD64 )
//    //{
//    //    ULONG  effproc;
//
//    //    hres = g_dbgMgr->control->GetEffectiveProcessorType( &effproc );
//    //    if ( FAILED( hres ) )
//    //        throw DbgEngException( L"IDebugControl::GetEffectiveProcessorType", hres );
//
//    //    if ( effproc == IMAGE_FILE_MACHINE_I386 )
//    //    {
//    //        m_wow64 = true;
//
//    //        ReadWow64Context();
//    //    }
//    //    else
//    //    {
//    //        hres = g_dbgMgr->advanced->GetThreadContext( &m_ctx_amd64, sizeof(m_ctx_amd64) );
//    //        if ( FAILED( hres ) )    
//    //            throw DbgEngException( L"IDebugAdvanced::GetThreadContext", hres );
//    //    }
//    //}
//    //else
//    //{
//    //    throw DbgException( "unknown processor type");
//    //}
//}

///////////////////////////////////////////////////////////////////////////////

//void CPUContextImpl::ReadWow64Context()
//{
//    // 
//    //  *** undoc ***
//    // !wow64exts.r
//    // http://www.woodmann.com/forum/archive/index.php/t-11162.html
//    // http://www.nynaeve.net/Code/GetThreadWow64Context.cpp
//    // 
//
//    HRESULT     hres;
//    ULONG       debugClass, debugQualifier;
//    
//    hres = g_dbgMgr->control->GetDebuggeeType( &debugClass, &debugQualifier );
//    
//    if ( FAILED( hres ) )
//        throw DbgEngException( L"IDebugControl::GetDebuggeeType", hres );   
//         
//    ULONG64 teb64Address;
//
//    if ( debugClass == DEBUG_CLASS_KERNEL )
//    {
//        DEBUG_VALUE  debugValue = {};
//        ULONG        remainderIndex = 0;
//
//        hres = g_dbgMgr->control->EvaluateWide( 
//            L"@@C++(#FIELD_OFFSET(nt!_KTHREAD, Teb))",
//            DEBUG_VALUE_INT64,
//            &debugValue,
//            &remainderIndex );
//            
//        if ( FAILED( hres ) )
//            throw  DbgEngException( L"IDebugControl::Evaluate", hres );
//            
//        ULONG64 tebOffset = debugValue.I64;
//
//        hres = g_dbgMgr->system->GetImplicitThreadDataOffset(&teb64Address);
//        if (FAILED(hres) )
//            throw DbgEngException( L"IDebugSystemObjects::GetImplicitThreadDataOffset", hres);
//
//        ULONG readedBytes;
//
//        hres = g_dbgMgr->dataspace->ReadVirtual( 
//            teb64Address + tebOffset,
//            &teb64Address,
//            sizeof(teb64Address),
//            &readedBytes);
//
//            if (FAILED(hres) )
//            throw MemoryException(teb64Address + tebOffset);
//    }
//    else
//    {
//        hres = g_dbgMgr->system->GetImplicitThreadDataOffset(&teb64Address);
//        if (S_OK != hres)
//            throw DbgEngException( L"IDebugSystemObjects::GetImplicitThreadDataOffset", hres);
//    }
//
//    // ? @@C++(#FIELD_OFFSET(nt!_TEB64, TlsSlots))
//    // hardcoded in !wow64exts.r (6.2.8250.0)
//    static const ULONG teb64ToTlsOffset = 0x01480;
//    static const ULONG WOW64_TLS_CPURESERVED = 1;
//    ULONG64 cpuAreaAddress;
//    ULONG readedBytes;
//
//    hres = g_dbgMgr->dataspace->ReadVirtual( 
//            teb64Address + teb64ToTlsOffset + (sizeof(ULONG64) * WOW64_TLS_CPURESERVED),
//            &cpuAreaAddress,
//            sizeof(cpuAreaAddress),
//            &readedBytes);
//
//    if (FAILED(hres) )
//        throw MemoryException(teb64Address + teb64ToTlsOffset + (sizeof(ULONG64) * WOW64_TLS_CPURESERVED));
//
//    // CPU Area is:
//    // +00 unknown ULONG
//    // +04 WOW64_CONTEXT struct
//    static const ULONG cpuAreaToWow64ContextOffset = sizeof(ULONG);
//
//    hres = g_dbgMgr->dataspace->ReadVirtual(
//            cpuAreaAddress + cpuAreaToWow64ContextOffset,
//            &m_ctx_wow64,
//            sizeof(m_ctx_wow64),
//            &readedBytes);
//
//    if (FAILED(hres) )
//        throw MemoryException(cpuAreaAddress + cpuAreaToWow64ContextOffset);
//}

///////////////////////////////////////////////////////////////////////////////

//void* CPUContextImpl::getContext()
//{
//    if ( m_processorType == IMAGE_FILE_MACHINE_I386 )
//    {
//        return &m_ctx_x86;
//    }
//
//    if ( m_processorType == IMAGE_FILE_MACHINE_AMD64 )
//    {
//        return m_wow64 ? (void*)&m_ctx_wow64 : (void*)&m_ctx_amd64;
//    }
//
//    throw DbgException( "unknown processor type");
//}

///////////////////////////////////////////////////////////////////////////////

//unsigned long CPUContextImpl::getContextSize() 
//{
//    if ( m_processorType == IMAGE_FILE_MACHINE_I386 )
//    {
//        return sizeof(m_ctx_x86);
//    }
//
//    if ( m_processorType == IMAGE_FILE_MACHINE_AMD64 )
//    {
//        return m_wow64 ? sizeof(m_ctx_wow64) : sizeof(m_ctx_amd64);
//    }
//
//    throw DbgException( "unknown processor type");
//}

///////////////////////////////////////////////////////////////////////////////

//unsigned long CPUContextImpl::getStackLength()
//{
//    HRESULT  hres;
//
//    ULONG   filledFrames = 1024;
//    std::vector<DEBUG_STACK_FRAME> dbgFrames(filledFrames);
//
//    unsigned long contextSize = getContextSize();
//
//    hres = g_dbgMgr->control->GetContextStackTrace( 
//        getContext(),
//        contextSize,
//        &dbgFrames[0],
//        filledFrames,
//        NULL, 
//        filledFrames*contextSize,
//        contextSize,
//        &filledFrames );
//
//    if (S_OK != hres)
//        throw DbgEngException( L"IDebugControl4::GetContextStackTrace", hres );
//
//    return filledFrames;
//}

///////////////////////////////////////////////////////////////////////////////

//void CPUContextImpl::getStackFrame( unsigned long frameIndex, MEMOFFSET_64 &ip, MEMOFFSET_64 &ret, MEMOFFSET_64 &fp, MEMOFFSET_64 &sp )
//{
//    HRESULT  hres;
//
//    ULONG   filledFrames = 1024;
//    std::vector<DEBUG_STACK_FRAME> dbgFrames(filledFrames);
//
//    unsigned long contextSize = getContextSize();
//
//    hres = g_dbgMgr->control->GetContextStackTrace( 
//        getContext(),
//        contextSize,
//        &dbgFrames[0],
//        filledFrames,
//        NULL, 
//        filledFrames*contextSize,
//        contextSize,
//        &filledFrames );
//
//    if (S_OK != hres)
//        throw DbgEngException( L"IDebugControl4::GetContextStackTrace", hres );
//
//    if ( frameIndex >= filledFrames )
//        throw IndexException( frameIndex );
//
//    ip = dbgFrames[frameIndex].InstructionOffset;
//    ret = dbgFrames[frameIndex].ReturnOffset;
//    fp = dbgFrames[frameIndex].FrameOffset;
//    sp = dbgFrames[frameIndex].StackOffset;
//}



} // kdlib namespace end
