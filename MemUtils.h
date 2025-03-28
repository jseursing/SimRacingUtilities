#pragma once
#include "GlobalConst.h"
#include <map>
#include <string>
#include <vector>

class Win32Sem;

/**
  * Class: MemUtils
  * Brief: Utility for probing and manipulating an active process.  
  */
class MemUtils
{
public:
  enum ScanTypeEnum
  {
    SCAN_CHAR,
    SCAN_SHORT,
    SCAN_INT,
    SCAN_FLOAT,
    SCAN_ARRAY,
    SCAN_POINTER,
    SCAN_INVALID
  };

  enum ScanModifierEnum
  {
    EXACT_VALUE,
    CHANGED_VALUE,
    UNCHANGED_VALUE,
    INCREASED_VALUE,
    DECREASED_VALUE,
    UNKNOWN_VALUE
  };

  struct ModuleEntry
  {
    std::string ModuleName;
    uintptr_t   BaseAddress;
    size_t      ModuleSize;
  };

  /**
   * Identify whether or not the executable has elevated privileges.
   */
  static bool IsElevated();

  /**
   * Enables SE_DEBUG_PRIV to support active process probing.
   */
  static bool EnableTokenPrivs();

  /**
   * Enumerate all active processes.
   */
  static void EnumerateProcesses(std::map<uint32_t, std::string>& procMap);

  /**
   * Identifies the module (strValue) address and appends an offset if specified.
   * Used to identify static vs. dynamic addresses. 
   */
  uintptr_t MapAddress(std::string strValue);

  /**
   * Retrieves the module list of the active process.
   */
  void GetModuleList(std::vector<ModuleEntry>& moduleList);

  /**
   * Identify whether or not the executable has elevated privileges.
   */
  void GetModule(uintptr_t address, std::string& module, uintptr_t& moduleAddress);

  /**
   * Opens the executable's process for READ/WRITE.
   */
  static void* GetProcessHandle(std::string executable);

  /**
   * Sets the debug status callback
   */
  void SetStatusCallback(fnStatusCallback cb);

  /**
   * Sets the target process handle.
   */
  bool SetActiveProcess(void* processHandle);

  /**
   * Retrieves the target process handle.
   */
  void* GetActiveProcess() const;

  /**
   * Performs an initial value scan.
   */
  bool NewScan(ScanTypeEnum type, 
               ScanModifierEnum modifier, 
               void* value, 
               size_t valueSize, 
               fnScanCompleteCallback cb);

  /**
   * Filters any previous scan results.
   */
  bool NextScan(ScanModifierEnum modifier, 
                void* value, 
                size_t valueSize, 
                fnScanCompleteCallback cb);

  /**
   * Scan for potential pointers for the specified address.
   */
  bool PointerScan(uintptr_t pointerAddress, 
                   size_t maxOffset, 
                   size_t maxLevel,
                   fnScanCompleteCallback cb);

  /**
   * Read 1-BYTE at the specified address belonging to the specified process.
   */
  uint8_t Read8(void* handle, uintptr_t address);

  /**
   * Read 2-BYTES at the specified address belonging to the specified process.
   */
  uint16_t Read16(void* handle, uintptr_t address);

  /**
   * Read 4-BYTES float at the specified address belonging to the specified process.
   */
  float ReadFloat(void* handle, uintptr_t address);

  /**
   * Read 4-BYTES at the specified address belonging to the specified process.
   */
  uint32_t Read32(void* handle, uintptr_t address);

  /**
   * Repeatedly read and apply the specified offsets to retrieve the final address. 
   */
  uintptr_t GetPointerAddress(void* handle, std::vector<uintptr_t> params);

  /**
   * Read variable length data at the specified address belonging to the specified process.
   */
  std::vector<char> ReadArray(void* handle, uintptr_t address, size_t readSize);

  /**
   * Write variable length data at the specified address belonging to the specified process.
   */
  bool Write(void* handle, uintptr_t address, std::vector<char>& buf);

  /**
   * Retrieve scan results.
   */
  void GetScanResults(std::vector<uintptr_t>& addresses,
                      std::vector<std::string>& values);

  /**
   * Retrieve pointer scan results.
   */
  void GetPointerResults(std::vector<uintptr_t>& addresses,
                         std::vector<uintptr_t>& offsets);

  /**
   * CTOR/DTOR
   */
  MemUtils();
  ~MemUtils();

private:

  struct TaskParams
  {
    void* ThisInstance;
    void* MemPage;
    bool  InitDone;
  };

  /**
   * Task functions used for scans.
   */
  static void ScannerTask(TaskParams* params);
  static void PointerTask(TaskParams* params);

  bool IsScanInProgress();
  void SendStatus(std::string status);

  struct PointerEntry
  {
    uintptr_t Address;
    uintptr_t Offset;
  };

  struct MemPageEntry
  {
    void*                     BaseAddress;
    size_t                    RegionSize;
    unsigned long             PageProtect;
    std::vector<uintptr_t>    ScanResults; 
    std::vector<PointerEntry> PointerResults;
    void*                     TaskHandle;
  };
  
  void*                     ActiveProcess;
  Win32Sem*                 ProgressSem;
  std::vector<MemPageEntry> MemoryPages; 
  std::vector<ModuleEntry>  ModuleList; 
  bool                      IsNewScan;
  int32_t                   ActiveScanners;
  int32_t                   QuarterProgress;
  ScanTypeEnum              ScanType;
  ScanModifierEnum          ScanModifier;
  void*                     ScanValuePtr;
  size_t                    ScanValueSize;
  fnStatusCallback          StatusCallback;
  fnScanCompleteCallback    ScanCompleteCallback;
};

