#pragma once

#include <string>
#include <sstream>

#include "gtest/gtest.h"

#include "kdlib/dbgengine.h"
#include "kdlib/exceptions.h"


class MemDumps
{
public:
    static constexpr wchar_t*  STACKTEST_WOW64 = L"targetapp_stacktest_wow64";
    static constexpr wchar_t*  STACKTEST_X64_RELEASE = L"targetapp_stacktest_x64_release";
    static constexpr wchar_t*  STACKTEST_WOW64_RELEASE = L"targetapp_stacktest_wow64_release";
    static constexpr wchar_t*  STACKTEST_CV_ALLREG_I386 = L"targetapp_test_cv_allreg_i386";
    static constexpr wchar_t*  STACKTEST_CV_ALLREG_AMD64 = L"targetapp_test_cv_allreg_amd64";
    static constexpr wchar_t*  WIN10_ARM64 = L"win10_arm64_mem";
    static constexpr wchar_t*  WIN10_ARM = L"win10_arm_rpi3_mem";
    static constexpr wchar_t*  WIN8_X64 = L"win8_x64_mem";
};

inline std::wstring makeDumpDirName(const wchar_t* dumpName)
{
    static const std::wstring  dumpRoot = L"..\\..\\..\\kdlib\\tests\\dumps\\";
    return dumpRoot + dumpName;
}

inline std::wstring makeDumpFullName(const wchar_t* dumpName)
{
    static constexpr wchar_t* dumpDirRoot = L"..\\..\\..\\kdlib\\tests\\dumps\\";
    std::wstringstream dumpNameStream;
    dumpNameStream << dumpDirRoot << dumpName << L'\\' << dumpName << ".cab";
    return  dumpNameStream.str();
}

class MemDumpFixture : public ::testing::Test 
{
public:

    MemDumpFixture( const std::wstring &dumpName ) :
        m_dumpName(dumpName),
        m_dumpId(-1)
        {}

    virtual void SetUp() 
    {
        m_dumpId = kdlib::loadDump(m_dumpName);
    }

    virtual void TearDown()
    {
        try {
            if (m_dumpId != -1)
                kdlib::closeDump(m_dumpId);
            m_dumpId = -1;
        } 
        catch(kdlib::DbgException&)
        {}
    }

private:    

    std::wstring  m_dumpName;
    kdlib::PROCESS_DEBUG_ID  m_dumpId;

};
