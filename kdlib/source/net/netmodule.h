#pragma once

#include <boost/enable_shared_from_this.hpp>

#include <atlbase.h>
#include <CorHdr.h>
#include <CorDebug.h>

#include "moduleimp.h"

#include "net/metadata.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class NetModule : public ModuleImp, public boost::enable_shared_from_this<NetModule>
{

    friend ModulePtr loadNetModule( MEMOFFSET_64 imageBase );

protected:

    explicit NetModule( MEMOFFSET_64 imageBase ) : ModuleImp(imageBase)
    {}

    explicit NetModule(ICorDebugModule*  module) : 
        ModuleImp(getImageBase(module)),
        m_corDebugModule(module)
    {}


protected:

    bool isManaged() const
    {
        return true;
    }

    TypeNameList NetModule::enumTypes(const std::wstring& mask);

protected:

    static
    MEMOFFSET_64  getImageBase(ICorDebugModule* module);

    void initCorDebugModule();

    CComPtr<ICorDebugModule>  m_corDebugModule;
    MetaDataProviderPtr  m_metaDataProvider;
};

///////////////////////////////////////////////////////////////////////////////

}
