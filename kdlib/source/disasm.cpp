#include "stdafx.h"

#include "kdlib/disasm.h"
#include "kdlib/disasmengine.h"
#include "kdlib/dbgengine.h"
#include "kdlib/memaccess.h"

namespace kdlib {

/////////////////////////////////////////////////////////////////////////////////

Disasm::Disasm( MEMOFFSET_64 offset )
    : m_ea(0)
{
    m_beginOffset = addr64(offset);

    if ( m_beginOffset == 0 )
        m_beginOffset = getInstructionOffset();

    m_currentOffset = m_beginOffset;

    doDisasm();
}

/////////////////////////////////////////////////////////////////////////////////

void Disasm::doDisasm()
{
    MEMOFFSET_64     endOffset = 0;

    disasmDisassembly( m_currentOffset, m_disasm, endOffset, m_ea );

    m_length = (size_t)(endOffset - m_currentOffset);
}

/////////////////////////////////////////////////////////////////////////////////

std::wstring Disasm::assembly( const std::wstring &instr )
{
    MEMOFFSET_64     endOffset = 0;
    disasmAssemblay( m_currentOffset, instr, endOffset );

    m_currentOffset = endOffset;

    doDisasm();
    
    return m_disasm;
}

/////////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 Disasm::getNearInstruction( MEMDISPLACEMENT delta ) {
    return kdlib::getNearInstruction( m_currentOffset, delta );
}

/////////////////////////////////////////////////////////////////////////////////

}; // end pykd namespace