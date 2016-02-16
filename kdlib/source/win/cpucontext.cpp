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

CPUContextPtr CPUContext::loadCPUCurrentContext()
{
    HRESULT  hres;

    ULONG  processorType;

    hres = g_dbgMgr->control->GetActualProcessorType(&processorType);
    if (FAILED(hres))
        throw DbgEngException(L"IDebugControl::GetActualProcessorType", hres);

    if (processorType == IMAGE_FILE_MACHINE_I386)
    {
        return CPUContextPtr(new CPUx86Context());
    }

    if (processorType == IMAGE_FILE_MACHINE_AMD64)
    {
        ULONG  effproc;

        hres = g_dbgMgr->control->GetEffectiveProcessorType(&effproc);
        if (FAILED(hres))
            throw DbgEngException(L"IDebugControl::GetEffectiveProcessorType", hres);

        if (effproc == IMAGE_FILE_MACHINE_AMD64)
        {
            return CPUContextPtr(new CPUx64Context());
        }

        if (effproc == IMAGE_FILE_MACHINE_I386)
        {
            return CPUContextPtr(new CPUWOW64Context());
        }
    }

    throw DbgException("uknown processor type");
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

//////////////////////////////////////////////////////////////////////////////

CPUx86Context::CPUx86Context()
{
    HRESULT  hres;

    hres = g_dbgMgr->advanced->GetThreadContext(&m_context, sizeof(CONTEXT_X86));
    if (FAILED(hres))
        throw DbgEngException(L"IDebugAdvanced:GetThreadContext", hres);
}

///////////////////////////////////////////////////////////////////////////////

StackPtr CPUx86Context::getStack()
{
    HRESULT  hres;

    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME> dbgFrames(filledFrames);
    std::vector<CONTEXT_X86> dbgContexts(filledFrames);

    g_dbgMgr->setQuietNotiification(true);

    hres =
        g_dbgMgr->control->GetContextStackTrace(
        &m_context,
        sizeof(CONTEXT_X86),
        &dbgFrames[0],
        filledFrames,
        &dbgContexts[0],
        filledFrames*sizeof(CONTEXT_X86),
        sizeof(CONTEXT_X86),
        &filledFrames);

    g_dbgMgr->setQuietNotiification(false);

    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::GetContextStackTrace", hres);

    std::vector<StackFramePtr>  frames;

    for (ULONG i = 0; i < filledFrames; ++i)
    {
        StackFramePtr  frame = StackFramePtr(new StackFrameWithContext(
            dbgFrames[i].InstructionOffset,
            dbgFrames[i].ReturnOffset,
            dbgFrames[i].FrameOffset,
            dbgFrames[i].StackOffset,
            CPUContextPtr(new CPUx86Context(dbgContexts[i]))));

        frames.push_back(frame);
    }

    return StackPtr(new StackImpl(frames));
}

///////////////////////////////////////////////////////////////////////////////

CPUx64Context::CPUx64Context()
{
    HRESULT  hres;

    hres = g_dbgMgr->advanced->GetThreadContext(&m_context, sizeof(CONTEXT_X64));
    if (FAILED(hres))
        throw DbgEngException(L"IDebugAdvanced:GetThreadContext", hres);
}

///////////////////////////////////////////////////////////////////////////////

StackPtr CPUx64Context::getStack()
{
    HRESULT  hres;

    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME> dbgFrames(filledFrames);
    std::vector<CONTEXT_X64> dbgContexts(filledFrames);

    g_dbgMgr->setQuietNotiification(true);

    hres =
        g_dbgMgr->control->GetContextStackTrace(
        &m_context,
        sizeof(CONTEXT_X64),
        &dbgFrames[0],
        filledFrames,
        &dbgContexts[0],
        filledFrames*sizeof(CONTEXT_X64),
        sizeof(CONTEXT_X64),
        &filledFrames);

    g_dbgMgr->setQuietNotiification(false);

    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::GetContextStackTrace", hres);

    std::vector<StackFramePtr>  frames;

    for (ULONG i = 0; i < filledFrames; ++i)
    {
        StackFramePtr  frame = StackFramePtr(new StackFrameWithContext(
            dbgFrames[i].InstructionOffset,
            dbgFrames[i].ReturnOffset,
            dbgFrames[i].FrameOffset,
            dbgFrames[i].StackOffset,
            CPUContextPtr(new CPUx64Context(dbgContexts[i]))));

        frames.push_back(frame);
    }

    return StackPtr(new StackImpl(frames));
}

///////////////////////////////////////////////////////////////////////////////

CPUWOW64Context::CPUWOW64Context()
{
    // 
    //  *** undoc ***
    // !wow64exts.r
    // http://www.woodmann.com/forum/archive/index.php/t-11162.html
    // http://www.nynaeve.net/Code/GetThreadWow64Context.cpp
    // 

    HRESULT  hres;
    ULONG64  teb64Address;

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
        &m_context,
        sizeof(WOW64_CONTEXT),
        &readedBytes);

    if (S_OK != hres)
        throw MemoryException(cpuAreaAddress + cpuAreaToWow64ContextOffset);
}

///////////////////////////////////////////////////////////////////////////////

StackPtr CPUWOW64Context::getStack()
{
    HRESULT  hres;

    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME> dbgFrames(filledFrames);
    std::vector<WOW64_CONTEXT> dbgContexts(filledFrames);

    g_dbgMgr->setQuietNotiification(true);

    hres =
        g_dbgMgr->control->GetContextStackTrace(
        &m_context,
        sizeof(WOW64_CONTEXT),
        &dbgFrames[0],
        filledFrames,
        &dbgContexts[0],
        filledFrames*sizeof(WOW64_CONTEXT),
        sizeof(WOW64_CONTEXT),
        &filledFrames);

    g_dbgMgr->setQuietNotiification(false);

    if (S_OK != hres)
        throw DbgEngException(L"IDebugControl::GetContextStackTrace", hres);

    std::vector<StackFramePtr>  frames;

    for (ULONG i = 0; i < filledFrames; ++i)
    {
        StackFramePtr  frame = StackFramePtr(new StackFrameWithContext(
            dbgFrames[i].InstructionOffset,
            dbgFrames[i].ReturnOffset,
            dbgFrames[i].FrameOffset,
            dbgFrames[i].StackOffset,
            CPUContextPtr(new CPUWOW64Context(dbgContexts[i]))));

        frames.push_back(frame);
    }

    return StackPtr(new StackImpl(frames));
}

///////////////////////////////////////////////////////////////////////////////


} // kdlib namespace end
