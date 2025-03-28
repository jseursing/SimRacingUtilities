#include "MemUtils.h"
#include "TelemetryManager.h"
#include "WindowManager.h"
#include <thread>
#include <TlHelp32.h>
#include <vector>
#include <winsock.h>
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")

/**
 * Function: Instance
 * Notes: See header file
 */
TelemetryManager* TelemetryManager::Instance()
{
  static TelemetryManager instance;
  return &instance;
}

/**
 * Function: GetStatus
 * Notes: See header file
 */
TelemetryManager::StatusEnum TelemetryManager::GetStatus() const
{
  return Status;
}

/**
 * Function: GetSubscriberCount
 * Notes: See header file
 */
size_t TelemetryManager::GetSubscriberCount() const
{
  return Subscribers.size();
}

/**
 * Function: SetStatusCallback
 * Notes: See header file
 */
void TelemetryManager::SetStatusCallback(fnStatusCallback cb)
{
  StatusCallback = cb;
  MemScanner->SetStatusCallback(cb);
}

/**
 * Function: GetProfiles
 * Notes: See header file
 */
std::vector<std::string> TelemetryManager::GetProfiles() const
{
  std::vector<std::string> profiles;

  std::map<size_t, ACTelemetry>::const_iterator itr = Telemetry.begin();
  for (; itr != Telemetry.end(); ++itr)
  {
    profiles.push_back(itr->second.GetProfileName());
  }
  
  return profiles;
}

/**
 * Function: SetActiveTelemetry
 * Notes: See header file
 */
void TelemetryManager::SetActiveTelemetry(std::string profile)
{
  std::map<size_t, ACTelemetry>::iterator itr = Telemetry.find(ProfileHasher(profile.c_str()));
  if (Telemetry.end() != itr)
  {
    ActiveTlm = &(itr->second);
    SendStatus("Active profile set: " + ActiveTlm->GetProfileName());
  }
  else
  {
    Telemetry[ProfileHasher(profile.c_str())] = ACTelemetry();
    Telemetry[ProfileHasher(profile.c_str())].SetProfileName(profile);
    ActiveTlm = &(Telemetry[ProfileHasher(profile.c_str())]);
    SendStatus("New Active profile set: " + ActiveTlm->GetProfileName());
  }
}

/** 
 * Function: DeleteProfile
 * Notes: See header file
 */
bool TelemetryManager::DeleteProfile(std::string profile)
{
  std::map<size_t, ACTelemetry>::iterator itr = Telemetry.find(ProfileHasher(profile.c_str()));
  if (Telemetry.end() != itr)
  {
    SendStatus("Delete profile: " + itr->second.GetProfileName());
    Telemetry.erase(itr->first);
    return true;
  }

  return false;
}

/**
 * Function: GetTargetExecutable
 * Notes: See header file
 */
std::string TelemetryManager::GetTargetExecutable() const
{
  return ActiveTlm->GetExecutableName();
}

/**
 * Function: SetTargetExecutable
 * Notes: See header file
 */
void TelemetryManager::SetTargetExecutable(std::string executable)
{
  ActiveTlm->SetExecutableName(executable);
  SendStatus(ActiveTlm->GetProfileName() + ": Target executable = " + executable);
}

/**
 * Function: SetTelemetry
 * Notes: See header file
 */
void TelemetryManager::SetTelemetry(std::string profile, ACTlmEnum::TlmTypeEnum type, std::string value)
{
  std::map<size_t, ACTelemetry>::iterator itr = Telemetry.find(ProfileHasher(profile.c_str()));
  if (Telemetry.end() != itr)
  {
    itr->second.SetTelemetry(type, value);
  }
  else
  {
    Telemetry[ProfileHasher(profile.c_str())] = ACTelemetry();
    SetTelemetry(profile.c_str(), type, value);
  }
}

/**
 * Function: GetTelemetry
 * Notes: See header file
 */
std::string TelemetryManager::GetTelemetry(std::string profile, ACTlmEnum::TlmTypeEnum type)
{
  std::string tlmValue = "";
  size_t test = ProfileHasher(profile);
  std::map<size_t, ACTelemetry>::iterator itr = Telemetry.find(test);
  if (Telemetry.end() != itr)
  {
    tlmValue = itr->second.GetTelemetry(type);
  }

  return tlmValue;
}

/**
 * Function: SetTelemetryFunction
 * Notes: See header file
 */
void TelemetryManager::SetTelemetryFunction(std::string profile, ACTlmEnum::TlmTypeEnum type, std::string value)
{
  std::map<size_t, ACTelemetry>::iterator itr = Telemetry.find(ProfileHasher(profile.c_str()));
  if (Telemetry.end() != itr)
  {
    itr->second.SetTelemetryFunction(type, value);
  }
  else
  {
    Telemetry[ProfileHasher(profile.c_str())] = ACTelemetry();
    SetTelemetryFunction(profile.c_str(), type, value);
  }
}

/**
 * Function: GetTelemetryFunction
 * Notes: See header file
 */
std::string TelemetryManager::GetTelemetryFunction(std::string profile, ACTlmEnum::TlmTypeEnum type)
{
  std::string function = "";
  size_t test = ProfileHasher(profile);
  std::map<size_t, ACTelemetry>::iterator itr = Telemetry.find(test);
  if (Telemetry.end() != itr)
  {
    function = itr->second.GetTelemetryFunctionString(type);
  }

  return function;
}

/**
 * Function: SetUpdateFreqMs
 * Notes: See header file
 */
void TelemetryManager::SetUpdateFreqMs(size_t ms)
{
  UpdateFreqMs = (50 > ms ? 50 : ms);
}

/**
 * Function: SetSpotFreq
 * Notes: See header file
 */
void TelemetryManager::SetSpotFreq(size_t mod)
{
  SpotFreq = mod;
}

/**
 * Function: IsTelemetryServerRunning
 * Notes: See header file
 */
bool TelemetryManager::IsTelemetryServerRunning() const
{
  return KeepAlive;
}

/**
 * Function: StartTelemetry
 * Notes: See header file
 */
bool TelemetryManager::StartTelemetry()
{
  KeepAlive = true;
  HANDLE hThread = CreateThread(nullptr, 
                                0, 
                                reinterpret_cast<LPTHREAD_START_ROUTINE>(TelemetrySubscriptionTask), 
                                nullptr, 
                                0, 
                                0);
  if (INVALID_HANDLE_VALUE == hThread)
  {
    SendStatus("[CRITICAL] Telemetry subscription service launch failed.");
    Status = TASK_SPAWN_ERR;
    return false;
  }

  hThread = CreateThread(nullptr, 
                         0, 
                         reinterpret_cast<LPTHREAD_START_ROUTINE>(TelemetryUpdateTask), 
                         nullptr, 
                         0, 
                         0);
  if (INVALID_HANDLE_VALUE == hThread)
  {
    SendStatus("[CRITICAL] Telemetry update service launch failed.");
    Status = TASK_SPAWN_ERR;
    return false;
  }

  return true;
}

/**
 * Function: StopTelemetry
 * Notes: See header file
 */
void TelemetryManager::StopTelemetry()
{
  KeepAlive = false;
}

/**
 * Function: Export
 * Notes: See header file
 */
void TelemetryManager::Export(std::vector<char>& data)
{
  size_t dataSize = 1; // Entry Count
  size_t dataLenPos = data.size() + 1;

  data.push_back(GlobalConst::TELEMETRY_MGR); // MGR ID
  data.push_back(0); // two reserve bytes for total size
  data.push_back(0);
  data.push_back(Telemetry.size());       // ENTRY Count
  for (std::map<size_t, ACTelemetry>::iterator itr = Telemetry.begin();
       itr != Telemetry.end();
       ++itr)
  {
    // Write length of profile name, profile name
    std::string profile = itr->second.GetProfileName();
    dataSize += 1 + profile.length();
    data.push_back(static_cast<char>(profile.length()));
    for (size_t i = 0; i < profile.length(); ++i)
    {
      data.push_back(profile[i]);
    }

    // Write length of executable, executable
    std::string exeName = itr->second.GetExecutableName();
    dataSize += 1 + exeName.length();
    data.push_back(static_cast<char>(exeName.length()));
    for (size_t i = 0; i < exeName.length(); ++i)
    {
      data.push_back(exeName[i]);
    }

    // Write length and tlm value
    for (size_t tlm = ACTlmEnum::HS_CAR_NAME; tlm < ACTlmEnum::MAX_TLM_TYPE; ++tlm)
    {
      std::string tlmValue = itr->second.GetTelemetryFunctionString(static_cast<ACTlmEnum::TlmTypeEnum>(tlm));
      dataSize += 1 + tlmValue.length();
      data.push_back(static_cast<char>(tlmValue.length()));
      for (size_t i = 0; i < tlmValue.length(); ++i)
      {
        data.push_back(tlmValue[i]);
      }
    }

    SendStatus(profile + ": successfully exported");
  }

  // Update total length
  *reinterpret_cast<uint16_t*>(&data[dataLenPos]) = dataSize;
}

/**
 * Function: Import
 * Notes: See header file
 */
void TelemetryManager::Import(std::vector<char> data)
{
  size_t pos = 0;

  size_t numEntries = data[pos++];
  if (0 != numEntries)
  {
    Telemetry.clear();
  }

  for (size_t i = 0; i < numEntries; ++i)
  {
    std::string profile    = "";
    std::string executable = "";

    if (pos > data.size()) return;
    size_t nameLen = data[pos++];

    if ((pos + nameLen) >= data.size()) return;
    profile.resize(nameLen);
    memcpy(profile.data(), &data[pos], nameLen);
    pos += nameLen;

    std::map<size_t, ACTelemetry>::iterator itr = Telemetry.find(ProfileHasher(profile.c_str()));
    if (Telemetry.end() == itr)
    {
      Telemetry[ProfileHasher(profile)] = ACTelemetry();
      itr = Telemetry.find(ProfileHasher(profile.c_str()));
      itr->second.SetProfileName(profile);
    }

    if (pos > data.size()) return;
    nameLen = data[pos++];

    if ((pos + nameLen) >= data.size()) return;
    executable.resize(nameLen);
    memcpy(executable.data(), &data[pos], nameLen);
    itr->second.SetExecutableName(executable);
    pos += nameLen;

    std::string tlmValue = "";
    for (size_t tlm = ACTlmEnum::HS_CAR_NAME; tlm < ACTlmEnum::MAX_TLM_TYPE; ++tlm)
    {
      nameLen = data[pos++];

      if ((pos + nameLen) >= data.size()) return;
      tlmValue.resize(nameLen);
      memcpy(tlmValue.data(), &data[pos], nameLen);
      pos += nameLen;
      itr->second.SetTelemetryFunction(static_cast<ACTlmEnum::TlmTypeEnum>(tlm), tlmValue);
    }

    SendStatus(profile + ": successfully imported");
  }

  ActiveTlm = &(Telemetry.begin()->second);
}

/**
 * Function: SendStatus
 * Notes: See header file
 */
void TelemetryManager::SendStatus(std::string status)
{
  if (nullptr != StatusCallback)
  {
    StatusCallback(TLM_DEST, status);
  }
}

/**
 * Function: SendTelemetryUpdates
 * Notes: See header file
 */
void TelemetryManager::SendTelemetryUpdates()
{
  if (nullptr == ActiveTlm)
  {
    return;
  }

  bool sendSpot = (0 == Frame % SpotFreq);

  std::vector<char> update_buf(ACTelemetry::GetUpdateResponseSize(), 0);
  ActiveTlm->GetUpdateResponse(update_buf.data(), update_buf.size());

  std::vector<char> spot_buf(ACTelemetry::GetSpotResponseSize(), 0);
  ActiveTlm->GetSpotResponse(spot_buf.data(), spot_buf.size());

  for (std::map<uint16_t, SubscriptionEntry*>::const_iterator itr = Subscribers.begin();
       itr != Subscribers.end();
       ++itr)
  {
    if (true == itr->second->Update)
    {
      Server->Send(update_buf.data(), update_buf.size(), itr->second->Address, 0);
    }
  
    if (false == sendSpot)
    {
      continue;
    }

    if (true == itr->second->Spot)
    {
      Server->Send(spot_buf.data(), spot_buf.size(), itr->second->Address, 0);
    }
  }
}

/**
 * Function: ProcessQuery
 * Notes: See header file
 */
void TelemetryManager::ProcessQuery(char* buf, size_t bufSize, sockaddr_in* sender)
{
  if (nullptr == ActiveTlm)
  {
    return;
  }

  // Query should accommodate ACSetupStruct
  if (bufSize < ACTelemetry::GetHandshakeQuerySize())
  {
    SendStatus("[INFO] Handshake request has invalid length.");
    return;
  }

  int32_t id = 0;
  int32_t version = 0;
  ACTlmEnum::OperationEnum operation = ACTlmEnum::INVALID_OP;
  if (false == ActiveTlm->ParseHandshakeQuery(buf, bufSize, id, version, operation))
  {
    SendStatus("[INFO] Received invalid handshake request.");
    return;
  }

  std::vector<char> response;
  switch (operation)
  {
    case ACTlmEnum::HANDSHAKE:
    {
      char dbgStr[512] = { 0 };
      sprintf_s(dbgStr, "[INFO] Handshake request (id=%d, ver=%d)", id, version);
      SendStatus(dbgStr);

      response.resize(ACTelemetry::GetHandshakeResponseSize(), 0);
      ActiveTlm->GetHandshakeResponse(response.data(), response.size(), id, version);
      if (false == Server->Send(response.data(), response.size(), sender, 10))
      {
        sprintf_s(dbgStr, "[WARNING] UDP Send failure (ERR_CODE %04X)", GetLastError());
        SendStatus(dbgStr);
      }
    }
    break;
    case ACTlmEnum::SUB_UPDATE:
    case ACTlmEnum::SUB_SPOT:
    {
      AddSubscriber(sender, operation);
    }
    break;
    case ACTlmEnum::DISMISS:
    {
      RemoveSubscriber(sender);
    }
    break;
    default: break;
  }
}

/**
 * Function: AddSubscriber
 * Notes: See header file
 */
void TelemetryManager::AddSubscriber(sockaddr_in* addr, ACTlmEnum::OperationEnum operation)
{
  std::map<uint16_t, SubscriptionEntry*>::iterator itr = Subscribers.find(addr->sin_port);
  if (Subscribers.end() == itr)
  {
    SubscriptionEntry* entry = new SubscriptionEntry;
    entry->Address = new sockaddr_in;
    memcpy(entry->Address, addr, sizeof(sockaddr_in));
    switch (operation)
    {
      case ACTlmEnum::SUB_UPDATE:
        entry->Update = true;
        break;
      case ACTlmEnum::SUB_SPOT:
        entry->Spot = true;
        break;
      default: break;
    }

    Subscribers[addr->sin_port] = entry;

    char dbgStr[512] = { 0 };
    sprintf_s(dbgStr, "[INFO] Registering Subscriber (id=%d, operation=%d)", addr->sin_port, operation);
    SendStatus(dbgStr);
  }
  else
  {
    memcpy(itr->second->Address, addr, sizeof(sockaddr_in));

    switch (operation)
    {
    case ACTlmEnum::SUB_UPDATE:
      itr->second->Update = true;
      break;
    case ACTlmEnum::SUB_SPOT:
      itr->second->Spot = true;
      break;
    default: break;
    }

    char dbgStr[512] = { 0 };
    sprintf_s(dbgStr, "[INFO] Updating Subscriber (%d, %d)", addr->sin_port, operation);
    SendStatus(dbgStr);
  }
}

/**
 * Function: RemoveSubscriber
 * Notes: See header file
 */
void TelemetryManager::RemoveSubscriber(sockaddr_in* addr)
{
  std::map<uint16_t, SubscriptionEntry*>::iterator itr = Subscribers.find(addr->sin_port);
  if (Subscribers.end() != itr)
  {
    SubscriptionEntry* entry = itr->second;

    char dbgStr[512] = { 0 };
    sprintf_s(dbgStr, "[INFO] Removing Subscriber (%d)", addr->sin_port);
    SendStatus(dbgStr);

    Subscribers.erase(addr->sin_port);
    delete entry->Address;
    delete entry;
  }
}

/**
 * Function: TelemetrySubscriptionTask
 * Notes: See header file
 */
void TelemetryManager::TelemetrySubscriptionTask()
{
  TelemetryManager* instance = Instance();
  if (SERVER_INIT != instance->Status)
  {
    instance->SendStatus("[CRITICAL] Telemetry service initialization failed");
    return;
  }

  std::vector<char> buf(ACTelemetry::GetHandshakeQuerySize(), 0);
  size_t bufSize  = 0;
  size_t readSize = 0;
  sockaddr_in client;

  // Launch the fake Assetto Corsa executable to trigger handshake requests
  STARTUPINFOA si;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(STARTUPINFOA);

  PROCESS_INFORMATION pi;
  memset(&pi, 0, sizeof(pi));

  char current_path[MAX_PATH] = {0};
  GetCurrentDirectoryA(MAX_PATH, current_path);
  strcat_s(current_path, "\\AssettoCorsa.exe");
  if (FALSE == CreateProcessA(nullptr, current_path, 0, 0, FALSE, CREATE_NEW_CONSOLE, 0, 0,  &si, &pi))
  {
    char dbgStr[512] = { 0 };
    sprintf_s(dbgStr, "[WARNING] Dummy spawn failed (ERR_CODE = %04X), continuing...", GetLastError());
    instance->SendStatus(dbgStr);

    instance->Status = PROC_SPAWN_ERR;
  }
  else
  {
    char dbgStr[512] = { 0 };
    sprintf_s(dbgStr, "[WARNING] Dummy process launched, PID = %08X", pi.dwProcessId);
    instance->SendStatus(dbgStr);
  }

  while (true == instance->KeepAlive)
  {
    static float ctr = 1;
    instance->Status = SERVER_ACTIVE;

    if (nullptr == instance->Server)
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(instance->UpdateFreqMs - 10));

    instance->Status = SERVER_ACTIVE;
    if (true == instance->Server->Receive(buf.data(), buf.size(), readSize, 10, &client))
    {
      instance->ProcessQuery(buf.data(), readSize, &client);
    }

    instance->SendTelemetryUpdates();
    ++instance->Frame;
  }

  instance->SendStatus("[INFO] Terminating telemetry subscription service.");

  TerminateProcess(pi.hProcess, 0);

  instance->Status = SERVER_INIT;
}

/**
 * Function: TelemetryUpdateTask
 * Notes: See header file
 */
void TelemetryManager::TelemetryUpdateTask()
{
  TelemetryManager* instance = Instance();

  if (SERVER_INIT != instance->Status)
  {
    instance->SendStatus("[CRITICAL] Telemetry service initialization failed");
    return;
  }

  char dbgStr[512] = { 0 };
  sprintf_s(dbgStr, 
            "[INFO] Waiting for target executable %s", 
            instance->ActiveTlm->GetExecutableName().c_str());
  instance->SendStatus(dbgStr);
  instance->ActiveTlm->SetProcessHandle(INVALID_HANDLE_VALUE);

  while (true == instance->KeepAlive)
  {
    static float ctr = 1;
    if (INVALID_HANDLE_VALUE == instance->ActiveTlm->GetProcessHandle())
    {
      HANDLE handle = MemUtils::GetProcessHandle(instance->ActiveTlm->GetExecutableName());
      if (INVALID_HANDLE_VALUE != handle)
      {
        instance->MemScanner->SetActiveProcess(handle);
        instance->ActiveTlm->SetProcessHandle(handle);
        sprintf_s(dbgStr, 
                  "[INFO] Attached to executable %s", 
                  instance->ActiveTlm->GetExecutableName().c_str());
        instance->SendStatus(dbgStr);
      }
      else
      {
        continue;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(instance->UpdateFreqMs));
    for (size_t tlm = ACTlmEnum::HS_CAR_NAME; tlm < ACTlmEnum::MAX_TLM_TYPE; ++tlm)
    {
      Interpreter::Function function;
      instance->ActiveTlm->GetTelemetryFunction(static_cast<ACTlmEnum::TlmTypeEnum>(tlm), function);

      if (Interpreter::VALUE != function.Type)
      {
        if (0 == function.Parameters.size()) continue;
        switch (function.Type)
        {
          case Interpreter::MAP_TO_INDEX:
          {
            size_t tlmIndex = Interpreter::InvokeFunction(function, nullptr).u32Ret;
            if (Interpreter::MAX_MODIFIER_TYPE == function.Modifier)
            {
              instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm), 
                                                instance->ActiveTlm->GetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlmIndex)));
            }
            else
            {
              std::string getVal = instance->ActiveTlm->GetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlmIndex));
              float baseVal = strtof(getVal.c_str(), nullptr);
              float modVal  = strtof(function.ModifierValue.c_str(), nullptr);
              switch (function.Modifier)
              {
              case Interpreter::FMULT:
                instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm),
                                                  std::to_string(baseVal * modVal));
                break;
              case Interpreter::FDIV:
                instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm),
                                                  std::to_string(baseVal / modVal));
                break;
              case Interpreter::FADD:
                instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm),
                                                  std::to_string(baseVal + modVal));
                break;
              case Interpreter::FSUB:
                instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm),
                                                  std::to_string(baseVal - modVal));
                break;
              case Interpreter::CMP_EQ:
                instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm),
                                                  baseVal == modVal ? "1" : "0");
                break;
              case Interpreter::CMP_NE:
                instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm),
                                                  baseVal != modVal ? "1" : "0");
                break;
              case Interpreter::CMP_G:
                instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm),
                                                  baseVal > modVal ? "1" : "0");
                break;
              case Interpreter::CMP_GE:
                instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm),
                                                  baseVal >= modVal ? "1" : "0");
                break;
              case Interpreter::CMP_L:
                instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm),
                                                  baseVal < modVal ? "1" : "0");
                break;
              case Interpreter::CMP_LE:
                instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm),
                                                  baseVal <= modVal ? "1" : "0");
                break;
              }
            }
          }
          break;
          case Interpreter::READ_8:
          {
            uint8_t value = Interpreter::InvokeFunction(function, instance->MemScanner).u8Ret;
            instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm), std::to_string(value));
          }
          break;
          case Interpreter::READ_8I:
          {
            int8_t value = Interpreter::InvokeFunction(function, instance->MemScanner).i8Ret;
            instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm), std::to_string(value));
          }
          break;
          case Interpreter::READ_16:
          {
            uint16_t value = Interpreter::InvokeFunction(function, instance->MemScanner).u16Ret;
            instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm), std::to_string(value));
          }
          break;
          case Interpreter::READ_16I:
          {
            int16_t value = Interpreter::InvokeFunction(function, instance->MemScanner).i16Ret;
            instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm), std::to_string(value));
          }
          break;
          case Interpreter::READ_FLOAT:
          {
            float value = Interpreter::InvokeFunction(function, instance->MemScanner).fRet;
            instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm), std::to_string(value));
          }
          break;
          case Interpreter::READ_32:
          {
            uint32_t value = Interpreter::InvokeFunction(function, instance->MemScanner).u32Ret;
            instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm), std::to_string(value));
          }
          break;
          case Interpreter::READ_32I:
          {
            int32_t value = Interpreter::InvokeFunction(function, instance->MemScanner).i32Ret;
            instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm), std::to_string(value));
          }
          break;
          case Interpreter::READ_ARRAY:
          {
            std::vector<char> value = Interpreter::InvokeFunction(function, instance->MemScanner).arrRet;
            std::string strValue = std::string(value.data());
            instance->ActiveTlm->SetTelemetry(static_cast<ACTlmEnum::TlmTypeEnum>(tlm), strValue);
           }
           break;
        }
      }
    }
  }

  instance->SendStatus("[INFO] Terminating telemetry update service.");

  instance->Status = SERVER_INIT;
}

/**
 * Function: TelemetryManager
 * Notes: See header file
 */
TelemetryManager::TelemetryManager() :
  Status(SERVER_INIT),
  StatusCallback(nullptr),
  UpdateFreqMs(50),
  SpotFreq(1200),
  Frame(1)
{
  InitializeWinsock();
  MemScanner = new MemUtils();
  Telemetry[ProfileHasher("Default Profile\0")] = ACTelemetry();
  ActiveTlm = &(Telemetry.begin()->second);

  Server = IpcUdpSocket::Create("127.0.0.1", ACTelemetry::DEFAULT_PORT, true);
  if (nullptr == Server)
  {
    char dbgStr[512] = { 0 };
    sprintf_s(dbgStr, "[INFO] UDP Socket error (ERR_CODE = %04x)", GetLastError());
    SendStatus(dbgStr);

    Status = SERVER_SOCK_ERR;
  }
}

/**
 * Function: InitializeWinsock
 * Notes: See header file
 */
bool TelemetryManager::InitializeWinsock()
{
  static bool initialized = false;
  if (false == initialized)
  {
    WSADATA wsaData;
    initialized = (0 == WSAStartup(MAKEWORD(2, 2), &wsaData));
  }

  return initialized;
}