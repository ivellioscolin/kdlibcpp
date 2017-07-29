#pragma once

#include <atlbase.h>

#include "net.h"

#include "moduleimp.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class NetModule : public ModuleImp
{
    friend ModulePtr loadNetModule( MEMOFFSET_64 imageBase );

protected:

    explicit NetModule( MEMOFFSET_64 imageBase ) : ModuleImp(imageBase)
    {}

protected:

    bool isManaged() const
    {
        return true;
    }

    TypeNameList enumTypes(const std::wstring& mask = L"*");

protected:

    IXCLRDataModule*  get();

    CComPtr<IXCLRDataModule>  m_xclrDataModule;
};

///////////////////////////////////////////////////////////////////////////////

}