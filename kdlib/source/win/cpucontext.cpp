#include "stdafx.h"

#include <cvconst.h>

#include "kdlib/dbgengine.h"
#include "kdlib/exceptions.h"
#include "kdlib/memaccess.h"

#include "stackimpl.h"
#include "cpucontextimpl.h"
#include "dbgmgr.h"


namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

NumVariant getRegisterByName(const std::wstring& regName)
{
    unsigned long index = getRegisterIndex(regName);
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
        throw DbgException( "unsupported registry type");
    }

    throw DbgException( "unsupported registry type");
}

///////////////////////////////////////////////////////////////////////////////

void setRegisterByName(const std::wstring& regName, const NumVariant& value)
{
    unsigned long index = getRegisterIndex(regName);
    return setRegisterByIndex(index, value);
}

///////////////////////////////////////////////////////////////////////////////

void setRegisterByIndex(unsigned long index, const NumVariant& value)
{
    CPURegType  regType = getRegisterType(index);

    switch( regType )
    {
    case  RegInt8:
        {
            unsigned char  val = value.asUChar();
            setRegisterValue( index, &val, sizeof(val) );
            return;
        }
        break;

    case  RegInt16:
        {
            unsigned short  val = value.asUShort();
            setRegisterValue( index, &val, sizeof(val) );
            return;
        }
        break;

    case  RegInt32:
        {
            unsigned long  val = value.asULong();
            setRegisterValue( index, &val, sizeof(val) );
            return;
        }
        break;


    case  RegInt64:
        {
            unsigned long long  val = value.asULongLong();
            setRegisterValue( index, &val, sizeof(val) );
            return;
        }
        break;

    case  RegFloat32:
        {
            float  val = value.asFloat();
            setRegisterValue( index, &val, sizeof(val) );
            return;
        }
        break;


    case  RegFloat64:
        {
            double  val = value.asDouble();
            setRegisterValue( index, &val, sizeof(val) );
            return;
        }
        break;

    case RegFloat80:
    case RegFloat128:
    case RegVector64:
    case RegVector128:
        throw DbgException( "unsupported registry type");
    }

    throw DbgException( "unsupported registry type");
}

///////////////////////////////////////////////////////////////////////////////

int alignStackPointer( int byteCount )
{
    int machineWord;

    switch( getCPUMode() )
    {
    case CPU_I386:
        machineWord = 4;
        break;

    case CPU_AMD64:
    case CPU_ARM64:
        machineWord = 8;
        break;

    default:
        throw DbgException( "Unknown processor type" );
    }

    return ( ( byteCount - 1) / machineWord + 1 ) * machineWord;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 stackAlloc(unsigned short byteCount)
{
    byteCount = alignStackPointer(byteCount);

    MEMOFFSET_64  offset = getStackOffset() - byteCount;
    setStackOffset(offset);
    return offset;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 stackFree(unsigned short byteCount)
{
    byteCount = alignStackPointer(byteCount);

    MEMOFFSET_64  offset = getStackOffset() + byteCount;
    setStackOffset(offset);
    return offset;
}

///////////////////////////////////////////////////////////////////////////////

void pushInStack(const NumVariant& value)
{
    if ( value.isChar() || value.isUChar() )
    {
        setByte( stackAlloc(1), value.asUChar() );
        return;
    }

    if ( value.isShort() || value.isUShort() )
    {
        setWord( stackAlloc(2), value.asUShort() );
        return;
    }

    if ( value.isLong() || value.isULong() || value.isInt()  || value.isUInt() )
    {
        setDWord( stackAlloc(4), value.asULong() );
        return;
    }

    if ( value.isLongLong() || value.isULongLong() )
    {
        setQWord( stackAlloc(8), value.asULongLong() );
        return;
    }

    if ( value.isFloat() )
    {
        setSingleFloat( stackAlloc(4), value.asFloat() );
        return;
    }

    if ( value.isDouble() )
    {
        setDoubleFloat( stackAlloc(8), value.asDouble() );
        return;
    }

    throw DbgException("unsupported argument type");
}

///////////////////////////////////////////////////////////////////////////////

void popFromStack(NumVariant& value)
{
    NOT_IMPLEMENTED();
}

///////////////////////////////////////////////////////////////////////////////

//void CallArg::pushInStack() const
//{
//    writeBytes(stackAlloc(m_rawBuffer.size()), m_rawBuffer);
//}
//
/////////////////////////////////////////////////////////////////////////////////
//
//void CallArg::saveToRegister(const std::wstring& regName) const
//{
//    unsigned long   regIndex  = getRegsiterIndex(regName);
//    setRegisterValue(regIndex,const_cast<unsigned char*>(&m_rawBuffer[0]), m_rawBuffer.size() );
//}

///////////////////////////////////////////////////////////////////////////////

CPUContextPtr loadCPUContext()
{
    switch( kdlib::getCPUType() )
    {
    case CPU_AMD64:
        switch (kdlib::getCPUMode())
        {
        case CPU_AMD64:
            return CPUContextPtr( new CPUContextAmd64() );

        case CPU_I386:
            return CPUContextPtr( new CPUContextWOW64() );
        }
        break;

    case CPU_I386:
        return CPUContextPtr( new CPUContextI386() ); 

    case CPU_ARM64:
        return CPUContextPtr( new CPUContextArm64() ); 
    }

    throw DbgException("Unknown CPU");
}

///////////////////////////////////////////////////////////////////////////////

template <class ContextType>
StackPtr getStackImpl()
{
    HRESULT  hres;

    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME>  frames(filledFrames);
    std::vector<ContextType::RawContextType>  contexts(filledFrames);

    g_dbgMgr->setQuietNotiification(true);

    hres =
        g_dbgMgr->control->GetContextStackTrace(
            NULL,
            0,
            &frames[0],
            filledFrames,
            &contexts[0],
            filledFrames * sizeof(ContextType::RawContextType),
            sizeof(ContextType::RawContextType),
            &filledFrames
            );

    g_dbgMgr->setQuietNotiification(false);

    if (S_OK != hres)
        throw DbgEngException( L"IDebugControl::GetContextStackTrace", hres );

    std::vector<StackFramePtr>  stackFrames;

    for (ULONG i = 0; i < filledFrames; ++i)
    {
        stackFrames.push_back(StackFramePtr(new StackFrameImpl(
            i,
            frames[i].InstructionOffset,
            frames[i].ReturnOffset,
            frames[i].FrameOffset,
            frames[i].StackOffset,
            CPUContextPtr(new ContextType(contexts[i])))));
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
        throw DbgEngException(L"IDebugControl::GetContextStackTrace", hres);

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

StackPtr getStack()
{
    HRESULT  hres;
    
    ULONG  cpuType;
    hres = g_dbgMgr->control->GetActualProcessorType((PULONG)&cpuType);

    if (FAILED(hres))
        throw DbgEngException(L"IDebugControl::GetActualProcessorType", hres);

    if (cpuType == IMAGE_FILE_MACHINE_I386)
        return getStackImpl<CPUContextI386>();

    if (cpuType == IMAGE_FILE_MACHINE_ARM64)
        return getStackImpl<CPUContextArm64>();

    ULONG  cpuMode;
    hres = g_dbgMgr->control->GetEffectiveProcessorType((PULONG)&cpuMode);
    if (FAILED(hres))
        throw DbgEngException(L"IDebugControl::GetEffectiveProcessorType", hres);

    if (cpuType == IMAGE_FILE_MACHINE_AMD64 && cpuMode == IMAGE_FILE_MACHINE_AMD64)
        return getStackImpl<CPUContextAmd64>();

    if (cpuType == IMAGE_FILE_MACHINE_AMD64 && cpuMode == IMAGE_FILE_MACHINE_I386)
        return getStackWow64();
    
    throw DbgException("Unknown CPU type/mode");
}

///////////////////////////////////////////////////////////////////////////////

template <class ContextType>
StackFramePtr getStackFrameImpl()
{
    HRESULT  hres;
    DEBUG_STACK_FRAME  stackFrame = {};
    ULONG  frameNumber;

    ULONG   filledFrames = 1024;
    std::vector<DEBUG_STACK_FRAME>  frames(filledFrames);
    std::vector<ContextType::RawContextType>  contexts(filledFrames);
    
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
        filledFrames*sizeof(ContextType::RawContextType),
        sizeof(ContextType::RawContextType),
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
        CPUContextPtr(new ContextType(contexts[frameNumber]))));
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

StackFramePtr getCurrentStackFrame()
{
    HRESULT  hres;

    ULONG  cpuType;
    hres = g_dbgMgr->control->GetActualProcessorType((PULONG)&cpuType);

    if (FAILED(hres))
        throw DbgEngException(L"IDebugControl::GetActualProcessorType", hres);

    if (cpuType == IMAGE_FILE_MACHINE_I386)
        return getStackFrameImpl<CPUContextI386>();

    if (cpuType == IMAGE_FILE_MACHINE_ARM64)
        return getStackFrameImpl<CPUContextArm64>();

    ULONG  cpuMode;
    hres = g_dbgMgr->control->GetEffectiveProcessorType((PULONG)&cpuMode);
    if (FAILED(hres))
        throw DbgEngException(L"IDebugControl::GetEffectiveProcessorType", hres);

    if (cpuType == IMAGE_FILE_MACHINE_AMD64 && cpuMode == IMAGE_FILE_MACHINE_AMD64)
        return getStackFrameImpl<CPUContextAmd64>();

    if (cpuType == IMAGE_FILE_MACHINE_AMD64 && cpuMode == IMAGE_FILE_MACHINE_I386)
        return getStackFrameWow64();

    throw DbgException("Unknown CPU type/mode");
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
        return L"al";
    case CV_REG_CL:
        return L"cl";
    case CV_REG_DL:
        return L"dl";
    case CV_REG_BL:
        return L"bl";
    case CV_REG_AH:
        return L"ah";
    case CV_REG_CH:
        return L"ch";
    case CV_REG_DH:
        return L"dh";
    case CV_REG_BH:
        return L"bh";
    case CV_REG_AX:
        return L"ax";
    case CV_REG_CX:
        return L"cx";
    case CV_REG_DX:
        return L"dx";
    case CV_REG_BX:
        return L"bx";
    case CV_REG_SP:
        return L"sp";
    case CV_REG_BP:
        return L"bp";
    case CV_REG_SI:
        return L"si";
    case CV_REG_DI:
        return L"di";
    case CV_REG_EAX:
        return L"eax";
    case CV_REG_ECX:
        return L"ecx";
    case CV_REG_EDX:
        return L"edx";
    case CV_REG_EBX:
        return L"ebx";
    case CV_REG_ESP:
        return L"esp";
    case CV_REG_EBP:
        return L"ebp";
    case CV_REG_ESI:
        return L"esi";
    case CV_REG_EDI:
        return L"edi";
    case CV_REG_ES:
        return L"es";
    case CV_REG_CS:
        return L"cs";
    case CV_REG_SS:
        return L"ss";
    case CV_REG_DS:
        return L"ds";
    case CV_REG_FS:
        return L"fs";
    case CV_REG_GS:
        return L"gs";
    case CV_REG_IP:
        return L"ip";
    case CV_REG_FLAGS:
        return L"flags";
    case CV_REG_EIP:
        return L"eip";
    case CV_REG_EFLAGS:
        return L"eflags";
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
        return L"al";
    case CV_REG_CL:
        return L"cl";
    case CV_REG_DL:
        return L"dl";
    case CV_REG_BL:
        return L"bl";
    case CV_REG_AH:
        return L"ah";
    case CV_REG_CH:
        return L"ch";
    case CV_REG_DH:
        return L"dh";
    case CV_REG_BH:
        return L"bh";
    case CV_REG_AX:
        return L"ax";
    case CV_REG_CX:
        return L"cx";
    case CV_REG_DX:
        return L"dx";
    case CV_REG_BX:
        return L"bx";
    case CV_REG_SP:
        return L"sp";
    case CV_REG_BP:
        return L"bp";
    case CV_REG_SI:
        return L"si";
    case CV_REG_DI:
        return L"di";
    case CV_REG_EAX:
        return L"eax";
    case CV_REG_ECX:
        return L"ecx";
    case CV_REG_EDX:
        return L"edx";
    case CV_REG_EBX:
        return L"ebx";
    case CV_REG_ESP:
        return L"esp";
    case CV_REG_EBP:
        return L"ebp";
    case CV_REG_ESI:
        return L"esi";
    case CV_REG_EDI:
        return L"edi";
    case CV_REG_ES:
        return L"es";
    case CV_REG_CS:
        return L"cs";
    case CV_REG_SS:
        return L"ss";
    case CV_REG_DS:
        return L"ds";
    case CV_REG_FS:
        return L"fs";
    case CV_REG_GS:
        return L"gs";
    case CV_REG_IP:
        return L"ip";
    case CV_REG_FLAGS:
        return L"flags";
    case CV_REG_EIP:
        return L"eip";
    case CV_REG_EFLAGS:
        return L"eflags";
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
    case CV_AMD64_R8B:
        return NumVariant(static_cast<unsigned char>(m_context.R8 & 0xFF));
    case CV_AMD64_R9B:
        return NumVariant(static_cast<unsigned char>(m_context.R9 & 0xFF));
    case CV_AMD64_R10B:
        return NumVariant(static_cast<unsigned char>(m_context.R10 & 0xFF));
    case CV_AMD64_R11B:
        return NumVariant(static_cast<unsigned char>(m_context.R11 & 0xFF));
    case CV_AMD64_R12B:
        return NumVariant(static_cast<unsigned char>(m_context.R12 & 0xFF));
    case CV_AMD64_R13B:
        return NumVariant(static_cast<unsigned char>(m_context.R13 & 0xFF));
    case CV_AMD64_R14B:
        return NumVariant(static_cast<unsigned char>(m_context.R14 & 0xFF));
    case CV_AMD64_R15B:
        return NumVariant(static_cast<unsigned char>(m_context.R15 & 0xFF));
    case CV_AMD64_R8W:
        return NumVariant(static_cast<unsigned short>(m_context.R8 & 0xFFFF));
    case CV_AMD64_R9W:
        return NumVariant(static_cast<unsigned short>(m_context.R9 & 0xFFFF));
    case CV_AMD64_R10W:
        return NumVariant(static_cast<unsigned short>(m_context.R10 & 0xFFFF));
    case CV_AMD64_R11W:
        return NumVariant(static_cast<unsigned short>(m_context.R11 & 0xFFFF));
    case CV_AMD64_R12W:
        return NumVariant(static_cast<unsigned short>(m_context.R12 & 0xFFFF));
    case CV_AMD64_R13W:
        return NumVariant(static_cast<unsigned short>(m_context.R13 & 0xFFFF));
    case CV_AMD64_R14W:
        return NumVariant(static_cast<unsigned short>(m_context.R14 & 0xFFFF));
    case CV_AMD64_R15W:
        return NumVariant(static_cast<unsigned short>(m_context.R15 & 0xFFFF));
    case CV_AMD64_R8D:
        return NumVariant(static_cast<unsigned long>(m_context.R8 & 0xFFFFFFFF));
    case CV_AMD64_R9D:
        return NumVariant(static_cast<unsigned long>(m_context.R9 & 0xFFFFFFFF));
    case CV_AMD64_R10D:
        return NumVariant(static_cast<unsigned long>(m_context.R10 & 0xFFFFFFFF));
    case CV_AMD64_R11D:
        return NumVariant(static_cast<unsigned long>(m_context.R11 & 0xFFFFFFFF));
    case CV_AMD64_R12D:
        return NumVariant(static_cast<unsigned long>(m_context.R12 & 0xFFFFFFFF));
    case CV_AMD64_R13D:
        return NumVariant(static_cast<unsigned long>(m_context.R13 & 0xFFFFFFFF));
    case CV_AMD64_R14D:
        return NumVariant(static_cast<unsigned long>(m_context.R14 & 0xFFFFFFFF));
    case CV_AMD64_R15D:
        return NumVariant(static_cast<unsigned long>(m_context.R15 & 0xFFFFFFFF));
    }

    std::stringstream sstr;
    sstr << "AMD64 context: unsupported register index " << std::dec << index;
    throw DbgException(sstr.str());
}

///////////////////////////////////////////////////////////////////////////////

std::wstring CPUContextAmd64::getRegisterName(unsigned long index)
{
    switch (index)
    {
    case CV_AMD64_AL:
        return L"al";
    case CV_AMD64_CL:
        return L"cl";
    case CV_AMD64_DL:
        return L"dl";
    case CV_AMD64_BL:
        return L"bl";
    case CV_AMD64_AH:
        return L"ah";
    case CV_AMD64_CH:
        return L"ch";
    case CV_AMD64_DH:
        return L"dh";
    case CV_AMD64_BH:
        return L"bh";
    case CV_AMD64_AX:
        return L"ax";
    case CV_AMD64_CX:
        return L"cx";
    case CV_AMD64_DX:
        return L"dx";
    case CV_AMD64_BX:
        return L"bx";
    case CV_AMD64_SP:
        return L"sp";
    case CV_AMD64_BP:
        return L"bp";
    case CV_AMD64_SI:
        return L"si";
    case CV_AMD64_DI:
        return L"di";
    case CV_AMD64_EAX:
        return L"eax";
    case CV_AMD64_ECX:
        return L"ecx";
    case CV_AMD64_EDX:
        return L"edx";
    case CV_AMD64_EBX:
        return L"ebx";
    case CV_AMD64_ESP:
        return L"esp";
    case CV_AMD64_EBP:
        return L"ebp";
    case CV_AMD64_ESI:
        return L"esi";
    case CV_AMD64_EDI:
        return L"edi";
    case CV_AMD64_ES:
        return L"es";
    case CV_AMD64_CS:
        return L"cs";
    case CV_AMD64_SS:
        return L"ss";
    case CV_AMD64_DS:
        return L"ds";
    case CV_AMD64_FS:
        return L"fs";
    case CV_AMD64_GS:
        return L"gs";
    case CV_AMD64_FLAGS:
        return L"eflags";
    case CV_AMD64_RIP:
        return L"rip";
    case CV_AMD64_SIL:
        return L"sil";
    case CV_AMD64_DIL:
        return L"dil";
    case CV_AMD64_BPL:
        return L"bpl";
    case CV_AMD64_SPL:
        return L"spl";
    case CV_AMD64_RAX:
        return L"rax";
    case CV_AMD64_RBX:
        return L"rbx";
    case  CV_AMD64_RDX:
        return L"rdx";
    case CV_AMD64_RCX:
        return L"rcx";
    case CV_AMD64_RSI:
        return L"rsi";
    case CV_AMD64_RDI:
        return L"rdi";
    case CV_AMD64_RBP:
        return L"rbp";
    case CV_AMD64_RSP:
        return L"rsp";
    case CV_AMD64_R8:
        return L"r8";
    case CV_AMD64_R9:
        return L"r9";
    case CV_AMD64_R10:
        return L"r10";
    case CV_AMD64_R11:
        return L"r11";
    case CV_AMD64_R12:
        return L"r12";
    case CV_AMD64_R13:
        return L"r13";
    case CV_AMD64_R14:
        return L"r14";
    case CV_AMD64_R15:
        return L"r15";
    case CV_AMD64_R8B:
        return L"r8b";
    case CV_AMD64_R9B:
        return L"r9b";
    case CV_AMD64_R10B:
        return L"r10b";
    case CV_AMD64_R11B:
        return L"r11b";
    case CV_AMD64_R12B:
        return L"r12b";
    case CV_AMD64_R13B:
        return L"r13b";
    case CV_AMD64_R14B:
        return L"r14b";
    case CV_AMD64_R15B:
        return L"r15b";
    case CV_AMD64_R8W:
        return L"r8w";
    case CV_AMD64_R9W:
        return L"r9w";
    case CV_AMD64_R10W:
        return L"r10w";
    case CV_AMD64_R11W:
        return L"r11w";
    case CV_AMD64_R12W:
        return L"r12w";
    case CV_AMD64_R13W:
        return L"r13w";
    case CV_AMD64_R14W:
        return L"r14w";
    case CV_AMD64_R15W:
        return L"r15w";
    case CV_AMD64_R8D:
        return L"r8d";
    case CV_AMD64_R9D:
        return L"r9d";
    case CV_AMD64_R10D:
        return L"r10d";
    case CV_AMD64_R11D:
        return L"r11d";
    case CV_AMD64_R12D:
        return L"r12d";
    case CV_AMD64_R13D:
        return L"r13d";
    case CV_AMD64_R14D:
        return L"r14d";
    case CV_AMD64_R15D:
        return L"r15d";
    }

    std::stringstream sstr;
    sstr << "AMD64 context: unsupported register index " << std::dec << index;
    throw DbgException(sstr.str());
}

///////////////////////////////////////////////////////////////////////////////

NumVariant CPUContextArm64::getRegisterByIndex(unsigned long index)
{
    switch (index)
    {
    case CV_ARM64_W0:
        return NumVariant(static_cast<unsigned long>(m_context.X0 & 0xFFFFFFFF));
    case CV_ARM64_W1:
        return NumVariant(static_cast<unsigned long>(m_context.X1 & 0xFFFFFFFF));
    case CV_ARM64_W2:
        return NumVariant(static_cast<unsigned long>(m_context.X2 & 0xFFFFFFFF));
    case CV_ARM64_W3:
        return NumVariant(static_cast<unsigned long>(m_context.X3 & 0xFFFFFFFF));
    case CV_ARM64_W4:
        return NumVariant(static_cast<unsigned long>(m_context.X4 & 0xFFFFFFFF));
    case CV_ARM64_W5:
        return NumVariant(static_cast<unsigned long>(m_context.X5 & 0xFFFFFFFF));
    case CV_ARM64_W6:
        return NumVariant(static_cast<unsigned long>(m_context.X6 & 0xFFFFFFFF));
    case CV_ARM64_W7:
        return NumVariant(static_cast<unsigned long>(m_context.X7 & 0xFFFFFFFF));
    case CV_ARM64_W8:
        return NumVariant(static_cast<unsigned long>(m_context.X8 & 0xFFFFFFFF));
    case CV_ARM64_W9:
        return NumVariant(static_cast<unsigned long>(m_context.X9 & 0xFFFFFFFF));
    case CV_ARM64_W10:
        return NumVariant(static_cast<unsigned long>(m_context.X10 & 0xFFFFFFFF));
    case CV_ARM64_W11:
        return NumVariant(static_cast<unsigned long>(m_context.X11 & 0xFFFFFFFF));
    case CV_ARM64_W12:
        return NumVariant(static_cast<unsigned long>(m_context.X12 & 0xFFFFFFFF));
    case CV_ARM64_W13:
        return NumVariant(static_cast<unsigned long>(m_context.X13 & 0xFFFFFFFF));
    case CV_ARM64_W14:
        return NumVariant(static_cast<unsigned long>(m_context.X14 & 0xFFFFFFFF));
    case CV_ARM64_W15:
        return NumVariant(static_cast<unsigned long>(m_context.X15 & 0xFFFFFFFF));
    case CV_ARM64_W16:
        return NumVariant(static_cast<unsigned long>(m_context.X16 & 0xFFFFFFFF));
    case CV_ARM64_W17:
        return NumVariant(static_cast<unsigned long>(m_context.X17 & 0xFFFFFFFF));
    case CV_ARM64_W18:
        return NumVariant(static_cast<unsigned long>(m_context.X18 & 0xFFFFFFFF));
    case CV_ARM64_W19:
        return NumVariant(static_cast<unsigned long>(m_context.X19 & 0xFFFFFFFF));
    case CV_ARM64_W20:
        return NumVariant(static_cast<unsigned long>(m_context.X20 & 0xFFFFFFFF));
    case CV_ARM64_W21:
        return NumVariant(static_cast<unsigned long>(m_context.X21 & 0xFFFFFFFF));
    case CV_ARM64_W22:
        return NumVariant(static_cast<unsigned long>(m_context.X22 & 0xFFFFFFFF));
    case CV_ARM64_W23:
        return NumVariant(static_cast<unsigned long>(m_context.X23 & 0xFFFFFFFF));
    case CV_ARM64_W24:
        return NumVariant(static_cast<unsigned long>(m_context.X24 & 0xFFFFFFFF));
    case CV_ARM64_W25:
        return NumVariant(static_cast<unsigned long>(m_context.X25 & 0xFFFFFFFF));
    case CV_ARM64_W26:
        return NumVariant(static_cast<unsigned long>(m_context.X26 & 0xFFFFFFFF));
    case CV_ARM64_W27:
        return NumVariant(static_cast<unsigned long>(m_context.X27 & 0xFFFFFFFF));
    case CV_ARM64_W28:
        return NumVariant(static_cast<unsigned long>(m_context.X28 & 0xFFFFFFFF));
    case CV_ARM64_W29:
        return NumVariant(static_cast<unsigned long>(m_context.Fp & 0xFFFFFFFF));
    case CV_ARM64_W30:
        return NumVariant(static_cast<unsigned long>(m_context.Lr & 0xFFFFFFFF));

    case CV_ARM64_X0:
        return NumVariant(m_context.X0);
    case CV_ARM64_X1:
        return NumVariant(m_context.X1);
    case CV_ARM64_X2:
        return NumVariant(m_context.X2);
    case CV_ARM64_X3:
        return NumVariant(m_context.X3);
    case CV_ARM64_X4:
        return NumVariant(m_context.X4);
    case CV_ARM64_X5:
        return NumVariant(m_context.X5);
    case CV_ARM64_X6:
        return NumVariant(m_context.X6);
    case CV_ARM64_X7:
        return NumVariant(m_context.X7);
    case CV_ARM64_X8:
        return NumVariant(m_context.X8);
    case CV_ARM64_X9:
        return NumVariant(m_context.X9);
    case CV_ARM64_X10:
        return NumVariant(m_context.X10);
    case CV_ARM64_X11:
        return NumVariant(m_context.X11);
    case CV_ARM64_X12:
        return NumVariant(m_context.X12);
    case CV_ARM64_X13:
        return NumVariant(m_context.X13);
    case CV_ARM64_X14:
        return NumVariant(m_context.X14);
    case CV_ARM64_X15:
        return NumVariant(m_context.X15);
    /* CV_ARM64_IP0 / CV_ARM64_IP1 ??? */
    case CV_ARM64_X18:
        return NumVariant(m_context.X18);
    case CV_ARM64_X19:
        return NumVariant(m_context.X19);
    case CV_ARM64_X20:
        return NumVariant(m_context.X20);
    case CV_ARM64_X21:
        return NumVariant(m_context.X21);
    case CV_ARM64_X22:
        return NumVariant(m_context.X22);
    case CV_ARM64_X23:
        return NumVariant(m_context.X23);
    case CV_ARM64_X24:
        return NumVariant(m_context.X24);
    case CV_ARM64_X25:
        return NumVariant(m_context.X25);
    case CV_ARM64_X26:
        return NumVariant(m_context.X26);
    case CV_ARM64_X27:
        return NumVariant(m_context.X27);
    case CV_ARM64_X28:
        return NumVariant(m_context.X28);
    case CV_ARM64_FP:
        return NumVariant(m_context.Fp);
    case CV_ARM64_LR:
        return NumVariant(m_context.Lr);
    case CV_ARM64_SP:
        return NumVariant(m_context.Sp);
    case CV_ARM64_PC:
        return NumVariant(m_context.Pc);

    case CV_ARM64_NZCV:
        return NumVariant(m_context.ContextFlags);
    case CV_ARM64_CPSR:
        return NumVariant(m_context.Cpsr);

    case CV_ARM64_S0:
        return NumVariant(m_context.V[0].S);
    case CV_ARM64_S1:
        return NumVariant(m_context.V[1].S);
    case CV_ARM64_S2:
        return NumVariant(m_context.V[2].S);
    case CV_ARM64_S3:
        return NumVariant(m_context.V[3].S);
    case CV_ARM64_S4:
        return NumVariant(m_context.V[4].S);
    case CV_ARM64_S5:
        return NumVariant(m_context.V[5].S);
    case CV_ARM64_S6:
        return NumVariant(m_context.V[6].S);
    case CV_ARM64_S7:
        return NumVariant(m_context.V[7].S);
    case CV_ARM64_S8:
        return NumVariant(m_context.V[8].S);
    case CV_ARM64_S9:
        return NumVariant(m_context.V[9].S);
    case CV_ARM64_S10:
        return NumVariant(m_context.V[10].S);
    case CV_ARM64_S11:
        return NumVariant(m_context.V[11].S);
    case CV_ARM64_S12:
        return NumVariant(m_context.V[12].S);
    case CV_ARM64_S13:
        return NumVariant(m_context.V[13].S);
    case CV_ARM64_S14:
        return NumVariant(m_context.V[14].S);
    case CV_ARM64_S15:
        return NumVariant(m_context.V[15].S);
    case CV_ARM64_S16:
        return NumVariant(m_context.V[16].S);
    case CV_ARM64_S17:
        return NumVariant(m_context.V[17].S);
    case CV_ARM64_S18:
        return NumVariant(m_context.V[18].S);
    case CV_ARM64_S19:
        return NumVariant(m_context.V[19].S);
    case CV_ARM64_S20:
        return NumVariant(m_context.V[20].S);
    case CV_ARM64_S21:
        return NumVariant(m_context.V[21].S);
    case CV_ARM64_S22:
        return NumVariant(m_context.V[22].S);
    case CV_ARM64_S23:
        return NumVariant(m_context.V[23].S);
    case CV_ARM64_S24:
        return NumVariant(m_context.V[24].S);
    case CV_ARM64_S25:
        return NumVariant(m_context.V[25].S);
    case CV_ARM64_S26:
        return NumVariant(m_context.V[26].S);
    case CV_ARM64_S27:
        return NumVariant(m_context.V[27].S);
    case CV_ARM64_S28:
        return NumVariant(m_context.V[28].S);
    case CV_ARM64_S29:
        return NumVariant(m_context.V[29].S);
    case CV_ARM64_S30:
        return NumVariant(m_context.V[30].S);
    case CV_ARM64_S31:
        return NumVariant(m_context.V[31].S);

    case CV_ARM64_D0:
        return NumVariant(m_context.V[0].D);
    case CV_ARM64_D1:
        return NumVariant(m_context.V[1].D);
    case CV_ARM64_D2:
        return NumVariant(m_context.V[2].D);
    case CV_ARM64_D3:
        return NumVariant(m_context.V[3].D);
    case CV_ARM64_D4:
        return NumVariant(m_context.V[4].D);
    case CV_ARM64_D5:
        return NumVariant(m_context.V[5].D);
    case CV_ARM64_D6:
        return NumVariant(m_context.V[6].D);
    case CV_ARM64_D7:
        return NumVariant(m_context.V[7].D);
    case CV_ARM64_D8:
        return NumVariant(m_context.V[8].D);
    case CV_ARM64_D9:
        return NumVariant(m_context.V[9].D);
    case CV_ARM64_D10:
        return NumVariant(m_context.V[10].D);
    case CV_ARM64_D11:
        return NumVariant(m_context.V[11].D);
    case CV_ARM64_D12:
        return NumVariant(m_context.V[12].D);
    case CV_ARM64_D13:
        return NumVariant(m_context.V[13].D);
    case CV_ARM64_D14:
        return NumVariant(m_context.V[14].D);
    case CV_ARM64_D15:
        return NumVariant(m_context.V[15].D);
    case CV_ARM64_D16:
        return NumVariant(m_context.V[16].D);
    case CV_ARM64_D17:
        return NumVariant(m_context.V[17].D);
    case CV_ARM64_D18:
        return NumVariant(m_context.V[18].D);
    case CV_ARM64_D19:
        return NumVariant(m_context.V[19].D);
    case CV_ARM64_D20:
        return NumVariant(m_context.V[20].D);
    case CV_ARM64_D21:
        return NumVariant(m_context.V[21].D);
    case CV_ARM64_D22:
        return NumVariant(m_context.V[22].D);
    case CV_ARM64_D23:
        return NumVariant(m_context.V[23].D);
    case CV_ARM64_D24:
        return NumVariant(m_context.V[24].D);
    case CV_ARM64_D25:
        return NumVariant(m_context.V[25].D);
    case CV_ARM64_D26:
        return NumVariant(m_context.V[26].D);
    case CV_ARM64_D27:
        return NumVariant(m_context.V[27].D);
    case CV_ARM64_D28:
        return NumVariant(m_context.V[28].D);
    case CV_ARM64_D29:
        return NumVariant(m_context.V[29].D);
    case CV_ARM64_D30:
        return NumVariant(m_context.V[30].D);
    case CV_ARM64_D31:
        return NumVariant(m_context.V[31].D);

    /* NumVariant can not provide 128-bit registers :( */
    // CV_ARM64_Q0 ... CV_ARM64_Q31

    case CV_ARM64_FPSR:
        return NumVariant(m_context.Fpsr);
    case CV_ARM64_FPCR:
        return NumVariant(m_context.Fpcr);
    }

    std::stringstream sstr;
    sstr << "ARM64 context: unsupported register index " << std::dec << index;
    throw DbgException(sstr.str());
}

///////////////////////////////////////////////////////////////////////////////

std::wstring CPUContextArm64::getRegisterName(unsigned long index)
{
    switch (index)
    {
    case CV_ARM64_W0:
        return L"w0";
    case CV_ARM64_W1:
        return L"w1";
    case CV_ARM64_W2:
        return L"w2";
    case CV_ARM64_W3:
        return L"w3";
    case CV_ARM64_W4:
        return L"w4";
    case CV_ARM64_W5:
        return L"w5";
    case CV_ARM64_W6:
        return L"w6";
    case CV_ARM64_W7:
        return L"w7";
    case CV_ARM64_W8:
        return L"w8";
    case CV_ARM64_W9:
        return L"w9";
    case CV_ARM64_W10:
        return L"w10";
    case CV_ARM64_W11:
        return L"w11";
    case CV_ARM64_W12:
        return L"w12";
    case CV_ARM64_W13:
        return L"w13";
    case CV_ARM64_W14:
        return L"w14";
    case CV_ARM64_W15:
        return L"w15";
    case CV_ARM64_W16:
        return L"w16";
    case CV_ARM64_W17:
        return L"w17";
    case CV_ARM64_W18:
        return L"w18";
    case CV_ARM64_W19:
        return L"w19";
    case CV_ARM64_W20:
        return L"w20";
    case CV_ARM64_W21:
        return L"w21";
    case CV_ARM64_W22:
        return L"w22";
    case CV_ARM64_W23:
        return L"w23";
    case CV_ARM64_W24:
        return L"w24";
    case CV_ARM64_W25:
        return L"w25";
    case CV_ARM64_W26:
        return L"w26";
    case CV_ARM64_W27:
        return L"w27";
    case CV_ARM64_W28:
        return L"w28";
    case CV_ARM64_W29:
        return L"w29";
    case CV_ARM64_W30:
        return L"w30";

    case CV_ARM64_X0:
        return L"x0";
    case CV_ARM64_X1:
        return L"x1";
    case CV_ARM64_X2:
        return L"x2";
    case CV_ARM64_X3:
        return L"x3";
    case CV_ARM64_X4:
        return L"x4";
    case CV_ARM64_X5:
        return L"x5";
    case CV_ARM64_X6:
        return L"x6";
    case CV_ARM64_X7:
        return L"x7";
    case CV_ARM64_X8:
        return L"x8";
    case CV_ARM64_X9:
        return L"x9";
    case CV_ARM64_X10:
        return L"x10";
    case CV_ARM64_X11:
        return L"x11";
    case CV_ARM64_X12:
        return L"x12";
    case CV_ARM64_X13:
        return L"x13";
    case CV_ARM64_X14:
        return L"x14";
    case CV_ARM64_X15:
        return L"x15";
    case CV_ARM64_X18:
        return L"x18";
    case CV_ARM64_X19:
        return L"x19";
    case CV_ARM64_X20:
        return L"x20";
    case CV_ARM64_X21:
        return L"x21";
    case CV_ARM64_X22:
        return L"x22";
    case CV_ARM64_X23:
        return L"x23";
    case CV_ARM64_X24:
        return L"x24";
    case CV_ARM64_X25:
        return L"x25";
    case CV_ARM64_X26:
        return L"x26";
    case CV_ARM64_X27:
        return L"x27";
    case CV_ARM64_X28:
        return L"x28";

    case CV_ARM64_FP:
        return L"fp";
    case CV_ARM64_LR:
        return L"lr";
    case CV_ARM64_SP:
        return L"sp";
    case CV_ARM64_PC:
        return L"pc";

    case CV_ARM64_CPSR:
        return L"cpsr";

    case CV_ARM64_S0:
        return L"s0";
    case CV_ARM64_S1:
        return L"s1";
    case CV_ARM64_S2:
        return L"s2";
    case CV_ARM64_S3:
        return L"s3";
    case CV_ARM64_S4:
        return L"s4";
    case CV_ARM64_S5:
        return L"s5";
    case CV_ARM64_S6:
        return L"s6";
    case CV_ARM64_S7:
        return L"s7";
    case CV_ARM64_S8:
        return L"s8";
    case CV_ARM64_S9:
        return L"s9";
    case CV_ARM64_S10:
        return L"s10";
    case CV_ARM64_S11:
        return L"s11";
    case CV_ARM64_S12:
        return L"s12";
    case CV_ARM64_S13:
        return L"s13";
    case CV_ARM64_S14:
        return L"s14";
    case CV_ARM64_S15:
        return L"s15";
    case CV_ARM64_S16:
        return L"s16";
    case CV_ARM64_S17:
        return L"s17";
    case CV_ARM64_S18:
        return L"s18";
    case CV_ARM64_S19:
        return L"s19";
    case CV_ARM64_S20:
        return L"s20";
    case CV_ARM64_S21:
        return L"s21";
    case CV_ARM64_S22:
        return L"s22";
    case CV_ARM64_S23:
        return L"s23";
    case CV_ARM64_S24:
        return L"s24";
    case CV_ARM64_S25:
        return L"s25";
    case CV_ARM64_S26:
        return L"s26";
    case CV_ARM64_S27:
        return L"s27";
    case CV_ARM64_S28:
        return L"s28";
    case CV_ARM64_S29:
        return L"s29";
    case CV_ARM64_S30:
        return L"s30";
    case CV_ARM64_S31:
        return L"s31";

    case CV_ARM64_D0:
        return L"d0";
    case CV_ARM64_D1:
        return L"d1";
    case CV_ARM64_D2:
        return L"d2";
    case CV_ARM64_D3:
        return L"d3";
    case CV_ARM64_D4:
        return L"d4";
    case CV_ARM64_D5:
        return L"d5";
    case CV_ARM64_D6:
        return L"d6";
    case CV_ARM64_D7:
        return L"d7";
    case CV_ARM64_D8:
        return L"d8";
    case CV_ARM64_D9:
        return L"d9";
    case CV_ARM64_D10:
        return L"d10";
    case CV_ARM64_D11:
        return L"d11";
    case CV_ARM64_D12:
        return L"d12";
    case CV_ARM64_D13:
        return L"d13";
    case CV_ARM64_D14:
        return L"d14";
    case CV_ARM64_D15:
        return L"d15";
    case CV_ARM64_D16:
        return L"d16";
    case CV_ARM64_D17:
        return L"d17";
    case CV_ARM64_D18:
        return L"d18";
    case CV_ARM64_D19:
        return L"d19";
    case CV_ARM64_D20:
        return L"d20";
    case CV_ARM64_D21:
        return L"d21";
    case CV_ARM64_D22:
        return L"d22";
    case CV_ARM64_D23:
        return L"d23";
    case CV_ARM64_D24:
        return L"d24";
    case CV_ARM64_D25:
        return L"d25";
    case CV_ARM64_D26:
        return L"d26";
    case CV_ARM64_D27:
        return L"d27";
    case CV_ARM64_D28:
        return L"d28";
    case CV_ARM64_D29:
        return L"d29";
    case CV_ARM64_D30:
        return L"d30";
    case CV_ARM64_D31:
        return L"d31";

    case CV_ARM64_Q0:
        return L"q0";
    case CV_ARM64_Q1:
        return L"q1";
    case CV_ARM64_Q2:
        return L"q2";
    case CV_ARM64_Q3:
        return L"q3";
    case CV_ARM64_Q4:
        return L"q4";
    case CV_ARM64_Q5:
        return L"q5";
    case CV_ARM64_Q6:
        return L"q6";
    case CV_ARM64_Q7:
        return L"q7";
    case CV_ARM64_Q8:
        return L"q8";
    case CV_ARM64_Q9:
        return L"q9";
    case CV_ARM64_Q10:
        return L"q10";
    case CV_ARM64_Q11:
        return L"q11";
    case CV_ARM64_Q12:
        return L"q12";
    case CV_ARM64_Q13:
        return L"q13";
    case CV_ARM64_Q14:
        return L"q14";
    case CV_ARM64_Q15:
        return L"q15";
    case CV_ARM64_Q16:
        return L"q16";
    case CV_ARM64_Q17:
        return L"q17";
    case CV_ARM64_Q18:
        return L"q18";
    case CV_ARM64_Q19:
        return L"q19";
    case CV_ARM64_Q20:
        return L"q20";
    case CV_ARM64_Q21:
        return L"q21";
    case CV_ARM64_Q22:
        return L"q22";
    case CV_ARM64_Q23:
        return L"q23";
    case CV_ARM64_Q24:
        return L"q24";
    case CV_ARM64_Q25:
        return L"q25";
    case CV_ARM64_Q26:
        return L"q26";
    case CV_ARM64_Q27:
        return L"q27";
    case CV_ARM64_Q28:
        return L"q28";
    case CV_ARM64_Q29:
        return L"q29";
    case CV_ARM64_Q30:
        return L"q30";
    case CV_ARM64_Q31:
        return L"q31";

    case CV_ARM64_FPSR:
        return L"fpsr";
    case CV_ARM64_FPCR:
        return L"fpcr";
    }

    std::stringstream sstr;
    sstr << "ARM64 context: unsupported register index " << std::dec << index;
    throw DbgException(sstr.str());
}

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end

///////////////////////////////////////////////////////////////////////////////
