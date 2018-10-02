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

} // kdlib namespace end

///////////////////////////////////////////////////////////////////////////////
