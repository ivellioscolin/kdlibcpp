
#include "stdafx.h"

#include <Windows.h>
#include <Psapi.h>

#include "kdlib/dbgengine.h"
#include "kdlib/exceptions.h"

namespace {

//////////////////////////////////////////////////////////////////////////////

std::wstring getLiveProcessName(DWORD processID)
{
    std::wstring  processName = L"<unknown>";

    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );

    if (NULL != hProcess )
    {
        std::vector<wchar_t>  nameBuffer(0x1000);
        size_t  nameLength = GetModuleBaseNameW(hProcess, NULL, &nameBuffer[0], nameBuffer.size());

        if ( nameLength > 0 )
        {
            processName = std::wstring( &nameBuffer[0], nameLength );
        }

        CloseHandle( hProcess );
    }

    return processName;
}

//////////////////////////////////////////////////////////////////////////////

std::wstring getLiveProcessUser(DWORD processID)
{
    std::wstring  fullUserName = L"<unknown>";
    HANDLE  hProcess = NULL;
    HANDLE  hToken = NULL;
    PCHAR  tokenUserBuffer = NULL;
        
    do {

       hProcess = OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, processID );
       if ( !hProcess )
           break;

       OpenProcessToken(hProcess,  TOKEN_QUERY | TOKEN_READ, &hToken);
       if ( !hToken )
           break;

       DWORD  tokenUserSize =  0;
       GetTokenInformation(hToken, TokenOwner, NULL, 0, &tokenUserSize);

       tokenUserBuffer = new char [tokenUserSize];
       PTOKEN_USER  tokenUser = reinterpret_cast<PTOKEN_USER>(tokenUserBuffer);
       if ( !GetTokenInformation(hToken, TokenOwner, tokenUserBuffer, tokenUserSize, &tokenUserSize) )
           break;

       WCHAR  userName[100];
       DWORD  userNameLength = sizeof(userName)/sizeof(WCHAR);
       WCHAR  domainName[100];
       DWORD  domainNameLength = sizeof(domainName)/sizeof(WCHAR);
       SID_NAME_USE  sidNameUse;
       if ( !LookupAccountSidW(
           NULL, 
           tokenUser->User.Sid, 
           userName,
           &userNameLength, 
           domainName,
           &domainNameLength,
           &sidNameUse ) )
             break;

       fullUserName = std::wstring(domainName) + L"@" + std::wstring(userName);

    } while(FALSE);

    if ( tokenUserBuffer )
        delete[] tokenUserBuffer;

    if ( hProcess != NULL )
        CloseHandle(hProcess);

    if ( hToken != NULL )
        CloseHandle(hToken);

    return fullUserName;
}

//////////////////////////////////////////////////////////////////////////////

}

namespace  kdlib {

//////////////////////////////////////////////////////////////////////////////

unsigned long getNumberLiveProcesses()
{
    DWORD  bytesNeeded;
    DWORD  processIds[0x1000];

    EnumProcesses(processIds, sizeof(processIds), &bytesNeeded);

    return bytesNeeded/sizeof(DWORD);
}

//////////////////////////////////////////////////////////////////////////////

void getLiveProcessesList(std::vector<LiveProcessInfo> &processInfo)
{
    DWORD  bytesNeeded;
    DWORD  processNumber;
    std::vector<DWORD>  processIds(0x1000);

    if ( !EnumProcesses( &processIds[0], processIds.size(), &bytesNeeded ) )
    {
        processIds.resize(bytesNeeded/sizeof(DWORD)*2);

        if ( !EnumProcesses( &processIds[0], processIds.size(), &bytesNeeded ))
        {
            throw DbgException("failed to enum process");
        }
    }

    processNumber = bytesNeeded / sizeof(DWORD);

    for ( size_t i = 0; i < processNumber; ++i)
    {
        LiveProcessInfo  pinfo;

        pinfo.pid = processIds[i];
        pinfo.name = getLiveProcessName(pinfo.pid);
        pinfo.user = getLiveProcessUser(pinfo.pid);

        processInfo.push_back(pinfo);
    }
}

}
