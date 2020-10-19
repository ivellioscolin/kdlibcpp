#pragma once

#include <map>

#include <dia2.h>
#include <atlbase.h>

#include "kdlib/symengine.h"
#include "kdlib/exceptions.h"

namespace kdlib {

//////////////////////////////////////////////////////////////////////////////

typedef CComPtr< IDiaSymbol > DiaSymbolPtr;
typedef CComPtr< IDiaEnumSymbols > DiaEnumSymbolsPtr;
typedef CComPtr< IDiaDataSource > DiaDataSourcePtr;
typedef CComPtr< IDiaSession > DiaSessionPtr;
typedef CComPtr< IDiaEnumSymbolsByAddr > DiaEnumSymbolsByAddrPtr;
typedef CComPtr< IDiaEnumLineNumbers > DiaEnumLineNumbersPtr;
typedef CComPtr< IDiaLineNumber> DiaLineNumberPtr;
typedef CComPtr< IDiaSourceFile > DiaSourceFilePtr;

typedef std::map<ULONG, ULONG> DiaRegToRegRelativeBase;

//////////////////////////////////////////////////////////////////////////////

class DiaException : public SymbolException {
public:
    DiaException(const std::wstring &desc, HRESULT hres, IDiaSymbol *symbol = NULL)
        : SymbolException( makeFullDesc(desc, hres, symbol) )
        , m_hres(hres)
    {
    }

    DiaException(const std::wstring &desc)
        : SymbolException(descPrefix + desc)
        , m_hres(S_FALSE)
    {
    }

    HRESULT getRes() const {
        return m_hres;
    }
private:

    static const std::wstring descPrefix;

    static std::wstring makeFullDesc(const std::wstring &desc, HRESULT hres, IDiaSymbol *symbol = NULL);

    HRESULT m_hres;
};

////////////////////////////////////////////////////////////////////////////

class DiaSymbol : public Symbol {
public:
    DiaSymbol(const  DiaSymbolPtr &_symbol, const std::wstring &_scope, MachineTypes machineType );

    static SymbolPtr fromGlobalScope( IDiaSymbol *_symbol, const std::wstring &_scope );

    SymbolPtr getChildByName(const std::wstring &_name ) override;

    ULONG getRva() override;
     
    SymbolPtrList findChildren(
        ULONG symTag,
        const std::wstring &name = L"",
        bool caseSensitive = FALSE
    ) override;

    SymbolPtrList findChildrenByRVA(unsigned long symTag, unsigned long rva) override;

    size_t getSize() override;

    std::wstring getName() override;
    std::wstring getScopeName() override;

    SymbolPtr getType() override;

    SymbolPtr getIndexType() override;

    SymTags getSymTag() override;

    ULONGLONG getVa() override;

    ULONG getLocType() override;

    LONG getOffset() override;

    size_t getCount() override;

    void getValue( NumVariant &vtValue ) override;

    bool isBasicType() override;

    bool isIndirectVirtualBaseClass() override;

    bool isVirtualBaseClass() override;

    bool isVirtual() override;

    ULONG getBaseType() override;

    ULONG getBitPosition() override;

    ULONG getUdtKind() override;

    ULONG getDataKind() override;

    ULONG getRegisterId() override;

    unsigned long getRegRelativeId() override;

    SymbolPtr getObjectPointerType() override;

    ULONG getCallingConvention() override;

    SymbolPtr getClassParent() override;

    MachineTypes getMachineType() override {
        return m_machineType;
    }

    size_t getChildCount( ULONG symTag ) override;

    size_t getChildCount() override
    {
        return getChildCount(SymTagNull);
    }

    SymbolPtr getChildByIndex(ULONG symTag, ULONG _index ) override;

    SymbolPtr getChildByIndex(ULONG _index ) override
    {
        return getChildByIndex( SymTagNull, _index );
    }
    
    bool isConstant() override;

    int getVirtualBasePointerOffset() override;

    ULONG getVirtualBaseDispIndex() override;

    ULONG getVirtualBaseDispSize() override;

    SymbolPtr getVirtualTableShape() override;

    unsigned long getVirtualBaseOffset() override;

    SymbolPtrList findInlineFramesByVA(MEMOFFSET_64 va) override;

    void getInlineSourceLine(MEMOFFSET_64, std::wstring &fileName, unsigned long &lineNo) override;

    SymbolPtr getLexicalParent() override;

public:
    typedef std::pair<ULONG, const wchar_t *> ValueNameEntry;
    static const ValueNameEntry basicTypeName[];
    static const size_t cntBasicTypeName;

    //static std::string getBasicTypeName( ULONG basicType );

protected:

    ULONG getRegRelativeIdImpl(const DiaRegToRegRelativeBase &DiaRegToRegRelative);

    bool isUndecorated(const std::wstring &undecName);
    
    template<typename TRet>
    struct ReturnType {
        typedef TRet type;
    };

    template <>
    struct ReturnType<IDiaSymbol*>{
        typedef CComPtr<IDiaSymbol> type;
    };


    template <typename TRet>
    typename ReturnType<TRet>::type callSymbolT(
        HRESULT(STDMETHODCALLTYPE IDiaSymbol::*method)(TRet *),
        const wchar_t *methodName
     )
    {
        ReturnType<TRet>::type retValue;
        HRESULT hres = (m_symbol->*method)(&retValue);
        if (S_OK != hres)
            throw DiaException(std::wstring(L"Call IDiaSymbol::") + methodName, hres, m_symbol);
        return retValue;
    }

    DiaSymbolPtr m_symbol;
    std::wstring m_scope;

    MachineTypes m_machineType;
};

////////////////////////////////////////////////////////////////////////////

class DiaSession : public SymbolSession
{
public:

    DiaSession( IDiaSession* session, IDiaSymbol *globalScope, const std::wstring symbolFile ) :
        m_globalScope( globalScope ),
        m_globalSymbol( DiaSymbol::fromGlobalScope( globalScope, getScopeName( session, globalScope ) ) ),
        m_session( session ),
        m_symbolFileName( symbolFile )
        {}

    virtual SymbolPtr getSymbolScope() {
        return m_globalSymbol;
    }

    virtual SymbolPtr findByRva( ULONG rva, ULONG symTag = SymTagNull, LONG* displacement = NULL );

    virtual void getSourceLine( ULONG64 offset, std::wstring &fileName, ULONG &lineNo, LONG &displacement );

    virtual std::wstring getSymbolFileName() {
        return m_symbolFileName;
    }

private:

    static std::wstring getScopeName( IDiaSession* session, IDiaSymbol *globalScope );

    ULONG findRvaByName( const std::string &name );

    DiaSymbolPtr    m_globalScope;
    SymbolPtr       m_globalSymbol;
    DiaSessionPtr   m_session;
    std::wstring     m_symbolFileName;

};

////////////////////////////////////////////////////////////////////////////

} // end kdlib namespace

