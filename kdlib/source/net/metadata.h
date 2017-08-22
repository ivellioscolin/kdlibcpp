#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "net/net.h"
#include <cor.h>

#include "kdlib/exceptions.h"
#include "kdlib/typeinfo.h"
#include "kdlib/module.h"

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

class MetaDataProvider;
typedef boost::shared_ptr<MetaDataProvider>   MetaDataProviderPtr;

MetaDataProviderPtr  getMetaDataProvider(ICorDebugModule*  module);

class MetaDataProvider : public boost::enable_shared_from_this<MetaDataProvider>
{

    friend MetaDataProviderPtr  getMetaDataProvider(ICorDebugModule*  module);

public:

    TypeInfoPtr  getTypeByToken(mdTypeDef typeDef);

    TypeNameList getTypeNames(const std::wstring& mask);

    std::wstring getTypeNameByToken(mdTypeDef typeDef);

private:

    MetaDataProvider(ICorDebugModule* module);

    CComPtr<IMetaDataImport>  m_metaDataImport;

};

///////////////////////////////////////////////////////////////////////////////

}

