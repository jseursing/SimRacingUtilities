#pragma once
#include "ACTelemetry.h"
#include "GlobalConst.h"
#include "IpcUdpSocket.h"
#include <map>
#include <vector>

class IpcUdpSocket;
class MemUtils;

/**
 * Class: TelemetryManager
 * Brief: Telemetry interface used to manage telemetry subscriptions and updates. 
 */
class TelemetryManager
{
public:

  enum StatusEnum
  {
    SERVER_INIT,
    SERVER_SOCK_ERR,
    TASK_SPAWN_ERR,
    PROC_SPAWN_ERR,
    SERVER_ACTIVE
  };

  /**
   * Singleton construction and access.
   */
  static TelemetryManager* Instance();

  /**
   * Retrieve current status.
   */
  StatusEnum GetStatus() const;

  /**
   * Retrieve subscriber count.
   */
  size_t GetSubscriberCount() const;

  /**
   * Sets the status callback for debug outputs.
   */
  void SetStatusCallback(fnStatusCallback cb);

  /**
   * Retrieve all telemetry profiles.
   */
  std::vector<std::string> GetProfiles() const;

  /**
   * Sets the specified profile as the active telemetry profile.
   */
  void SetActiveTelemetry(std::string profile);

  /**
   * Delete the specified telemetry profile.
   */
  bool DeleteProfile(std::string profile);

  /**
   * Retrieve the active telemetry's target executable.
   */
  std::string GetTargetExecutable() const;

  /**
   * Sets the active telemetry's target executable.
   */
  void SetTargetExecutable(std::string executable);

  /**
   * Sets the active telemetry's specified telemetry value.
   */
  void SetTelemetry(std::string profile, ACTlmEnum::TlmTypeEnum type, std::string value);

  /**
   * Retrieves the active telemetry's specified telemetry value.
   */
  std::string GetTelemetry(std::string profile, ACTlmEnum::TlmTypeEnum type);

  /**
   * Sets the active telemetry's specified telemetry function.
   */
  void SetTelemetryFunction(std::string profile, ACTlmEnum::TlmTypeEnum type, std::string value);

  /**
   * Retrieves the active telemetry's specified telemetry function.
   */
  std::string GetTelemetryFunction(std::string profile, ACTlmEnum::TlmTypeEnum type);

  /**
   * Sets the update frequency in milliseconds for sending subscription updates.
   */
  void SetUpdateFreqMs(size_t ms);

  /**
   * Sets the update frequency in milliseconds for sending spot updates.
   */
  void SetSpotFreq(size_t mod);

  /**
   * Retrieves the telemetry server state.
   */
  bool IsTelemetryServerRunning() const;

  /**
   * Launch the telemetry service.
   */
  bool StartTelemetry();

  /**
   * Stop the telemetry service.
   */
  void StopTelemetry();

  /**
   * Export all telemetry manager configurations
   */
  void Export(std::vector<char>& data);

  /**
   * Import telemetry manager configurations.
   */
  void Import(std::vector<char> data);

private:

  /**
   * Send debug output.
   */
  void SendStatus(std::string status);

  /**
   * Send telemetry updates to subscribers.
   */
  void SendTelemetryUpdates();

  /**
   * Process inbound handshake queries.
   */
  void ProcessQuery(char* buf, size_t bufSize, sockaddr_in* sender);

  /**
   * Add a new subscriber.
   */
  void AddSubscriber(sockaddr_in* addr, ACTlmEnum::OperationEnum opType);

  /**
   * Remove subscriber.
   */
  void RemoveSubscriber(sockaddr_in* addr);

  /**
   * UDP Server task responsible for processing handshake queries and managing
   * subscriptions.
   */
  static void TelemetrySubscriptionTask();

  /**
   * Task responsible for executing telemetry functions and sending telemetry
   * updates to subscribers. 
   */
  static void TelemetryUpdateTask();

  /**
   * Initialize winsock library because windows requires this...
   */
  bool InitializeWinsock();

  /**
   * CTOR
   */
  TelemetryManager();

  struct SubscriptionEntry
  {
    sockaddr_in*             Address;
    bool                     Update;
    bool                     Spot;
  };

  std::hash<std::string>                 ProfileHasher;
  MemUtils*                              MemScanner;
  StatusEnum                             Status;
  fnStatusCallback                       StatusCallback;
  bool                                   KeepAlive;
  IpcUdpSocket*                          Server;
  ACTelemetry*                           ActiveTlm;
  std::map<size_t, ACTelemetry>          Telemetry;
  size_t                                 UpdateFreqMs;
  size_t                                 SpotFreq;
  size_t                                 Frame;
  std::map<uint16_t, SubscriptionEntry*> Subscribers;
};

