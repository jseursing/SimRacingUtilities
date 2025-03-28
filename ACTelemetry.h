#pragma once
#include "ACTlmEnum.h"
#include "Interpreter.h"
#include <stdint.h>
#include <string>
#include <vector>

/** 
 * Class: ACTelemetry
 * Brief: Class used to manage the data structure used for simulating Assetto Corsa 
 *        telemetry for subscribers. 
 */
class ACTelemetry
{
public:

  /**
   * Retrieve the owner's profile name.
   */
  std::string GetProfileName() const;

  /**
   * Sets the owner's profile name.
   */
  void SetProfileName(std::string profile);

  /**
   * Retrieve the target executable name used for populating telemetry.
   */
  std::string GetExecutableName() const;

  /**
   * Sets the target executable name used for populating telemetry.
   */
  void SetExecutableName(std::string exe);

  /**
   * Retrieves the target executables process handle.
   */
  void* GetProcessHandle() const;

  /**
   * Sets the target executables process handle.
   */
  void SetProcessHandle(void* handle);

  /**
   * Parses the handshake request. 
   * The format is expected to follow Assetto Corsa remote telemetry specifications.
   */
  bool ParseHandshakeQuery(char* query, 
                           size_t bufSize, 
                           int32_t& id, 
                           int32_t& version, 
                           ACTlmEnum::OperationEnum& operation) const;

  /**
   * Generates a handshake response.
   * Format follows Assetto Corsa remote telemetry specifications.
   */
  bool GetHandshakeResponse(char* destBuf, size_t bufSize, int32_t id, int32_t version);

  /**
   * Generates a update subscription response.
   * Format follows Assetto Corsa remote telemetry specifications.
   */
  bool GetUpdateResponse(char* destBuf, size_t bufSize);

  /**
   * Generates a spot subscription response.
   * Format follows Assetto Corsa remote telemetry specifications.
   */
  bool GetSpotResponse(char* destBuf, size_t bufSize);

  /**
   * Sets the specified telemetry's value.
   */
  void SetTelemetry(ACTlmEnum::TlmTypeEnum type, std::string value);

  /**
   * Retrieves the specified telemetry's value.
   */
  std::string GetTelemetry(ACTlmEnum::TlmTypeEnum type); 

  /**
   * Sets the specified telemetry's function.
   * If a unknown function is detected, the value is treated as an exact value and set immediately.
   */
  void SetTelemetryFunction(ACTlmEnum::TlmTypeEnum type, std::string value);

  /**
    * Retrieves the specified telemetry's function.
    */
  void GetTelemetryFunction(ACTlmEnum::TlmTypeEnum type, Interpreter::Function& function);

  /**
    * Retrieves the specified telemetry's function in human readable format.
    */
  std::string GetTelemetryFunctionString(ACTlmEnum::TlmTypeEnum type);

  /**
  * CTOR/DTOR
  */
  ACTelemetry();
  ~ACTelemetry();

  /**
    * Retrieves the telemetry's mnemonic
    */
  static std::string GetMnemonic(ACTlmEnum::TlmTypeEnum type);

  /**
    * Retrieves the specified data structure size as defined by Assetto Corsa
    * remote telemetry documentation.
    */
  static size_t GetHandshakeQuerySize();
  static size_t GetHandshakeResponseSize();
  static size_t GetUpdateResponseSize();
  static size_t GetSpotResponseSize();

  /**
    * Assetto Corsa default UDP port
    */
  static const uint16_t DEFAULT_PORT = 9996; // Assetto Corsa default port

  /**
    * MAX character length for any string-based telemetry.
    */
  static const size_t   MAX_CHAR_LEN = 50;

private:

  struct HandShakeQuery
  {
    int32_t id;
    int32_t version;
    int32_t operation;
  };

  struct HandshakeResponse
  {
    uint16_t CarName[MAX_CHAR_LEN];  // Every other character starting with index 0, terminate with %
    uint16_t DriverName[MAX_CHAR_LEN];
    int32_t  id;           // Get from Query?
    int32_t  version;      // Get from Query?
    uint16_t TrackName[MAX_CHAR_LEN];
    uint16_t TrackConfig[MAX_CHAR_LEN];
  };

  struct RTCarInfo // Supposed to be Length 328
  {
    int32_t id; // alse
    int32_t size;
    float   speed_Kmh;
    float   speed_Mph;
    float   speed_Ms;
    uint8_t isAbsEnabled;
    uint8_t isAbsInAction;
    uint8_t isTcInAction;
    uint8_t isTcEnabled;
    uint8_t isInPit;
    uint8_t isEngineLimiterOn;
    uint8_t unknown0; // M
    uint8_t unknown1; // a
    float   accG_vertical;
    float   accG_horizontal;
    float   accG_frontal;
    int32_t lapTime;
    int32_t lastLap;
    int32_t bestLap;
    int32_t lapCount;
    float   gas;
    float   brake;
    float   clutch;
    float   engineRPM;
    float   steer;
    int32_t gear;
    float   cgHeight;
    float   wheelAngularSpeed[4];
    float   slipAngle[4];
    float   slipAngle_ContactPatch[4];
    float   slipRatio[4];
    float   tireSlip[4];
    float   ndSlip[4];
    float   load[4];
    float   Dy[4];
    float   Mz[4];
    float   tireDirtyLevel[4];
    float   camberRAD[4];
    float   tireRadius[4];
    float   tireLoadedRadius[4];
    float   suspensionHeight[4];
    float   carPositionNormalized;
    float   carSlope;
    float   carCoordinates[3];
  };

  struct RTLap
  {
    int32_t  CarId;
    int32_t  Lap;
    uint16_t DriverName[MAX_CHAR_LEN];
    uint16_t CarName[MAX_CHAR_LEN];
    int32_t Time;
  };

  HandshakeResponse     Response;
  RTCarInfo             UpdateResponse;
  RTLap                 SpotResponse;
  Interpreter::Function TelemetryFunctions[ACTlmEnum::MAX_TLM_TYPE];
  std::string           Executable;
  std::string           Profile;
  void*                 ProcessHandle;
  
  static const size_t HANDSHAKE_QUERY_SIZE    = sizeof(HandShakeQuery);
  static const size_t HANDSHAKE_RESPONSE_SIZE = sizeof(HandshakeResponse);
  static const size_t UPDATE_RESPONSE_SIZE    = sizeof(RTCarInfo);
  static const size_t SPOT_RESPONSE_SIZE      = sizeof(RTLap);
};

