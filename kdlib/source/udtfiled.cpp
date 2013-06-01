#include "stdafx.h"

#include <sstream>

#include "kdlib/exceptions.h"

#include "udtfield.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

const UdtFieldPtr& FieldCollection::lookup(const std::wstring &name) const
{
    FieldList::const_reverse_iterator it;
    for ( it = m_fields.rbegin(); it !=  m_fields.rend(); ++it )
    {
        if ( (*it)->getName() == name )
            return *it;
    }

    std::wstringstream   sstr;
    sstr << L"field \"" << name << L" not found";

    throw TypeException( m_name, sstr.str() );
}

///////////////////////////////////////////////////////////////////////////////

UdtFieldPtr& FieldCollection::lookup(const std::wstring &name)
{
    const UdtFieldPtr &filedPtr = const_cast<const FieldCollection&>(*this).lookup(name);
    return const_cast<UdtFieldPtr&>(filedPtr);
}


///////////////////////////////////////////////////////////////////////////////

const UdtFieldPtr& FieldCollection::lookup(size_t index) const
{
    if ( index >= m_fields.size() )
        throw IndexException( index );

    return m_fields[index];
}

//////////////////////////////////////////////////////////////////////////////

UdtFieldPtr& FieldCollection::lookup(size_t index)
{
    const UdtFieldPtr &filedPtr = const_cast<const FieldCollection&>(*this).lookup(index);
    return const_cast<UdtFieldPtr&>(filedPtr);
}

//////////////////////////////////////////////////////////////////////////////

TypeInfoPtr SymbolUdtField::getTypeInfo()
{
    return loadType(m_symbol);
}

///////////////////////////////////////////////////////////////////////////////

} // end kdlib namespace