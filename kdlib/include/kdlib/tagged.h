
#pragma once

#include <vector>
#include <list>

#include <guiddef.h>

#include "kdlib/dbgtypedef.h"

namespace kdlib {


///////////////////////////////////////////////////////////////////////////////

using TaggedId = GUID;
using TaggedBuffer = std::vector<unsigned char>;

std::list<TaggedId> enumTagged();

TaggedBuffer loadTaggedBuffer(TaggedId id);

///////////////////////////////////////////////////////////////////////////////

} // kdlib namespace end
