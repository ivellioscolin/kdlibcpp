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

NumVariant CPUContextArm::getRegisterByIndex(unsigned long index)
{
    switch (index)
    {
    case CV_ARM_R0:
        return NumVariant(m_context.R0);
    case CV_ARM_R1:
        return NumVariant(m_context.R1);
    case CV_ARM_R2:
        return NumVariant(m_context.R2);
    case CV_ARM_R3:
        return NumVariant(m_context.R3);
    case CV_ARM_R4:
        return NumVariant(m_context.R4);
    case CV_ARM_R5:
        return NumVariant(m_context.R5);
    case CV_ARM_R6:
        return NumVariant(m_context.R6);
    case CV_ARM_R7:
        return NumVariant(m_context.R7);
    case CV_ARM_R8:
        return NumVariant(m_context.R8);
    case CV_ARM_R9:
        return NumVariant(m_context.R9);
    case CV_ARM_R10:
        return NumVariant(m_context.R10);
    case CV_ARM_R11:
        return NumVariant(m_context.R11);
    case CV_ARM_R12:
        return NumVariant(m_context.R12);

    case CV_ARM_SP:
        return NumVariant(m_context.Sp);
    case CV_ARM_LR:
        return NumVariant(m_context.Lr);
    case CV_ARM_PC:
        return NumVariant(m_context.Pc);
    case CV_ARM_CPSR:
        return NumVariant(m_context.Cpsr);
    case CV_ARM_FPSCR:
        return NumVariant(m_context.Fpscr);

    case CV_ARM_ND0:
        return NumVariant(m_context.D[0]);
    case CV_ARM_ND1:
        return NumVariant(m_context.D[1]);
    case CV_ARM_ND2:
        return NumVariant(m_context.D[2]);
    case CV_ARM_ND3:
        return NumVariant(m_context.D[3]);
    case CV_ARM_ND4:
        return NumVariant(m_context.D[4]);
    case CV_ARM_ND5:
        return NumVariant(m_context.D[5]);
    case CV_ARM_ND6:
        return NumVariant(m_context.D[6]);
    case CV_ARM_ND7:
        return NumVariant(m_context.D[7]);
    case CV_ARM_ND8:
        return NumVariant(m_context.D[8]);
    case CV_ARM_ND9:
        return NumVariant(m_context.D[9]);
    case CV_ARM_ND10:
        return NumVariant(m_context.D[10]);
    case CV_ARM_ND11:
        return NumVariant(m_context.D[11]);
    case CV_ARM_ND12:
        return NumVariant(m_context.D[12]);
    case CV_ARM_ND13:
        return NumVariant(m_context.D[13]);
    case CV_ARM_ND14:
        return NumVariant(m_context.D[14]);
    case CV_ARM_ND15:
        return NumVariant(m_context.D[15]);
    case CV_ARM_ND16:
        return NumVariant(m_context.D[16]);
    case CV_ARM_ND17:
        return NumVariant(m_context.D[17]);
    case CV_ARM_ND18:
        return NumVariant(m_context.D[18]);
    case CV_ARM_ND19:
        return NumVariant(m_context.D[19]);
    case CV_ARM_ND20:
        return NumVariant(m_context.D[20]);
    case CV_ARM_ND21:
        return NumVariant(m_context.D[21]);
    case CV_ARM_ND22:
        return NumVariant(m_context.D[22]);
    case CV_ARM_ND23:
        return NumVariant(m_context.D[23]);
    case CV_ARM_ND24:
        return NumVariant(m_context.D[24]);
    case CV_ARM_ND25:
        return NumVariant(m_context.D[25]);
    case CV_ARM_ND26:
        return NumVariant(m_context.D[26]);
    case CV_ARM_ND27:
        return NumVariant(m_context.D[27]);
    case CV_ARM_ND28:
        return NumVariant(m_context.D[28]);
    case CV_ARM_ND29:
        return NumVariant(m_context.D[29]);
    case CV_ARM_ND30:
        return NumVariant(m_context.D[30]);
    case CV_ARM_ND31:
        return NumVariant(m_context.D[31]);

    /* NumVariant can not provide 128-bit registers :( */
    // CV_ARM_NQ0 ... CV_ARM_NQ15

    }
    std::stringstream sstr;
    sstr << "ARM context: unsupported register index " << std::dec << index;
    throw DbgException(sstr.str());
}

///////////////////////////////////////////////////////////////////////////////

std::wstring CPUContextArm::getRegisterName(unsigned long index)
{
    switch (index)
    {
    case CV_ARM_R0:
        return L"r0";
    case CV_ARM_R1:
        return L"r1";
    case CV_ARM_R2:
        return L"r2";
    case CV_ARM_R3:
        return L"r3";
    case CV_ARM_R4:
        return L"r4";
    case CV_ARM_R5:
        return L"r5";
    case CV_ARM_R6:
        return L"r6";
    case CV_ARM_R7:
        return L"r7";
    case CV_ARM_R8:
        return L"r8";
    case CV_ARM_R9:
        return L"r9";
    case CV_ARM_R10:
        return L"r10";
    case CV_ARM_R11:
        return L"r11";
    case CV_ARM_R12:
        return L"r12";

    case CV_ARM_SP:
        return L"sp";
    case CV_ARM_LR:
        return L"lr";
    case CV_ARM_PC:
        return L"pc";
    case CV_ARM_CPSR:
        return L"psr";
    case CV_ARM_FPSCR:
        return L"fpscr";

    case CV_ARM_ND0:
        return L"d0";
    case CV_ARM_ND1:
        return L"d1";
    case CV_ARM_ND2:
        return L"d2";
    case CV_ARM_ND3:
        return L"d3";
    case CV_ARM_ND4:
        return L"d4";
    case CV_ARM_ND5:
        return L"d5";
    case CV_ARM_ND6:
        return L"d6";
    case CV_ARM_ND7:
        return L"d7";
    case CV_ARM_ND8:
        return L"d8";
    case CV_ARM_ND9:
        return L"d9";
    case CV_ARM_ND10:
        return L"d10";
    case CV_ARM_ND11:
        return L"d11";
    case CV_ARM_ND12:
        return L"d12";
    case CV_ARM_ND13:
        return L"d13";
    case CV_ARM_ND14:
        return L"d14";
    case CV_ARM_ND15:
        return L"d15";
    case CV_ARM_ND16:
        return L"d16";
    case CV_ARM_ND17:
        return L"d17";
    case CV_ARM_ND18:
        return L"d18";
    case CV_ARM_ND19:
        return L"d19";
    case CV_ARM_ND20:
        return L"d20";
    case CV_ARM_ND21:
        return L"d21";
    case CV_ARM_ND22:
        return L"d22";
    case CV_ARM_ND23:
        return L"d23";
    case CV_ARM_ND24:
        return L"d24";
    case CV_ARM_ND25:
        return L"d25";
    case CV_ARM_ND26:
        return L"d26";
    case CV_ARM_ND27:
        return L"d27";
    case CV_ARM_ND28:
        return L"d28";
    case CV_ARM_ND29:
        return L"d29";
    case CV_ARM_ND30:
        return L"d30";
    case CV_ARM_ND31:
        return L"d31";

    case CV_ARM_NQ0:
        return L"q0";
    case CV_ARM_NQ1:
        return L"q1";
    case CV_ARM_NQ2:
        return L"q2";
    case CV_ARM_NQ3:
        return L"q3";
    case CV_ARM_NQ4:
        return L"q4";
    case CV_ARM_NQ5:
        return L"q5";
    case CV_ARM_NQ6:
        return L"q6";
    case CV_ARM_NQ7:
        return L"q7";
    case CV_ARM_NQ8:
        return L"q8";
    case CV_ARM_NQ9:
        return L"q9";
    case CV_ARM_NQ10:
        return L"q10";
    case CV_ARM_NQ11:
        return L"q11";
    case CV_ARM_NQ12:
        return L"q12";
    case CV_ARM_NQ13:
        return L"q13";
    case CV_ARM_NQ14:
        return L"q14";
    case CV_ARM_NQ15:
        return L"q15";
    }

    std::stringstream sstr;
    sstr << "ARM context: unsupported register index " << std::dec << index;
    throw DbgException(sstr.str());
}

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end

///////////////////////////////////////////////////////////////////////////////
