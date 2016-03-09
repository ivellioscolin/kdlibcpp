#include "stdafx.h"

#include <cvconst.h>

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
   // std::vector<WOW64_CONTEXT>  contexts(filledFrames);

    WOW64_CONTEXT  wow64Context;
    ReadWow64Context( wow64Context);

    g_dbgMgr->setQuietNotiification(true);

    hres =
        g_dbgMgr->control->GetContextStackTrace(
            &wow64Context,
            sizeof(WOW64_CONTEXT),
            &frames[0],
            filledFrames,
            NULL, //&contexts[0],
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
            CPUContextPtr(new CPUContextWOW64(wow64Context)))));
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
    ULONG  frameNumber;

    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME>  frames(filledFrames);
    std::vector<CONTEXT_X64>  contexts(filledFrames);
    
    g_dbgMgr->setQuietNotiification(true);

    hres = g_dbgMgr->symbols->GetScope(NULL, &stackFrame, NULL, 0);

    frameNumber = stackFrame.FrameNumber;

    if (FAILED(hres))
    {
        g_dbgMgr->setQuietNotiification(false);
        throw DbgEngException(L"IDebugSymbols::GetScope", hres);
    }

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

    if (FAILED(hres))
    {
        g_dbgMgr->setQuietNotiification(false);
        throw DbgEngException(L"IDebugControl4::GetContextStackTrace", hres);
    }

    g_dbgMgr->setQuietNotiification(false);

    return StackFramePtr(new StackFrameImpl(
        frameNumber,
        frames[frameNumber].InstructionOffset,
        frames[frameNumber].ReturnOffset,
        frames[frameNumber].FrameOffset,
        frames[frameNumber].StackOffset,
        CPUContextPtr(new CPUContextAmd64(contexts[frameNumber]))));
}

///////////////////////////////////////////////////////////////////////////////

StackFramePtr getStackFrameWow64()
{
    HRESULT  hres;
    DEBUG_STACK_FRAME  stackFrame = {};
    ULONG  frameNumber;

    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME>  frames(filledFrames);
    //std::vector<WOW64_CONTEXT>  contexts(filledFrames);

    g_dbgMgr->setQuietNotiification(true);

    hres = g_dbgMgr->symbols->GetScope(NULL, &stackFrame, NULL, 0);

    frameNumber = stackFrame.FrameNumber;

    if (FAILED(hres))
    {
        g_dbgMgr->setQuietNotiification(false);
        throw DbgEngException(L"IDebugSymbols::GetScope", hres);
    }

    WOW64_CONTEXT  wow64Context;
    ReadWow64Context(wow64Context);

    hres =
        g_dbgMgr->control->GetContextStackTrace(
            &wow64Context,
            sizeof(wow64Context),
            &frames[0],
            filledFrames,
            NULL, //&contexts[0],
            filledFrames*sizeof(WOW64_CONTEXT),
            sizeof(WOW64_CONTEXT),
            &filledFrames
            );

    if (FAILED(hres))
    {
        g_dbgMgr->setQuietNotiification(false);
        throw DbgEngException(L"IDebugControl4::GetContextStackTrace", hres);
    }

    g_dbgMgr->setQuietNotiification(false);

    return StackFramePtr(new StackFrameImpl(
        frameNumber,
        frames[frameNumber].InstructionOffset,
        frames[frameNumber].ReturnOffset,
        frames[frameNumber].FrameOffset,
        frames[frameNumber].StackOffset,
        CPUContextPtr(new CPUContextWOW64(wow64Context))));
}

///////////////////////////////////////////////////////////////////////////////

StackFramePtr getStackFrameX86()
{
    HRESULT  hres;
    DEBUG_STACK_FRAME  stackFrame = {};
    ULONG  frameNumber;

    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME>  frames(filledFrames);
    std::vector<CONTEXT_X86>  contexts(filledFrames);
    
    g_dbgMgr->setQuietNotiification(true);

    hres = g_dbgMgr->symbols->GetScope(NULL, &stackFrame, NULL, 0);

    frameNumber = stackFrame.FrameNumber;

    if (FAILED(hres))
    {
        g_dbgMgr->setQuietNotiification(false);
        throw DbgEngException(L"IDebugSymbols::GetScope", hres);
    }

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

    if (FAILED(hres))
    {
        g_dbgMgr->setQuietNotiification(false);
        throw DbgEngException(L"IDebugControl4::GetContextStackTrace", hres);
    }

    g_dbgMgr->setQuietNotiification(false);

    return StackFramePtr(new StackFrameImpl(
        frameNumber,
        frames[frameNumber].InstructionOffset,
        frames[frameNumber].ReturnOffset,
        frames[frameNumber].FrameOffset,
        frames[frameNumber].StackOffset,
        CPUContextPtr(new CPUContextI386(contexts[frameNumber]))));
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

NumVariant CPUContextI386::getRegisterByIndex(unsigned long index)
{
    switch (index)
    {
    case CV_REG_AL:
        return NumVariant(static_cast<unsigned char>(m_context.Eax & 0xFF));
    case CV_REG_CL:
        return NumVariant(static_cast<unsigned char>(m_context.Ecx & 0xFF));
    case CV_REG_DL:
        return NumVariant(static_cast<unsigned char>(m_context.Edx & 0xFF));
    case CV_REG_BL:
        return NumVariant(static_cast<unsigned char>(m_context.Ebx & 0xFF));
    case CV_REG_AH:
        return NumVariant(static_cast<unsigned char>((m_context.Eax & 0xFF00) >> 8));
    case CV_REG_CH:
        return NumVariant(static_cast<unsigned char>((m_context.Ecx & 0xFF00) >> 8));
    case CV_REG_DH:
        return NumVariant(static_cast<unsigned char>((m_context.Edx & 0xFF00) >> 8));
    case CV_REG_BH:
       return NumVariant(static_cast<unsigned char>((m_context.Ebx & 0xFF00) >> 8));
    case CV_REG_AX:
        return NumVariant(static_cast<unsigned short>(m_context.Eax & 0xFFFF));
    case CV_REG_CX:
        return NumVariant(static_cast<unsigned short>(m_context.Ecx & 0xFFFF));
    case CV_REG_DX:
        return NumVariant(static_cast<unsigned short>(m_context.Edx & 0xFFFF));
    case CV_REG_BX:
        return NumVariant(static_cast<unsigned short>(m_context.Ebx & 0xFFFF));
    case CV_REG_SP:
        return NumVariant(static_cast<unsigned short>(m_context.Esp & 0xFFFF));
    case CV_REG_BP:
        return NumVariant(static_cast<unsigned short>(m_context.Ebp & 0xFFFF));
    case CV_REG_SI:
        return NumVariant(static_cast<unsigned short>(m_context.Esi & 0xFFFF));
    case CV_REG_DI:
        return NumVariant(static_cast<unsigned short>(m_context.Edi & 0xFFFF));
    case CV_REG_EAX:
        return NumVariant(m_context.Eax);
    case CV_REG_ECX:
        return NumVariant(m_context.Ecx);
    case CV_REG_EDX:
        return NumVariant(m_context.Edx);
    case CV_REG_EBX:
        return NumVariant(m_context.Ebx);
    case CV_REG_ESP:
        return NumVariant(m_context.Esp);
    case CV_REG_EBP:
        return NumVariant(m_context.Ebp);
    case CV_REG_ESI:
        return NumVariant(m_context.Esi);
    case CV_REG_EDI:
       return NumVariant(m_context.Edi);
    case CV_REG_ES:
       return NumVariant(m_context.SegEs);
    case CV_REG_CS:
       return NumVariant(m_context.SegCs);
    case CV_REG_SS:
       return NumVariant(m_context.SegSs);
    case CV_REG_DS:
       return NumVariant(m_context.SegDs);
    case CV_REG_FS:
        return NumVariant(m_context.SegFs);
    case CV_REG_GS:
        return NumVariant(m_context.SegGs);
    case CV_REG_IP:
        return NumVariant(static_cast<unsigned short>(m_context.Eip & 0xFFFF));
    case CV_REG_FLAGS:
        return NumVariant(static_cast<unsigned short>(m_context.EFlags & 0xFFFF));
    case CV_REG_EIP:
        return NumVariant(m_context.Eip);
    case CV_REG_EFLAGS:
        return NumVariant(m_context.EFlags);
    }

    std::stringstream sstr;
    sstr << "I386 context: unsupported register index " << std::dec << index;
    throw DbgException(sstr.str());
}

///////////////////////////////////////////////////////////////////////////////

std::wstring  CPUContextI386::getRegisterName(unsigned long index)
{
    switch (index)
    {
    case CV_REG_AL:
        return L"AL";
    case CV_REG_CL:
        return L"CL";
    case CV_REG_DL:
        return L"DL";
    case CV_REG_BL:
        return L"BL";
    case CV_REG_AH:
        return L"AH";
    case CV_REG_CH:
        return L"CH";
    case CV_REG_DH:
        return L"DH";
    case CV_REG_BH:
        return L"BH";
    case CV_REG_AX:
        return L"AX";
    case CV_REG_CX:
        return L"CX";
    case CV_REG_DX:
        return L"DX";
    case CV_REG_BX:
        return L"BX";
    case CV_REG_SP:
        return L"SP";
    case CV_REG_BP:
        return L"BP";
    case CV_REG_SI:
        return L"SI";
    case CV_REG_DI:
        return L"DI";
    case CV_REG_EAX:
        return L"EAX";
    case CV_REG_ECX:
        return L"ECX";
    case CV_REG_EDX:
        return L"EDX";
    case CV_REG_EBX:
        return L"EBX";
    case CV_REG_ESP:
        return L"ESP";
    case CV_REG_EBP:
        return L"EBP";
    case CV_REG_ESI:
        return L"ESI";
    case CV_REG_EDI:
        return L"EDI";
    case CV_REG_ES:
        return L"ES";
    case CV_REG_CS:
        return L"CS";
    case CV_REG_SS:
        return L"SS";
    case CV_REG_DS:
        return L"DS";
    case CV_REG_FS:
        return L"FS";
    case CV_REG_GS:
        return L"GS";
    case CV_REG_IP:
        return L"IP";
    case CV_REG_FLAGS:
        return L"FLAGS";
    case CV_REG_EIP:
        return L"EIP";
    case CV_REG_EFLAGS:
        return L"EFLAGS";
    }

    std::stringstream sstr;
    sstr << "I386 context: unsupported register index " << std::dec << index;
    throw DbgException(sstr.str());
}


///////////////////////////////////////////////////////////////////////////////

NumVariant CPUContextWOW64::getRegisterByIndex(unsigned long index)
{
    switch (index)
    {
    case CV_REG_AL:
        return NumVariant(static_cast<unsigned char>(m_context.Eax & 0xFF));
    case CV_REG_CL:
        return NumVariant(static_cast<unsigned char>(m_context.Ecx & 0xFF));
    case CV_REG_DL:
        return NumVariant(static_cast<unsigned char>(m_context.Edx & 0xFF));
    case CV_REG_BL:
        return NumVariant(static_cast<unsigned char>(m_context.Ebx & 0xFF));
    case CV_REG_AH:
        return NumVariant(static_cast<unsigned char>((m_context.Eax & 0xFF00) >> 8));
    case CV_REG_CH:
        return NumVariant(static_cast<unsigned char>((m_context.Ecx & 0xFF00) >> 8));
    case CV_REG_DH:
        return NumVariant(static_cast<unsigned char>((m_context.Edx & 0xFF00) >> 8));
    case CV_REG_BH:
        return NumVariant(static_cast<unsigned char>((m_context.Ebx & 0xFF00) >> 8));
    case CV_REG_AX:
        return NumVariant(static_cast<unsigned short>(m_context.Eax & 0xFFFF));
    case CV_REG_CX:
        return NumVariant(static_cast<unsigned short>(m_context.Ecx & 0xFFFF));
    case CV_REG_DX:
        return NumVariant(static_cast<unsigned short>(m_context.Edx & 0xFFFF));
    case CV_REG_BX:
        return NumVariant(static_cast<unsigned short>(m_context.Ebx & 0xFFFF));
    case CV_REG_SP:
        return NumVariant(static_cast<unsigned short>(m_context.Esp & 0xFFFF));
    case CV_REG_BP:
        return NumVariant(static_cast<unsigned short>(m_context.Ebp & 0xFFFF));
    case CV_REG_SI:
        return NumVariant(static_cast<unsigned short>(m_context.Esi & 0xFFFF));
    case CV_REG_DI:
        return NumVariant(static_cast<unsigned short>(m_context.Edi & 0xFFFF));
    case CV_REG_EAX:
        return NumVariant(m_context.Eax);
    case CV_REG_ECX:
        return NumVariant(m_context.Ecx);
    case CV_REG_EDX:
        return NumVariant(m_context.Edx);
    case CV_REG_EBX:
        return NumVariant(m_context.Ebx);
    case CV_REG_ESP:
        return NumVariant(m_context.Esp);
    case CV_REG_EBP:
        return NumVariant(m_context.Ebp);
    case CV_REG_ESI:
        return NumVariant(m_context.Esi);
    case CV_REG_EDI:
        return NumVariant(m_context.Edi);
    case CV_REG_ES:
        return NumVariant(m_context.SegEs);
    case CV_REG_CS:
        return NumVariant(m_context.SegCs);
    case CV_REG_SS:
        return NumVariant(m_context.SegSs);
    case CV_REG_DS:
        return NumVariant(m_context.SegDs);
    case CV_REG_FS:
        return NumVariant(m_context.SegFs);
    case CV_REG_GS:
        return NumVariant(m_context.SegGs);
    case CV_REG_IP:
        return NumVariant(static_cast<unsigned short>(m_context.Eip & 0xFFFF));
    case CV_REG_FLAGS:
        return NumVariant(static_cast<unsigned short>(m_context.EFlags & 0xFFFF));
    case CV_REG_EIP:
        return NumVariant(m_context.Eip);
    case CV_REG_EFLAGS:
        return NumVariant(m_context.EFlags);
    }

    std::stringstream sstr;
    sstr << "WOW64 context: unsupported register index " << std::dec << index;
    throw DbgException(sstr.str());
}

///////////////////////////////////////////////////////////////////////////////

std::wstring  CPUContextWOW64::getRegisterName(unsigned long index)
{
    switch (index)
    {
    case CV_REG_AL:
        return L"AL";
    case CV_REG_CL:
        return L"CL";
    case CV_REG_DL:
        return L"DL";
    case CV_REG_BL:
        return L"BL";
    case CV_REG_AH:
        return L"AH";
    case CV_REG_CH:
        return L"CH";
    case CV_REG_DH:
        return L"DH";
    case CV_REG_BH:
        return L"BH";
    case CV_REG_AX:
        return L"AX";
    case CV_REG_CX:
        return L"CX";
    case CV_REG_DX:
        return L"DX";
    case CV_REG_BX:
        return L"BX";
    case CV_REG_SP:
        return L"SP";
    case CV_REG_BP:
        return L"BP";
    case CV_REG_SI:
        return L"SI";
    case CV_REG_DI:
        return L"DI";
    case CV_REG_EAX:
        return L"EAX";
    case CV_REG_ECX:
        return L"ECX";
    case CV_REG_EDX:
        return L"EDX";
    case CV_REG_EBX:
        return L"EBX";
    case CV_REG_ESP:
        return L"ESP";
    case CV_REG_EBP:
        return L"EBP";
    case CV_REG_ESI:
        return L"ESI";
    case CV_REG_EDI:
        return L"EDI";
    case CV_REG_ES:
        return L"ES";
    case CV_REG_CS:
        return L"CS";
    case CV_REG_SS:
        return L"SS";
    case CV_REG_DS:
        return L"DS";
    case CV_REG_FS:
        return L"FS";
    case CV_REG_GS:
        return L"GS";
    case CV_REG_IP:
        return L"IP";
    case CV_REG_FLAGS:
        return L"FLAGS";
    case CV_REG_EIP:
        return L"EIP";
    case CV_REG_EFLAGS:
        return L"EFLAGS";
    }

    std::stringstream sstr;
    sstr << "I386 context: unsupported register index " << std::dec << index;
    throw DbgException(sstr.str());
}

///////////////////////////////////////////////////////////////////////////////

NumVariant CPUContextAmd64::getRegisterByIndex(unsigned long index)
{

    switch (index)
    {

    case CV_AMD64_AL:
        return NumVariant(static_cast<unsigned char>(m_context.Rax & 0xFF));
    case CV_AMD64_CL:
        return NumVariant(static_cast<unsigned char>(m_context.Rcx & 0xFF));
    case CV_AMD64_DL:
        return NumVariant(static_cast<unsigned char>(m_context.Rdx & 0xFF));
    case CV_AMD64_BL:
        return NumVariant(static_cast<unsigned char>(m_context.Rbx & 0xFF));
    case CV_AMD64_AH:
        return NumVariant(static_cast<unsigned char>((m_context.Rax & 0xFF00) >> 8));
    case CV_AMD64_CH:
        return NumVariant(static_cast<unsigned char>((m_context.Rcx & 0xFF00) >> 8));
    case CV_AMD64_DH:
        return NumVariant(static_cast<unsigned char>((m_context.Rdx & 0xFF00) >> 8));
    case CV_AMD64_BH:
        return NumVariant(static_cast<unsigned char>((m_context.Rbx & 0xFF00) >> 8));
    case CV_AMD64_AX:
        return NumVariant(static_cast<unsigned short>(m_context.Rax & 0xFFFF));
    case CV_AMD64_CX:
        return NumVariant(static_cast<unsigned short>(m_context.Rcx & 0xFFFF));
    case CV_AMD64_DX:
        return NumVariant(static_cast<unsigned short>(m_context.Rdx & 0xFFFF));
    case CV_AMD64_BX:
        return NumVariant(static_cast<unsigned short>(m_context.Rbx & 0xFFFF));
    case CV_AMD64_SP:
        return NumVariant(static_cast<unsigned short>(m_context.Rsp & 0xFFFF));
    case CV_AMD64_BP:
        return NumVariant(static_cast<unsigned short>(m_context.Rbp & 0xFFFF));
    case CV_AMD64_SI:
        return NumVariant(static_cast<unsigned short>(m_context.Rsi & 0xFFFF));
    case CV_AMD64_DI:
        return NumVariant(static_cast<unsigned short>(m_context.Rdi & 0xFFFF));
    case CV_AMD64_EAX:
        return NumVariant(static_cast<unsigned long>(m_context.Rax & 0xFFFFFFFF));
    case CV_AMD64_ECX:
        return NumVariant(static_cast<unsigned long>(m_context.Rcx & 0xFFFFFFFF));
    case CV_AMD64_EDX:
        return NumVariant(static_cast<unsigned long>(m_context.Rdx & 0xFFFFFFFF));
    case CV_AMD64_EBX:
        return NumVariant(static_cast<unsigned long>(m_context.Rbx & 0xFFFFFFFF));
    case CV_AMD64_ESP:
        return NumVariant(static_cast<unsigned long>(m_context.Rsp & 0xFFFFFFFF));
    case CV_AMD64_EBP:
        return NumVariant(static_cast<unsigned long>(m_context.Rbp & 0xFFFFFFFF));
    case CV_AMD64_ESI:
        return NumVariant(static_cast<unsigned long>(m_context.Rsi & 0xFFFFFFFF));
    case CV_AMD64_EDI:
        return NumVariant(static_cast<unsigned long>(m_context.Rdi & 0xFFFFFFFF));
    case CV_AMD64_ES:
        return NumVariant(m_context.SegEs);
    case CV_AMD64_CS:
        return NumVariant(m_context.SegCs);
    case CV_AMD64_SS:
        return NumVariant(m_context.SegSs);
    case CV_AMD64_DS:
        return NumVariant(m_context.SegDs);
    case CV_AMD64_FS:
        return NumVariant(m_context.SegFs);
    case CV_AMD64_GS:
        return NumVariant(m_context.SegGs);
    case CV_AMD64_FLAGS:
        return NumVariant(m_context.EFlags);
    case CV_AMD64_RIP:
        return NumVariant(m_context.Rip);
    case CV_AMD64_SIL:
        return NumVariant(static_cast<unsigned char>(m_context.Rsi & 0xFF));
    case CV_AMD64_DIL:
        return NumVariant(static_cast<unsigned char>(m_context.Rdi & 0xFF));
    case CV_AMD64_BPL:
        return NumVariant(static_cast<unsigned char>(m_context.Rbp & 0xFF));
    case CV_AMD64_SPL:
       return NumVariant(static_cast<unsigned char>(m_context.Rsp & 0xFF));
    case CV_AMD64_RAX:
        return NumVariant(m_context.Rax);
    case CV_AMD64_RBX:
        return NumVariant(m_context.Rbx);
    case  CV_AMD64_RDX:
        return NumVariant(m_context.Rdx);
    case CV_AMD64_RCX:
        return NumVariant(m_context.Rcx);
    case CV_AMD64_RSI:
        return NumVariant(m_context.Rsi);
    case CV_AMD64_RDI:
        return NumVariant(m_context.Rdi);
    case CV_AMD64_RBP:
        return NumVariant(m_context.Rbp);
    case CV_AMD64_RSP:
        return NumVariant(m_context.Rsp);
    case CV_AMD64_R8:
        return NumVariant(m_context.R8);
    case CV_AMD64_R9:
        return NumVariant(m_context.R9);
    case CV_AMD64_R10:
        return NumVariant(m_context.R10);
    case CV_AMD64_R11:
        return NumVariant(m_context.R11);
    case CV_AMD64_R12:
        return NumVariant(m_context.R12);
    case CV_AMD64_R13:
        return NumVariant(m_context.R13);
    case CV_AMD64_R14:
        return NumVariant(m_context.R14);
    case CV_AMD64_R15:
        return NumVariant(m_context.R15);
    }

    std::stringstream sstr;
    sstr << "AMD64 context: unsupported register index " << std::dec << index;
    throw DbgException(sstr.str());
}


std::wstring CPUContextAmd64::getRegisterName(unsigned long index)
{
    switch (index)
    {
    case CV_AMD64_AL:
        return L"AL";
    case CV_AMD64_CL:
        return L"CL";
    case CV_AMD64_DL:
        return L"DL";
    case CV_AMD64_BL:
        return L"BL";
    case CV_AMD64_AH:
        return L"AH";
    case CV_AMD64_CH:
        return L"CH";
    case CV_AMD64_DH:
        return L"DH";
    case CV_AMD64_BH:
        return L"BH";
    case CV_AMD64_AX:
        return L"AX";
    case CV_AMD64_CX:
        return L"CX";
    case CV_AMD64_DX:
        return L"DX";
    case CV_AMD64_BX:
        return L"BX";
    case CV_AMD64_SP:
        return L"SP";
    case CV_AMD64_BP:
        return L"BP";
    case CV_AMD64_SI:
        return L"SI";
    case CV_AMD64_DI:
        return L"DI";
    case CV_AMD64_EAX:
        return L"EAX";
    case CV_AMD64_ECX:
        return L"ECX";
    case CV_AMD64_EDX:
        return L"EDX";
    case CV_AMD64_EBX:
        return L"EBX";
    case CV_AMD64_ESP:
        return L"ESP";
    case CV_AMD64_EBP:
        return L"EBP";
    case CV_AMD64_ESI:
        return L"ESI";
    case CV_AMD64_EDI:
        return L"EDI";
    case CV_AMD64_ES:
        return L"ES";
    case CV_AMD64_CS:
        return L"CS";
    case CV_AMD64_SS:
        return L"SS";
    case CV_AMD64_DS:
        return L"DS";
    case CV_AMD64_FS:
        return L"FS";
    case CV_AMD64_GS:
        return L"GS";
    case CV_AMD64_FLAGS:
        return L"EFLAGS";
    case CV_AMD64_RIP:
        return L"RIP";
    case CV_AMD64_SIL:
        return L"SIL";
    case CV_AMD64_DIL:
        return L"DIL";
    case CV_AMD64_BPL:
        return L"BPL";
    case CV_AMD64_SPL:
        return L"SPL";
    case CV_AMD64_RAX:
        return L"RAX";
    case CV_AMD64_RBX:
        return L"RBX";
    case  CV_AMD64_RDX:
        return L"RDX";
    case CV_AMD64_RCX:
        return L"RCX";
    case CV_AMD64_RSI:
        return L"RSI";
    case CV_AMD64_RDI:
        return L"RDI";
    case CV_AMD64_RBP:
        return L"RBP";
    case CV_AMD64_RSP:
        return L"RSP";
    case CV_AMD64_R8:
        return L"R8";
    case CV_AMD64_R9:
        return L"R9";
    case CV_AMD64_R10:
        return L"R10";
    case CV_AMD64_R11:
        return L"R11";
    case CV_AMD64_R12:
        return L"R12";
    case CV_AMD64_R13:
        return L"R13";
    case CV_AMD64_R14:
        return L"R14";
    case CV_AMD64_R15:
        return L"R15";
    }

    std::stringstream sstr;
    sstr << "AMD64 context: unsupported register index " << std::dec << index;
    throw DbgException(sstr.str());
}

///////////////////////////////////////////////////////////////////////////////


} // kdlib namespace end
