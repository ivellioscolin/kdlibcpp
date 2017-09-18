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

typedef std::list< std::pair< std::wstring, mdFieldDef > >  NetFieldList;

class MetaDataProvider : public boost::enable_shared_from_this<MetaDataProvider>
{

    friend MetaDataProviderPtr  getMetaDataProvider(ICorDebugModule*  module);

public:


    CorElementType  getFieldElementType(mdFieldDef fieldDef);

    CorElementType getFieldSZArrayElementType(mdFieldDef fieldDef);

    DWORD  getFieldAttr(mdFieldDef fieldDef);

    mdTypeDef getFieldClassToken(mdFieldDef fieldDef);

    std::wstring  getFieldName(mdFieldDef fieldDef);

    TypeNameList getTypeNames(const std::wstring& mask);

    std::wstring getTypeNameByToken(mdTypeDef typeDef);

    mdTypeDef getTypeTokenByName(const std::wstring& typeName);

    void getFields(mdTypeDef typeDef, NetFieldList& fields);



private:

    MetaDataProvider(ICorDebugModule* module);

    CComPtr<IMetaDataImport>  m_metaDataImport;

};

///////////////////////////////////////////////////////////////////////////////

}

