#pragma once

#include <string>

#include "gtest/gtest.h"

#include "kdlib/dbgengine.h"
#include "kdlib/exceptions.h"

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

    static std::wstring getDumpsDirName()
    {
        return L"..\\..\\..\\kdlib\\tests\\dumps\\";
    }

    static std::wstring getKernelDumpName()
    {
        return getDumpsDirName() + L"win8_x64_mem.cab";
    }

    static std::wstring getARM64KernelDumpName()
    {
        return getDumpsDirName() + L"win10_arm64_mem.cab";
    }

    static std::wstring getARMKernelDumpName()
    {
        return getDumpsDirName() + L"win10_arm_rpi3_mem.cab";
    }

    static std::wstring getWow64UserDumpName()
    {
        return getDumpsDirName() + L"targetapp_stacktest_wow64.cab";
    }

private:

    std::wstring  m_dumpName;
    kdlib::PROCESS_DEBUG_ID  m_dumpId;

};
