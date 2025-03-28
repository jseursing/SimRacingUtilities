#include "Interpreter.h"
#include "MemUtils.h"
#include "Win32Sem.h"
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>

/**
 * Function: IsElevated
 * Notes: See header file
 */
bool MemUtils::IsElevated()
{
  bool elevated = false;
  HANDLE hToken = INVALID_HANDLE_VALUE;
  if (TRUE == OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
  {
    TOKEN_ELEVATION elevation;
    unsigned long size = sizeof(TOKEN_ELEVATION);
    if (TRUE == GetTokenInformation(hToken,
                                    TokenElevation,
                                    &elevation,
                                    sizeof(elevation),
                                    &size))
    {
      elevated = elevation.TokenIsElevated;
    }

    CloseHandle(hToken);
  }

  return elevated;
}

/**
 * Function: EnableTokenPrivs
 * Notes: See header file
 */
bool MemUtils::EnableTokenPrivs()
{
  HANDLE hToken = INVALID_HANDLE_VALUE;
  if (FALSE == OpenProcessToken(GetCurrentProcess(),
                                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                &hToken))
  {
    return false;
  }

  LUID luid;
  if (FALSE == LookupPrivilegeValueA(0, "SeCreateGlobalPrivilege", &luid))
  {
    return false;
  }

  TOKEN_PRIVILEGES privs;
  privs.PrivilegeCount = 1;
  privs.Privileges[0].Luid = luid;
  privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  bool success = AdjustTokenPrivileges(hToken,
                                       FALSE,
                                       &privs,
                                       sizeof(TOKEN_PRIVILEGES),
                                       0,
                                       0);
  CloseHandle(hToken);

  return success;
}

/**
 * Function: EnumerateProcesses
 * Notes: See header file
 */
void MemUtils::EnumerateProcesses(std::map<uint32_t, std::string>& procMap)
{
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (INVALID_HANDLE_VALUE != hSnapshot)
  {
    PROCESSENTRY32 pe32;
    memset(&pe32, 0, sizeof(PROCESSENTRY32));
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    if (true == Process32First(hSnapshot, &pe32))
    {
      do
      {
        // Check the owner. Do not view system processes..
        bool logProcess = false;
        HANDLE procHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
        if (INVALID_HANDLE_VALUE != procHandle)
        {
          HANDLE tokenHandle = INVALID_HANDLE_VALUE;
          if (TRUE == OpenProcessToken(procHandle, TOKEN_QUERY, &tokenHandle))
          {
            unsigned long bufSize = 512;
            TOKEN_USER* tokenUser = reinterpret_cast<TOKEN_USER*>(new char[bufSize]);
            if (TRUE == GetTokenInformation(tokenHandle, TokenUser, tokenUser, bufSize, &bufSize))
            {
              SID_NAME_USE sidType;
              char domain[256] = { 0 };
              char username[256] = { 0 };
              unsigned long domainLen = sizeof(domain);
              unsigned long userNameLen = sizeof(username);
              if (TRUE == LookupAccountSidA(nullptr, 
                                            tokenUser->User.Sid, 
                                            username, 
                                            &userNameLen,
                                            domain, 
                                            &domainLen,
                                            &sidType))  
              {
                logProcess = true;
              }
            }
        
            CloseHandle(tokenHandle);
          }

          CloseHandle(procHandle);
        }

        if (true == logProcess)
        {
          std::string filename = "";
          for (size_t j = 0; j < lstrlenW(pe32.szExeFile); ++j)
          {
            filename += static_cast<char>(pe32.szExeFile[j] & 0xFF);
          }

          procMap[pe32.th32ProcessID] = filename;
        }
      }
      while (true == Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
  }
}

/**
 * Function: MapAddress
 * Notes: See header file
 */
uintptr_t MemUtils::MapAddress(std::string strValue)
{
  uintptr_t baseAddress = 0;
  uintptr_t offset      = 0;  

  std::string strBaseAddr = strValue;
  std::string strOffset   = "";

  // Check for a base address + offset
  size_t tokenPos = strValue.find("+");
  if (std::string::npos != tokenPos)
  {
    strBaseAddr = strValue.substr(0, tokenPos);
    strOffset   = strValue.substr(tokenPos + 1);
    offset = Interpreter::ToInteger(strOffset);
  }

  // Check if the base address is a module
  tokenPos = strBaseAddr.find(".");
  if (std::string::npos != tokenPos)
  {
    // Map the module to base address
    for (size_t i = 0; i < ModuleList.size(); ++i)
    {
      if (std::string::npos != ModuleList[i].ModuleName.find(strBaseAddr))
      {
        baseAddress = ModuleList[i].BaseAddress;
        break;
      }
    }
  }

  if (0 == baseAddress)
  {
    baseAddress = Interpreter::ToInteger(strBaseAddr);
  }

  return baseAddress + offset;
}

/**
 * Function: GetModuleList
 * Notes: See header file
 */
void MemUtils::GetModuleList(std::vector<ModuleEntry>& moduleList)
{
  moduleList = ModuleList;
}

/**
 * Function: GetModule
 * Notes: See header file
 */
void MemUtils::GetModule(uintptr_t address, std::string& module, uintptr_t& moduleAddress)
{
  module = "";
  moduleAddress = 0;
  for (size_t i = 0; i < ModuleList.size(); ++i)
  {
    if ((address >= ModuleList[i].BaseAddress) &&
        (address <= (ModuleList[i].BaseAddress + ModuleList[i].ModuleSize)))
    {
      module = ModuleList[i].ModuleName;
      moduleAddress = ModuleList[i].BaseAddress;
      break;
    }
  }
}

/**
 * Function: GetProcessHandle
 * Notes: See header file
 */
void* MemUtils::GetProcessHandle(std::string executable)
{
  HANDLE procHandle = INVALID_HANDLE_VALUE;

  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (INVALID_HANDLE_VALUE != hSnapshot)
  {
    PROCESSENTRY32 pe32;
    memset(&pe32, 0, sizeof(PROCESSENTRY32));
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (TRUE == Process32First(hSnapshot, &pe32))
    {
      do
      {
        std::string filename = "";
        for (size_t j = 0; j < lstrlenW(pe32.szExeFile); ++j)
        {
          filename += static_cast<char>(pe32.szExeFile[j] & 0xFF);
        }

        if (0 == filename.compare(executable.c_str()))
        {
          procHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
          break;
        }

      } while (TRUE == Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
  }

  return procHandle;
}

/**
 * Function: SetStatusCallback
 * Notes: See header file
 */
void MemUtils::SetStatusCallback(fnStatusCallback cb)
{
  StatusCallback = cb;
  if (nullptr != StatusCallback)
  {
    //if (false == IsElevated())
    //{
    //  SendStatus("[WARNING] Executing without admin privileges. Telemetry support may be limited.");
    //}

    if (false == EnableTokenPrivs())
    {
      SendStatus("[WARNING] Failed updating process token privileges");
    }
  }
}

/**
 * Function: SetActiveProcess
 * Notes: See header file
 */
bool MemUtils::SetActiveProcess(void* processHandle)
{
  bool refreshModules = false;
  if (ActiveProcess != processHandle)
  {
    refreshModules = true;
    if (INVALID_HANDLE_VALUE != ActiveProcess)
    {
      CloseHandle(ActiveProcess);
    }

    ActiveProcess = processHandle;
  }

  if (true == refreshModules)
  {
    MemoryPages.clear();
    ModuleList.clear();

    MEMORY_BASIC_INFORMATION mbi;
    for (char* pAddress = nullptr;
         sizeof(mbi) == VirtualQueryEx(ActiveProcess, pAddress, &mbi, sizeof(mbi));
         pAddress += mbi.RegionSize)
    {
      if ((MEM_COMMIT == mbi.State) &&
          (0 == (mbi.Protect & PAGE_GUARD) &&
          (0 == (mbi.Protect & PAGE_NOACCESS))))
      {
        MemPageEntry entry;
        entry.BaseAddress = mbi.BaseAddress;
        entry.RegionSize  = mbi.RegionSize;
        entry.PageProtect = mbi.Protect;
        MemoryPages.emplace_back(entry);
      }
    }

    if (0 != MemoryPages.size())
    {
      HANDLE moduleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(processHandle));
      if (INVALID_HANDLE_VALUE != moduleSnap)
      {
        MODULEENTRY32 me32;
        me32.dwSize = sizeof(MODULEENTRY32);
        if (TRUE == Module32First(moduleSnap, &me32))
        {
          do
          {
            ModuleEntry me;
            me.BaseAddress = reinterpret_cast<uintptr_t>(me32.modBaseAddr);
            me.ModuleSize  = me32.modBaseSize;
            me.ModuleName  = "";
            for (size_t i = 0; i < lstrlen(me32.szExePath); ++i)
            { 
              me.ModuleName += static_cast<char>(me32.szExePath[i] & 0xFF);
            }

            ModuleList.emplace_back(me);
          }
          while (Module32Next(moduleSnap, &me32));
        }

        CloseHandle(moduleSnap);
      }

      QuarterProgress = MemoryPages.size() / 4;
    }
  }

  return (0 != ModuleList.size());
}

/**
 * Function: GetActiveProcess
 * Notes: See header file
 */
void* MemUtils::GetActiveProcess() const
{
  return ActiveProcess;
}

/**
 * Function: NewScan
 * Notes: See header file
 */
bool MemUtils::NewScan(ScanTypeEnum type, 
                          ScanModifierEnum modifier, 
                          void* value, 
                          size_t valueSize, 
                          fnScanCompleteCallback cb)
{
  char dbgStr[256] = { 0 };

  if ((0 == MemoryPages.size()) ||
      (nullptr == value) ||
      (0 == valueSize))
  {
    SendStatus("[ERROR] Scanner was not initialized properly.");
    return false;
  }

  if (nullptr != ScanValuePtr)
  {
    free(ScanValuePtr);
    ScanValuePtr = nullptr;
  }

  // Check status of all scanning threads
  if (0 != ActiveScanners)
  {
    SendStatus("[WARNING] Scanner is still active, try again later.");
    return false;
  }

  IsNewScan     = true;
  ScanType      = type;
  ScanModifier  = modifier;
  ScanValueSize = valueSize;
  ScanValuePtr  = malloc(ScanValueSize);
  memcpy(ScanValuePtr, value, ScanValueSize);
  ScanCompleteCallback = cb;

  // Spawn tasks
  TaskParams taskParams;
  taskParams.ThisInstance = this;
  
  ActiveScanners = MemoryPages.size();
  for (size_t i = 0; i < MemoryPages.size(); ++i)
  {
    taskParams.InitDone = false;
    taskParams.MemPage  = &MemoryPages[i];
    MemoryPages[i].TaskHandle = CreateThread(nullptr,
                                             0, 
                                             reinterpret_cast<LPTHREAD_START_ROUTINE>(ScannerTask), 
                                             &taskParams, 
                                             0, 
                                             0);
    if (INVALID_HANDLE_VALUE == MemoryPages[i].TaskHandle)
    {
      sprintf_s(dbgStr, "[WARNING] Task spawn failed (ERR_CODE = %04X)", GetLastError());
      SendStatus(dbgStr);
      --ActiveScanners;
    }

    while (false == taskParams.InitDone)
    {
      Sleep(1);
    }
  }

  sprintf_s(dbgStr, "[INFO] Spawned %d scan jobs", MemoryPages.size());
  SendStatus(dbgStr);

  return true;
}

/**
 * Function: NextScan
 * Notes: See header file
 */
bool MemUtils::NextScan(ScanModifierEnum modifier, 
                           void* value, 
                           size_t valueSize, 
                           fnScanCompleteCallback cb)
{
  char dbgStr[256] = {0};

  if ((0 == MemoryPages.size()) ||
      ((EXACT_VALUE == modifier) && ((nullptr == value) || (0 == valueSize))))
  {
    SendStatus("[ERROR] Scanner was not initialized properly.");
    return false;
  }

  // Check status of all scanning threads
  if (0 != ActiveScanners)
  {
    SendStatus("[WARNING] Scanner is still active, try again later.");
    return false;
  }

  IsNewScan     = false;
  ScanModifier  = modifier;
   
  if (EXACT_VALUE == modifier)
  {
    if (nullptr != ScanValuePtr)
    {
      free(ScanValuePtr);
      ScanValuePtr = nullptr;
    }

    ScanValueSize = valueSize;
    ScanValuePtr  = malloc(ScanValueSize);
    memcpy(ScanValuePtr, value, ScanValueSize);
  }

  ScanCompleteCallback = cb;

  // Spawn tasks  
  TaskParams taskParams;
  taskParams.ThisInstance = this;

  ActiveScanners = MemoryPages.size();
  for (size_t i = 0; i < MemoryPages.size(); ++i)
  {
    taskParams.InitDone = false;
    taskParams.MemPage  = &MemoryPages[i];
    MemoryPages[i].TaskHandle = CreateThread(nullptr,
                                             0, 
                                             reinterpret_cast<LPTHREAD_START_ROUTINE>(ScannerTask), 
                                             &taskParams,
                                             0, 
                                             0);
    if (INVALID_HANDLE_VALUE == MemoryPages[i].TaskHandle)
    {
      --ActiveScanners;
      sprintf_s(dbgStr, "[WARNING] Task spawn failed (ERR_CODE = %04X)", GetLastError());
      SendStatus(dbgStr);
    }

    while (false == taskParams.InitDone)
    {
      Sleep(1);
    }
  }

  sprintf_s(dbgStr, "[INFO] Spawned %d scan jobs", MemoryPages.size());
  SendStatus(dbgStr);

  return true;
}

/**
 * Function: PointerScan
 * Notes: See header file
 */
bool MemUtils::PointerScan(uintptr_t pointerAddress, 
                              size_t maxOffset, 
                              size_t maxLevel, 
                              fnScanCompleteCallback cb)
{
  char dbgStr[256] = {0};

  if (0 == MemoryPages.size())
  {
    SendStatus("[ERROR] Scanner was not initialized properly.");
    return false;
  }

  if (nullptr != ScanValuePtr)
  {
    free(ScanValuePtr);
    ScanValuePtr = nullptr;
  }

  // Check status of all scanning threads
  if (0 != ActiveScanners)
  {
    SendStatus("[WARNING] Scanner is still active, try again later.");
    return false;
  }

  ScanValuePtr  = malloc(sizeof(uintptr_t));
  memcpy(ScanValuePtr, &pointerAddress, sizeof(uintptr_t));
  ScanCompleteCallback = cb;

  // Spawn tasks  
  TaskParams taskParams;
  taskParams.ThisInstance = this;

  ActiveScanners = MemoryPages.size();
  for (size_t i = 0; i < MemoryPages.size(); ++i)
  {
    taskParams.InitDone = false;
    taskParams.MemPage  = &MemoryPages[i];
    MemoryPages[i].TaskHandle = CreateThread(nullptr,
                                             0, 
                                             reinterpret_cast<LPTHREAD_START_ROUTINE>(PointerTask), 
                                             &taskParams, 
                                             0, 
                                             0);
    if (INVALID_HANDLE_VALUE == MemoryPages[i].TaskHandle)
    {
      --ActiveScanners;
      sprintf_s(dbgStr, "[WARNING] Task spawn failed (ERR_CODE = %04X)", GetLastError());
      SendStatus(dbgStr);
    }

    while (false == taskParams.InitDone)
    {
      Sleep(1);
    }
  }

  sprintf_s(dbgStr, "[INFO] Spawned %d pointer scan jobs", MemoryPages.size());
  SendStatus(dbgStr);
    
  return true;
}

/**
 * Function: Read8
 * Notes: See header file
 */
uint8_t MemUtils::Read8(void* handle, uintptr_t address)
{
  uint32_t value = Read32(handle, address);
  return *(uint8_t*)&value;
}

/**
 * Function: Read16
 * Notes: See header file
 */
uint16_t MemUtils::Read16(void* handle, uintptr_t address)
{
  uint32_t value = Read32(handle, address);
  return *(uint16_t*)&value;
}

/**
 * Function: ReadFloat
 * Notes: See header file
 */
float MemUtils::ReadFloat(void* handle, uintptr_t address)
{
  uint32_t value = Read32(handle, address);
  return *(float*)&value;
}

/**
 * Function: Read32
 * Notes: See header file
 */
uint32_t MemUtils::Read32(void* handle, uintptr_t address)
{
  uint32_t value   = 0;
  size_t bytesRead = 0;
  ReadProcessMemory(handle, reinterpret_cast<void*>(address), &value, sizeof(value), &bytesRead);

  return value;
}

/**
 * Function: GetPointerAddress
 * Notes: See header file
 */
uintptr_t MemUtils::GetPointerAddress(void* handle, std::vector<uintptr_t> params)
{
  if (1 >= params.size())
  {
    return 0;
  }

  uintptr_t address = params[0];
  
  size_t bytesRead = 0;
  for (size_t i = 1; i < params.size(); ++i)
  {
    if (FALSE == ReadProcessMemory(handle, 
                                   reinterpret_cast<void*>(address), 
                                   &address, 
                                   sizeof(address), 
                                   &bytesRead))
    {
      break;
    }

    address += params[i];
  }

  return address;
}

/**
 * Function: ReadArray
 * Notes: See header file
 */
std::vector<char> MemUtils::ReadArray(void* handle, uintptr_t address, size_t readSize)
{
  std::vector<char> value(readSize + 1, 0);

  size_t bytesRead = 0;

  if (FALSE == ReadProcessMemory(handle, 
                                 reinterpret_cast<LPCVOID>(address), 
                                 value.data(), 
                                 readSize, 
                                 &bytesRead))
  {
    char dbgStr[256] = { 0 };
    sprintf_s(dbgStr,
              "[WARNING] Failed reading virtual memory @ 0x%llx (ERR_CODE = %04X)",
              address,
              GetLastError());
    SendStatus(dbgStr);
  }

  return value;
}

/**
 * Function: Write
 * Notes: See header file
 */
bool MemUtils::Write(void* handle, uintptr_t address, std::vector<char>& buf)
{
  if (false == Interpreter::DeveloperMode)
  {
    return false;
  }

  bool success = true;
  size_t wSize = 0;
  if (FALSE == WriteProcessMemory(handle, 
                                  reinterpret_cast<LPVOID>(address), 
                                  buf.data(), 
                                  buf.size(),
                                  &wSize))
  {
    success = false;
    char dbgStr[256] = { 0 };
    sprintf_s(dbgStr,
              "[WARNING] Failed writing virtual memory @ 0x%llx (ERR_CODE = %04X)",
              address,
              GetLastError());
    SendStatus(dbgStr);
  }

  return success;
}

/**
 * Function: GetScanResults
 * Notes: See header file
 */
void MemUtils::GetScanResults(std::vector<uintptr_t>& addresses,
                                 std::vector<std::string>& values)
{
  size_t bytesRead;
  char value[256] = {0};
  std::vector<char> buf;
  for (size_t i = 0; i < MemoryPages.size(); ++i)
  {
    for (size_t j = 0; j < MemoryPages[i].ScanResults.size(); ++j)
    {
      uintptr_t address = reinterpret_cast<uintptr_t>(MemoryPages[i].BaseAddress) +
                          MemoryPages[i].ScanResults[j];
      addresses.push_back(address);
      switch (ScanType)
      {
        case SCAN_CHAR:
          buf.resize(1, 0);
          ReadProcessMemory(ActiveProcess,
                            reinterpret_cast<LPCVOID>(address),
                            buf.data(),
                            buf.size(),
                            &bytesRead);
          sprintf_s(value, "%d", *reinterpret_cast<uint8_t*>(buf.data()));
          values.push_back(value);
        break;
        case SCAN_SHORT:
          buf.resize(2, 0);
          ReadProcessMemory(ActiveProcess,
                            reinterpret_cast<LPCVOID>(address),
                            buf.data(),
                            buf.size(),
                            &bytesRead);
          sprintf_s(value, "%d", *reinterpret_cast<uint16_t*>(buf.data()));
          values.push_back(value);
          break;
        case SCAN_INT:
          buf.resize(4, 0);
          ReadProcessMemory(ActiveProcess,
                            reinterpret_cast<LPCVOID>(address),
                            buf.data(),
                            buf.size(),
                            &bytesRead);
          sprintf_s(value, "%d", *reinterpret_cast<uint32_t*>(buf.data()));
          values.push_back(value);
          break;
        case SCAN_FLOAT:
          buf.resize(4, 0);
          ReadProcessMemory(ActiveProcess,
                            reinterpret_cast<LPCVOID>(address),
                            buf.data(),
                            buf.size(),
                            &bytesRead);
          sprintf_s(value, "%f", *reinterpret_cast<float*>(buf.data()));
          values.push_back(value);
          break;
        case SCAN_ARRAY:
          buf.resize(ScanValueSize + 1, 0);
          ReadProcessMemory(ActiveProcess,
                            reinterpret_cast<LPCVOID>(address),
                            buf.data(),
                            ScanValueSize,
                            &bytesRead);
          sprintf_s(value, "%s", buf.data());
          values.push_back(value);
          break;
      }
    }
  }
}

/**
 * Function: GetPointerResults
 * Notes: See header file
 */
void MemUtils::GetPointerResults(std::vector<uintptr_t>& addresses,
                                    std::vector<uintptr_t>& offsets)
{
  addresses.clear();
  offsets.clear();

  for (size_t i = 0; i < MemoryPages.size(); ++i)
  {
    for (size_t j = 0; j < MemoryPages[i].PointerResults.size(); ++j)
    {
      addresses.push_back(MemoryPages[i].PointerResults[j].Address);
      offsets.push_back(MemoryPages[i].PointerResults[j].Offset);
    }
  }
}

/**
 * Function: MemUtils
 * Notes: See header file
 */
MemUtils::MemUtils() :
  ProgressSem(new Win32Sem(1, 1)),
  ActiveProcess(INVALID_HANDLE_VALUE),
  ScanType(SCAN_INVALID),
  ScanValuePtr(nullptr),
  StatusCallback(nullptr),
  ScanCompleteCallback(nullptr),
  ActiveScanners(0)
{
}

/**
 * Function: ~MemUtils
 * Notes: See header file
 */
MemUtils::~MemUtils()
{
  while (true == IsScanInProgress())
  {
    Sleep(100);
  }

  if (nullptr != ScanValuePtr)
  {
    free(ScanValuePtr);
  }
}

/**
 * Function: ScannerTask
 * Notes: See header file
 */
void MemUtils::ScannerTask(TaskParams* params)
{ 
  char dbgStr[256] = { 0 };

  MemUtils* scanner     = reinterpret_cast<MemUtils*>(params->ThisInstance);
  MemPageEntry* memPageRef = reinterpret_cast<MemPageEntry*>(params->MemPage);
  params->InitDone         = true;

  if (nullptr != memPageRef)
  {
    size_t bytesRead = 0;
    std::vector<uint8_t> buf(memPageRef->RegionSize, 0);
    if (FALSE == ReadProcessMemory(scanner->ActiveProcess,
                                    reinterpret_cast<LPCVOID>(memPageRef->BaseAddress),
                                    buf.data(),
                                    buf.size(),
                                    &bytesRead))
    {
      sprintf_s(dbgStr,
                "[WARNING] Failed reading virtual memory @ 0x%llx (ERR_CODE = %04X)",
                memPageRef->BaseAddress,
                GetLastError());
      scanner->SendStatus(dbgStr);
      --scanner->ActiveScanners;
      return;
    }    

    if (true == scanner->IsNewScan)
    {
      memPageRef->ScanResults.clear();

      switch (scanner->ScanType)
      {
        case SCAN_CHAR:
        {
          for (size_t i = 0; i < buf.size(); ++i)
          {
            if ((UNKNOWN_VALUE == scanner->ScanModifier) ||
                (0 == memcmp(scanner->ScanValuePtr, &buf[i], sizeof(uint8_t)))) // Exact Value
            {
              memPageRef->ScanResults.push_back(static_cast<uintptr_t>(i));
            }
          }
        }
        break;
        case SCAN_SHORT:
        {
          for (size_t i = 0; i < buf.size() - sizeof(uint16_t) - 1; ++i)
          {
            if ((UNKNOWN_VALUE == scanner->ScanModifier) ||
                (0 == memcmp(scanner->ScanValuePtr, &buf[i], sizeof(uint16_t)))) // Exact Value
            {
              memPageRef->ScanResults.push_back(static_cast<uintptr_t>(i));
            }
          }
        }
        break;
        case SCAN_INT:
        case SCAN_FLOAT:
        {
          for (size_t i = 0; i < buf.size() - sizeof(uint32_t) - 1; ++i)
          {
            if ((UNKNOWN_VALUE == scanner->ScanModifier) ||
                (0 == memcmp(scanner->ScanValuePtr, &buf[i], sizeof(uint32_t)))) // Exact Value
            {
              memPageRef->ScanResults.push_back(static_cast<uintptr_t>(i));
            }
          }
        }
        break;
        case SCAN_ARRAY:
        {
          for (size_t i = 0; i < buf.size() - scanner->ScanValueSize - 1; ++i)
          {
            if ((UNKNOWN_VALUE == scanner->ScanModifier) ||
                (0 == memcmp(scanner->ScanValuePtr, &buf[i], scanner->ScanValueSize))) // Exact Value
            {
              memPageRef->ScanResults.push_back(static_cast<uintptr_t>(i));
            }
          }
        }
        break;
        default:
          break;
      }
    }    
    else
    {
      for (size_t i = 0; i < memPageRef->ScanResults.size(); ++i)
      {
        switch (scanner->ScanType)
        {
          case SCAN_CHAR:
          {
            if ((UNKNOWN_VALUE == scanner->ScanModifier) ||
                (((UNCHANGED_VALUE == scanner->ScanModifier) ||
                  (EXACT_VALUE == scanner->ScanModifier)) &&
                  (0 == memcmp(scanner->ScanValuePtr, &buf[memPageRef->ScanResults[i]], sizeof(uint8_t)))) ||
                ((CHANGED_VALUE == scanner->ScanModifier) &&
                  (0 != memcmp(scanner->ScanValuePtr, &buf[memPageRef->ScanResults[i]], sizeof(uint8_t)))))
            {
              break;
            }
    
            uint8_t scanValue = *reinterpret_cast<uint8_t*>(scanner->ScanValuePtr);
            uint8_t currValue = *reinterpret_cast<uint8_t*>(&buf[memPageRef->ScanResults[i]]);
            if (((INCREASED_VALUE == scanner->ScanModifier) &&
                 (currValue > scanValue)) ||
                ((DECREASED_VALUE == scanner->ScanModifier) &&
                  (currValue < scanValue)))
            {
              break;
            }
      
            memPageRef->ScanResults.erase(memPageRef->ScanResults.begin() + i);
            --i;
          }
          break;
          case SCAN_SHORT:
          {
            if ((UNKNOWN_VALUE == scanner->ScanModifier) ||
                (((UNCHANGED_VALUE == scanner->ScanModifier) ||
                  (EXACT_VALUE == scanner->ScanModifier)) &&
                  (0 == memcmp(scanner->ScanValuePtr, &buf[memPageRef->ScanResults[i]], sizeof(uint16_t)))) ||
                ((CHANGED_VALUE == scanner->ScanModifier) &&
                  (0 != memcmp(scanner->ScanValuePtr, &buf[memPageRef->ScanResults[i]], sizeof(uint16_t)))))
            {
              break;
            }
    
            uint16_t scanValue = *reinterpret_cast<uint16_t*>(scanner->ScanValuePtr);
            uint16_t currValue = *reinterpret_cast<uint16_t*>(&buf[memPageRef->ScanResults[i]]);
            if (((INCREASED_VALUE == scanner->ScanModifier) &&
                 (currValue > scanValue)) ||
                ((DECREASED_VALUE == scanner->ScanModifier) &&
                  (currValue < scanValue)))
            {
              break;
            }
      
            memPageRef->ScanResults.erase(memPageRef->ScanResults.begin() + i);
            --i;
          }
          break;
          case SCAN_INT:
          {
            if ((UNKNOWN_VALUE == scanner->ScanModifier) ||
                (((UNCHANGED_VALUE == scanner->ScanModifier) ||
                  (EXACT_VALUE == scanner->ScanModifier)) &&
                  (0 == memcmp(scanner->ScanValuePtr, &buf[memPageRef->ScanResults[i]], sizeof(uint32_t)))) ||
                ((CHANGED_VALUE == scanner->ScanModifier) &&
                  (0 != memcmp(scanner->ScanValuePtr, &buf[memPageRef->ScanResults[i]], sizeof(uint32_t)))))
            {
              break;
            }
    
            uint32_t scanValue = *reinterpret_cast<uint32_t*>(scanner->ScanValuePtr);
            uint32_t currValue = *reinterpret_cast<uint32_t*>(&buf[memPageRef->ScanResults[i]]);
            if (((INCREASED_VALUE == scanner->ScanModifier) &&
                 (currValue > scanValue)) ||
                ((DECREASED_VALUE == scanner->ScanModifier) &&
                  (currValue < scanValue)))
            {
              break;
            }
      
            memPageRef->ScanResults.erase(memPageRef->ScanResults.begin() + i);
            --i;
          }
          break;
          case SCAN_FLOAT:
          {
            if ((UNKNOWN_VALUE == scanner->ScanModifier) ||
                (((UNCHANGED_VALUE == scanner->ScanModifier) ||
                  (EXACT_VALUE == scanner->ScanModifier)) &&
                  (0 == memcmp(scanner->ScanValuePtr, &buf[memPageRef->ScanResults[i]], sizeof(uint32_t)))) ||
                ((CHANGED_VALUE == scanner->ScanModifier) &&
                  (0 != memcmp(scanner->ScanValuePtr, &buf[memPageRef->ScanResults[i]], sizeof(uint32_t)))))
            {
              break;
            }
    
            float scanValue = *reinterpret_cast<float*>(scanner->ScanValuePtr);
            float currValue = *reinterpret_cast<float*>(&buf[memPageRef->ScanResults[i]]);
            if (((INCREASED_VALUE == scanner->ScanModifier) &&
                 (currValue > scanValue)) ||
                ((DECREASED_VALUE == scanner->ScanModifier) &&
                 (currValue < scanValue)))
            {
              break;
            }
      
            memPageRef->ScanResults.erase(memPageRef->ScanResults.begin() + i);
            --i;
          }
          break;
          case SCAN_ARRAY:
          {
            if ((UNKNOWN_VALUE == scanner->ScanModifier) ||
                (((UNCHANGED_VALUE == scanner->ScanModifier) || (EXACT_VALUE == scanner->ScanType)) &&
                  (0 == memcmp(scanner->ScanValuePtr, &buf[memPageRef->ScanResults[i]], scanner->ScanValueSize))) ||
                ((CHANGED_VALUE == scanner->ScanModifier) &&
                  (0 != memcmp(scanner->ScanValuePtr, &buf[memPageRef->ScanResults[i]], scanner->ScanValueSize))))
            {
              break;
            }
      
            memPageRef->ScanResults.erase(memPageRef->ScanResults.begin() + i);
            --i;
          }
          break;
          default:
            break;
        }
      }
    }

    // Set completion flag
    scanner->ProgressSem->Take(10);
    --scanner->ActiveScanners;
    scanner->ProgressSem->Give();
  }

  if (nullptr != scanner->ScanCompleteCallback)
  {
    if (0 < scanner->ActiveScanners)
    {
      if (scanner->QuarterProgress == (scanner->MemoryPages.size() - scanner->ActiveScanners))
      {
        scanner->ScanCompleteCallback(1);
      }
      else if (scanner->QuarterProgress * 2 == (scanner->MemoryPages.size() - scanner->ActiveScanners))
      {
        scanner->ScanCompleteCallback(2);
      }
      else if (scanner->QuarterProgress * 3 == (scanner->MemoryPages.size() - scanner->ActiveScanners))
      {
        scanner->ScanCompleteCallback(3);
      }
    }
    else
    {
      scanner->ScanCompleteCallback(4);
      scanner->SendStatus("[INFO] Scan complete");
    }
  }
}

/**
 * Function: PointerTask
 * Notes: See header file
 */
void MemUtils::PointerTask(TaskParams* params)
{
  char dbgStr[256] = { 0 };

  MemUtils* scanner     = reinterpret_cast<MemUtils*>(params->ThisInstance);
  MemPageEntry* memPageRef = reinterpret_cast<MemPageEntry*>(params->MemPage);
  params->InitDone         = true;
  if (nullptr != memPageRef)
  {
    memPageRef->PointerResults.clear();

    size_t bytesRead = 0;
    std::vector<uint8_t> buf(memPageRef->RegionSize, 0);
    if (FALSE == ReadProcessMemory(scanner->ActiveProcess,
                                   reinterpret_cast<LPCVOID>(memPageRef->BaseAddress),
                                   buf.data(),
                                   buf.size(),
                                   &bytesRead))
    {
      sprintf_s(dbgStr,
                "[WARNING] Failed reading virtual memory @ 0x%llx (ERR_CODE = %04X)",
                memPageRef->BaseAddress,
                GetLastError());
      scanner->SendStatus(dbgStr);
      --scanner->ActiveScanners;
      return;
    }    

    size_t maxOffset = 1024;
    for (size_t i = 0; i < buf.size() - sizeof(uint32_t) - 1; ++i)
    {
      uintptr_t address = *reinterpret_cast<uintptr_t*>(scanner->ScanValuePtr);
      uintptr_t offset = address - *reinterpret_cast<uintptr_t*>(&buf[i]);
      if (maxOffset >= offset)
      {
        PointerEntry e;
        e.Address = reinterpret_cast<uintptr_t>(memPageRef->BaseAddress) + i;
        e.Offset  = offset;
        memPageRef->PointerResults.emplace_back(e);
      }
    }

    // Set completion flag
    --scanner->ActiveScanners;
  }

  if (nullptr != scanner->ScanCompleteCallback)
  {
    if (0 < scanner->ActiveScanners)
    {
      if (scanner->QuarterProgress == (scanner->MemoryPages.size() - scanner->ActiveScanners))
      {
        scanner->ScanCompleteCallback(1);
      }
      else if (scanner->QuarterProgress * 2 == (scanner->MemoryPages.size() - scanner->ActiveScanners))
      {
        scanner->ScanCompleteCallback(2);
      }
      else if (scanner->QuarterProgress * 3 == (scanner->MemoryPages.size() - scanner->ActiveScanners))
      {
        scanner->ScanCompleteCallback(3);
      }
    }
    else
    {
      scanner->SendStatus("[INFO] Scan complete");
      scanner->ScanCompleteCallback(4);
    }
  }
}

/**
 * Function: IsScanInProgress
 * Notes: See header file
 */
bool MemUtils::IsScanInProgress()
{
  return 0 != ActiveScanners;
}

/**
 * Function: SendStatus
 * Notes: See header file
 */
void MemUtils::SendStatus(std::string status)
{
  if (nullptr != StatusCallback)
  {
    StatusCallback(SCAN_DEST, status);
  }
}
