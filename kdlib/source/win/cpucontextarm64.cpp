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
