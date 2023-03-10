#pragma once

#include <string>

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

bool fnmatch( const std::string& pattern, const std::string& str);

bool fnmatch( const std::wstring& pattern, const std::wstring& str);

///////////////////////////////////////////////////////////////////////////////

}
