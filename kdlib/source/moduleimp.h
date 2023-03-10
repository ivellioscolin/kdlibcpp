
#include <boost\thread\recursive_mutex.hpp>
#include <boost\enable_shared_from_this.hpp>

#include "kdlib\module.h"
#include "kdlib\exceptions.h"

namespace kdlib {

struct ModuleCacheKey {

    PROCESS_DEBUG_ID  processId;
    MEMOFFSET_64  moduleBase;

    ModuleCacheKey( PROCESS_DEBUG_ID p, MEMOFFSET_64 m ) :
        processId(p), moduleBase(m)
        {}
};

inline bool operator< ( const ModuleCacheKey &key1, const ModuleCacheKey &key2 ) 
{
    if ( key1.processId < key2.processId )
        return true;
    if ( key1.processId > key2.processId)
        return false;
    return key1.moduleBase < key2.moduleBase;
}

ModulePtr loadNetModule( MEMOFFSET_64 offset );

class ModuleBaseImp : public Module {

protected:

    virtual std::wstring  getName() 
    {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64  getBase() const 
    {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64  getEnd() const
    {
        NOT_IMPLEMENTED();
    }

    virtual size_t getSize() const
    {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getImageName() const
    {
        NOT_IMPLEMENTED();
    }
    
    virtual unsigned long getCheckSum() const
    {
        NOT_IMPLEMENTED();
    }

    virtual unsigned long getTimeDataStamp() const
    {
        NOT_IMPLEMENTED();
    }

    virtual bool isUnloaded() const
    {
        NOT_IMPLEMENTED();
    }

    virtual bool isUserMode() const
    {
        NOT_IMPLEMENTED();
    }

    virtual bool isSymbolLoaded() const
    {
        NOT_IMPLEMENTED();
    }

    virtual bool isManaged() const
    {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getSymFile() 
    {
        NOT_IMPLEMENTED();
    }

    virtual void reloadSymbols()
    {
        NOT_IMPLEMENTED();
    }

    virtual void resetSymbols() 
    {
        NOT_IMPLEMENTED();
    }

    virtual MEMOFFSET_64 getSymbolVa( const std::wstring &symbolName )
    {
        NOT_IMPLEMENTED();
    }
    
    virtual MEMOFFSET_32 getSymbolRva( const std::wstring &symbolName )
    {
        NOT_IMPLEMENTED();
    }

    virtual SymbolPtr getSymbolByVa( MEMOFFSET_64 offset, MEMDISPLACEMENT* displacement = 0 )
    {
        NOT_IMPLEMENTED();
    }

    virtual SymbolPtr getSymbolByVa( MEMOFFSET_64 offset, unsigned long  symTag, MEMDISPLACEMENT* displacement ) 
    {
        NOT_IMPLEMENTED();
    }

    virtual SymbolPtr getSymbolByRva( MEMOFFSET_32 offset, MEMDISPLACEMENT* displacement = 0 )
    {
        NOT_IMPLEMENTED();
    }

    virtual SymbolPtr getSymbolByRva( MEMOFFSET_32 offset, unsigned long  symTag, MEMDISPLACEMENT* displacement )
    {
        NOT_IMPLEMENTED();
    }

    virtual SymbolPtr getSymbolByName( const std::wstring &symbolName )
    {
        NOT_IMPLEMENTED();
    }

    virtual SymbolPtr getSymbolScope() 
    {
        NOT_IMPLEMENTED();
    }

    virtual size_t getSymbolSize( const std::wstring &symName )
    {
        NOT_IMPLEMENTED();
    }

    virtual TypeInfoPtr getTypeByName( const std::wstring &typeName )
    {
        NOT_IMPLEMENTED();
    }

    virtual TypedVarPtr getTypedVarByAddr( MEMOFFSET_64 offset )
    {
        NOT_IMPLEMENTED();
    }

    virtual TypedVarPtr getTypedVarByName( const std::wstring &symName )
    {
        NOT_IMPLEMENTED();
    }

    virtual TypedVarPtr getTypedVarByTypeName( const std::wstring &typeName, MEMOFFSET_64 addr )
    {
        NOT_IMPLEMENTED();
    }

    virtual TypedVarPtr getTypedVarWithPrototype( const std::wstring &symName, const std::wstring &prototype)
    {
        NOT_IMPLEMENTED();
    }

    virtual TypedVarPtr containingRecord( MEMOFFSET_64 offset, const std::wstring &typeName, const std::wstring &fieldName ) 
    {
        NOT_IMPLEMENTED();
    }
    
    virtual TypedVarPtr getFunctionByAddr( MEMOFFSET_64 offset )
    {
        NOT_IMPLEMENTED();
    }

    virtual TypedVarList loadTypedVarList( MEMOFFSET_64 addr, const std::wstring &typeName, const std::wstring &fieldName )
    {
        NOT_IMPLEMENTED();
    }

    virtual TypedVarList loadTypedVarArray( MEMOFFSET_64 addr, const std::wstring &typeName, size_t count )
    {
        NOT_IMPLEMENTED();
    }

    virtual SymbolOffsetList  enumSymbols( const std::wstring  &mask = L"*" )
    {
        NOT_IMPLEMENTED();
    }

    virtual TypeNameList enumTypes(const std::wstring  &mask = L"*")
    {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring findSymbol( MEMOFFSET_64 offset, MEMDISPLACEMENT &displacement )
    {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring getSourceFile( MEMOFFSET_64 offset )
    {
        NOT_IMPLEMENTED();
    }

    virtual std::wstring  getSourceFileFromSrcSrv(MEMOFFSET_64 offset)
    {
        NOT_IMPLEMENTED();
    }

    virtual void getSourceLine( MEMOFFSET_64 offset, std::wstring &fileName, unsigned long &lineno, long &displacement ) 
    {
        NOT_IMPLEMENTED();
    }

    virtual std::string getVersionInfo( const std::string &value )
    {
        NOT_IMPLEMENTED();
    }

    virtual void getFileVersion(unsigned long& majorVersion, unsigned long& minorVerion, unsigned long& revision, unsigned long& build)
    {
        NOT_IMPLEMENTED();
    }

    virtual void getFixedFileInfo( FixedFileInfo &fixedFileInfo )
    {
        NOT_IMPLEMENTED();
    }

    virtual ScopePtr getScope()
    {
        NOT_IMPLEMENTED();
    }
};



class ModuleImp : public ModuleBaseImp, public boost::enable_shared_from_this<ModuleImp> {

    friend ModulePtr loadModule(MEMOFFSET_64 offset);

protected:

    explicit ModuleImp(MEMOFFSET_64 offset);

protected:

    std::wstring  getName() {
        return m_name;
    }

    MEMOFFSET_64  getBase() const {
        return m_base;
    }

    MEMOFFSET_64  getEnd() const {
        return m_base + m_size;
    }

    size_t getSize() const {
        return m_size;
    }

    std::wstring getImageName() const {
        return m_imageName;
    }

    unsigned long getCheckSum() const {
        return m_checkSum;
    }

    unsigned long getTimeDataStamp() const {
        return m_timeDataStamp;
    }

    bool isUnloaded() const {
        return m_isUnloaded;
    }

    bool isUserMode() const {
        return m_isUserMode;
    }

    bool isManaged() const
    {
        return false;
    }

    std::wstring getSymFile();

    void
        reloadSymbols();

    void
        resetSymbols()
    {
        m_symSession.reset();
    }

    bool isSymbolLoaded() const
    {
        return !m_exportSymbols && !m_noSymbols;
    }

    MEMOFFSET_64 getSymbolVa(const std::wstring &symbolName);
    MEMOFFSET_32 getSymbolRva(const std::wstring &symbolName);

    SymbolPtr getSymbolByVa(MEMOFFSET_64 offset, MEMDISPLACEMENT* displacement = 0);
    SymbolPtr getSymbolByVa(MEMOFFSET_64 offset, unsigned long  symTag, MEMDISPLACEMENT* displacement);
    SymbolPtr getSymbolByRva(MEMOFFSET_32 offset, MEMDISPLACEMENT* displacement = 0);
    SymbolPtr getSymbolByRva(MEMOFFSET_32 offset, unsigned long  symTag, MEMDISPLACEMENT* displacement);

    SymbolPtr getSymbolByName(const std::wstring &symbolName);

    SymbolPtr getSymbolScope();

    size_t getSymbolSize(const std::wstring &symName);

    TypeInfoPtr getTypeByName(const std::wstring &typeName)
    {
        return loadType(getSymbolScope(), typeName);
    }

    TypedVarPtr getTypedVarByAddr( MEMOFFSET_64 offset );
    TypedVarPtr getTypedVarByName( const std::wstring &symName );
    TypedVarPtr getTypedVarByTypeName( const std::wstring &typeName, MEMOFFSET_64 addr );
    TypedVarPtr getTypedVarWithPrototype( const std::wstring &symName, const std::wstring &prototype);
    TypedVarPtr containingRecord( MEMOFFSET_64 offset, const std::wstring &typeName,  const std::wstring &fieldName );
    TypedVarPtr getFunctionByAddr( MEMOFFSET_64 offset );

    TypedVarList loadTypedVarList( MEMOFFSET_64 addr, const std::wstring &typeName, const std::wstring &fieldName );
    TypedVarList loadTypedVarArray( MEMOFFSET_64 addr, const std::wstring &typeName, size_t count );

    SymbolOffsetList  enumSymbols( const std::wstring  &mask = L"*" );

    TypeNameList enumTypes(const std::wstring& mask = L"*");

    std::wstring findSymbol( MEMOFFSET_64 offset, MEMDISPLACEMENT &displacement );

    std::wstring getSourceFile( MEMOFFSET_64 offset );

    std::wstring  getSourceFileFromSrcSrv(MEMOFFSET_64 offset);

    void getSourceLine( MEMOFFSET_64 offset, std::wstring &fileName, unsigned long &lineno, long &displacement );

    std::string getVersionInfo( const std::string &value );

    void getFileVersion(unsigned long& majorVersion, unsigned long& minorVerion, unsigned long& revision, unsigned long& build);

    void getFixedFileInfo( FixedFileInfo &fixedFileInfo );

    virtual ScopePtr getScope();

protected:

    void fillFields(); // ctor-helper

    SymbolSessionPtr& getSymSession();

    bool inRange(MEMOFFSET_64 offset) const {
        return (offset >= m_base) && (offset < (m_base + m_size));
    }

    virtual NumVariant getValue() const {
        return NumVariant( m_base );
    }

    void findSymSessionSymbol(MEMOFFSET_64 offset, std::wstring &name, MEMDISPLACEMENT &displacement);

    std::wstring  m_name;
    std::wstring  m_imageName;
    MEMOFFSET_64  m_base;
    size_t  m_size;
    unsigned long  m_timeDataStamp;
    unsigned long  m_checkSum;
    SymbolSessionPtr  m_symSession;
    bool m_isUnloaded;
    bool m_isUserMode;
    bool m_exportSymbols;
    bool m_noSymbols;
};




} // namesapce kdlib

