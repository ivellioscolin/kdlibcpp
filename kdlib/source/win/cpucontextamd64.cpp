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

} // kdlib namespace end

///////////////////////////////////////////////////////////////////////////////
