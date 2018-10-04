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
    case CPU_ARM:
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

    case CPU_ARM:
        return CPUContextPtr( new CPUContextArm() ); 
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
    if (getLastEventThreadId() == getCurrentThreadId())
        return getStackImpl<CPUContextWOW64>();

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

    if (cpuType == IMAGE_FILE_MACHINE_ARMNT)
        return getStackImpl<CPUContextArm>();

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
    if (getLastEventThreadId() == getCurrentThreadId())
        return getStackFrameImpl<CPUContextWOW64>();

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

    if (cpuType == IMAGE_FILE_MACHINE_ARMNT)
        return getStackFrameImpl<CPUContextArm>();

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

void setCurrentStackFrame(const StackFramePtr& stackFrame)
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

} // kdlib namespace end

///////////////////////////////////////////////////////////////////////////////
