#pragma once

#include <string>

namespace kdlib {

void dprint( const std::wstring &str, bool dml = false );
void dprintln( const std::wstring &str, bool dml = false );
std::wstring dreadline();
void eprint( const std::wstring &str );
void eprintln( const std::wstring &str );

}