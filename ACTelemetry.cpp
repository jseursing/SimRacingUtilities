#include "ACTelemetry.h"
#include "MemUtils.h"
#include <cstring>
#include <format>

/**
 * Function: GetProfileName
 * Notes: See header file
 */
std::string ACTelemetry::GetProfileName() const
{
  return Profile;
}

/**
 * Function: SetProfileName
 * Notes: See header file
 */
void ACTelemetry::SetProfileName(std::string profile)
{
  Profile = profile;
}

/**
 * Function: GetExecutableName
 * Notes: See header file
 */
std::string ACTelemetry::GetExecutableName() const
{
  return Executable;
}

/**
 * Function: SetExecutableName
 * Notes: See header file
 */
void ACTelemetry::SetExecutableName(std::string exe)
{
  Executable = exe;
}

/**
 * Function: GetProcessHandle
 * Notes: See header file
 */
void* ACTelemetry::GetProcessHandle() const
{
  return ProcessHandle;
}

/**
 * Function: SetProcessHandle
 * Notes: See header file
 */
void ACTelemetry::SetProcessHandle(void* handle)
{
  ProcessHandle = handle;
}

/**
 * Function: ParseHandshakeQuery
 * Notes: See header file
 */
bool ACTelemetry::ParseHandshakeQuery(char* query,
                                      size_t bufSize,
                                      int32_t& id,
                                      int32_t& version,
                                      ACTlmEnum::OperationEnum& operation) const
{
  if (HANDSHAKE_QUERY_SIZE <= bufSize)
  {
    HandShakeQuery* handshake = reinterpret_cast<HandShakeQuery*>(query);
    id        = handshake->id;
    version   = handshake->version;
    operation = static_cast<ACTlmEnum::OperationEnum>(handshake->operation);
    return true;
  }

  return false;
}

/**
 * Function: GetHandshakeResponse
 * Notes: See header file
 */
bool ACTelemetry::GetHandshakeResponse(char* destBuf, size_t bufSize, int32_t id, int32_t version)
{
  if (bufSize == HANDSHAKE_RESPONSE_SIZE)
  {
    Response.id = id;
    Response.version = version;
    memcpy(destBuf, &Response, HANDSHAKE_RESPONSE_SIZE);
    return true;
  }

  return false;
}

/**
 * Function: GetUpdateResponse
 * Notes: See header file
 */
bool ACTelemetry::GetUpdateResponse(char* destBuf, size_t bufSize)
{
  if (bufSize == UPDATE_RESPONSE_SIZE)
  {
    memcpy(destBuf, &UpdateResponse, UPDATE_RESPONSE_SIZE);
    return true;
  }

  return false;
}

/**
 * Function: GetSpotResponse
 * Notes: See header file
 */
bool ACTelemetry::GetSpotResponse(char* destBuf, size_t bufSize)
{
  if (bufSize == SPOT_RESPONSE_SIZE)
  {
    memcpy(destBuf, &SpotResponse, SPOT_RESPONSE_SIZE);
    return true;
  }

  return false;
}

/**
 * Function: SetTelemetry
 * Notes: See header file
 */
void ACTelemetry::SetTelemetry(ACTlmEnum::TlmTypeEnum type, std::string value)
{
  size_t i = 0;
  switch (type)
  {
    case ACTlmEnum::HS_CAR_NAME:
      memset(Response.CarName, 0, sizeof(Response.CarName));
      for (i = 0; i < value.length() && i < MAX_CHAR_LEN - 1; ++i)
      {
        Response.CarName[i] = static_cast<uint16_t>(value[i]);
      }
      Response.CarName[i] = static_cast<uint16_t>('%');
    break;
    case ACTlmEnum::HS_DRIVER_NAME:
      memset(Response.DriverName, 0, sizeof(Response.DriverName));
      for (i = 0; i < value.length() && i < MAX_CHAR_LEN - 1; ++i)
      {
        Response.DriverName[i] = static_cast<uint16_t>(value[i]);
      }
      Response.DriverName[i] = static_cast<uint16_t>('%');
      break;
    case ACTlmEnum::HS_TRACK_NAME:
      memset(Response.TrackName, 0, sizeof(Response.TrackName));
      for (i = 0; i < value.length() && i < MAX_CHAR_LEN - 1; ++i)
      {
        Response.TrackName[i] = static_cast<uint16_t>(value[i]);
      }
      Response.TrackName[i] = static_cast<uint16_t>('%');
      break;
    case ACTlmEnum::HS_TRACK_CONFIG:
      memset(Response.TrackConfig, 0, sizeof(Response.TrackConfig));
      for (i = 0; i < value.length() && i < MAX_CHAR_LEN - 1; ++i)
      {
        Response.TrackConfig[i] = static_cast<uint16_t>(value[i]);
      }
      Response.TrackConfig[i] = static_cast<uint16_t>('%');
      break;
    case ACTlmEnum::UPDATE_SPEED_KMH:
      UpdateResponse.speed_Kmh = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_SPEED_MPH:
      UpdateResponse.speed_Mph = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_SPEED_MS:
      UpdateResponse.speed_Ms = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_ABS_ENABLED:
      UpdateResponse.isAbsEnabled = Interpreter::ToInteger(value);
      break;
    case ACTlmEnum::UPDATE_ABS_ACTIVE:
      UpdateResponse.isAbsInAction = Interpreter::ToInteger(value);
      break;
    case ACTlmEnum::UPDATE_TC_ACTIVE:
      UpdateResponse.isTcInAction = Interpreter::ToInteger(value);
      break;
    case ACTlmEnum::UPDATE_TC_ENABLED:
      UpdateResponse.isTcEnabled = Interpreter::ToInteger(value);
      break;
    case ACTlmEnum::UPDATE_IN_PITS:
      UpdateResponse.isInPit = Interpreter::ToInteger(value);
      break;
    case ACTlmEnum::UPDATE_ENGINE_LIMITER_ON:
      UpdateResponse.isEngineLimiterOn = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_ACCG_VERTICAL:
      UpdateResponse.accG_vertical = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_ACCG_HORIZONTAL:
      UpdateResponse.accG_horizontal = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_ACCG_FRONTAL:
      UpdateResponse.accG_frontal = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_LAP_TIME:
      UpdateResponse.lapTime = Interpreter::ToInteger(value);
      break;
    case ACTlmEnum::UPDATE_LAST_LAP:
      UpdateResponse.lastLap = Interpreter::ToInteger(value);
      break;
    case ACTlmEnum::UPDATE_BEST_LAP:
      UpdateResponse.bestLap = Interpreter::ToInteger(value);
      break;
    case ACTlmEnum::UPDATE_LAP_COUNT:
      UpdateResponse.lapCount = Interpreter::ToInteger(value);
      break;
    case ACTlmEnum::UPDATE_GAS:
      UpdateResponse.gas = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_BRAKE:
      UpdateResponse.brake = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_CLUTCH:
      UpdateResponse.clutch = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_RPM:
      UpdateResponse.engineRPM = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_STEER:
      UpdateResponse.steer = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_GEAR:
      UpdateResponse.gear = Interpreter::ToInteger(value);
      break;
    case ACTlmEnum::UPDATE_CG_HEIGHT:
      UpdateResponse.cgHeight = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_FL:
    case ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_FR:
    case ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_RL:
    case ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_RR:
      UpdateResponse.wheelAngularSpeed[type - ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_FL] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_SLIP_ANGLE_FL:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_FR:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_RL:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_RR:
      UpdateResponse.slipAngle[type - ACTlmEnum::UPDATE_SLIP_ANGLE_FL] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_FL:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_FR:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_RL:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_RR:
      UpdateResponse.slipAngle_ContactPatch[type - ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_FL] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_SLIP_RATIO_FL:
    case ACTlmEnum::UPDATE_SLIP_RATIO_FR:
    case ACTlmEnum::UPDATE_SLIP_RATIO_RL:
    case ACTlmEnum::UPDATE_SLIP_RATIO_RR:
      UpdateResponse.slipRatio[type - ACTlmEnum::UPDATE_SLIP_RATIO_FL] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_TIRE_SLIP_FL:
    case ACTlmEnum::UPDATE_TIRE_SLIP_FR:
    case ACTlmEnum::UPDATE_TIRE_SLIP_RL:
    case ACTlmEnum::UPDATE_TIRE_SLIP_RR:
      UpdateResponse.tireSlip[type - ACTlmEnum::UPDATE_TIRE_SLIP_FL] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_ND_SLIP_FL:
    case ACTlmEnum::UPDATE_ND_SLIP_FR:
    case ACTlmEnum::UPDATE_ND_SLIP_RL:
    case ACTlmEnum::UPDATE_ND_SLIP_RR:
      UpdateResponse.ndSlip[type - ACTlmEnum::UPDATE_ND_SLIP_FL] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_LOAD_FL:
    case ACTlmEnum::UPDATE_LOAD_FR:
    case ACTlmEnum::UPDATE_LOAD_RL:
    case ACTlmEnum::UPDATE_LOAD_RR:
      UpdateResponse.load[type - ACTlmEnum::UPDATE_LOAD_FL] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_DY_FL:
    case ACTlmEnum::UPDATE_DY_FR:
    case ACTlmEnum::UPDATE_DY_RL:
    case ACTlmEnum::UPDATE_DY_RR:
      UpdateResponse.Dy[type - ACTlmEnum::UPDATE_DY_FL] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_MZ_FL:
    case ACTlmEnum::UPDATE_MZ_FR:
    case ACTlmEnum::UPDATE_MZ_RL:
    case ACTlmEnum::UPDATE_MZ_RR:
      UpdateResponse.Mz[type - ACTlmEnum::UPDATE_MZ_FL] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_FL:
    case ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_FR:
    case ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_RL:
    case ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_RR:
      UpdateResponse.tireDirtyLevel[type - ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_FL] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_CAMBER_RAD_FL:
    case ACTlmEnum::UPDATE_CAMBER_RAD_FR:
    case ACTlmEnum::UPDATE_CAMBER_RAD_RL:
    case ACTlmEnum::UPDATE_CAMBER_RAD_RR:
      UpdateResponse.camberRAD[type - ACTlmEnum::UPDATE_CAMBER_RAD_FL] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_TIRE_RADIUS_FL:
    case ACTlmEnum::UPDATE_TIRE_RADIUS_FR:
    case ACTlmEnum::UPDATE_TIRE_RADIUS_RL:
    case ACTlmEnum::UPDATE_TIRE_RADIUS_RR:
      UpdateResponse.tireRadius[type - ACTlmEnum::UPDATE_TIRE_RADIUS_FL] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_FL:
    case ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_FR:
    case ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_RL:
    case ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_RR:
      UpdateResponse.tireLoadedRadius[type - ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_FL] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_FL:
    case ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_FR:
    case ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_RL:
    case ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_RR:
      UpdateResponse.suspensionHeight[type - ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_FL] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_CAR_POS_NORM:
      UpdateResponse.carPositionNormalized = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_CAR_SLOPE:
      UpdateResponse.carSlope = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::UPDATE_CAR_COORD_X:
    case ACTlmEnum::UPDATE_CAR_COORD_Y:
    case ACTlmEnum::UPDATE_CAR_COORD_Z:
      UpdateResponse.carCoordinates[type - ACTlmEnum::UPDATE_CAR_COORD_X] = strtof(value.c_str(), nullptr);
      break;
    case ACTlmEnum::SPOT_CAR_ID:
      SpotResponse.CarId = Interpreter::ToInteger(value);
      break;
    case ACTlmEnum::SPOT_LAP:
      SpotResponse.Lap = Interpreter::ToInteger(value);
      break;
    case ACTlmEnum::SPOT_DRIVER_NAME:
      memset(SpotResponse.DriverName, 0, sizeof(SpotResponse.DriverName));
      for (i = 0; i < value.length() && i < MAX_CHAR_LEN - 1; ++i)
      {
        SpotResponse.DriverName[i] = static_cast<uint16_t>(value[i]);
      }
      SpotResponse.DriverName[i] = static_cast<uint16_t>('%');
      break;
    case ACTlmEnum::SPOT_CAR_NAME:
      memset(SpotResponse.CarName, 0, sizeof(SpotResponse.CarName));
      for (i = 0; i < value.length() && i < MAX_CHAR_LEN - 1; ++i)
      {
        SpotResponse.CarName[i] = static_cast<uint16_t>(value[i]);
      }
      SpotResponse.CarName[i] = static_cast<uint16_t>('%');
      break;
    case ACTlmEnum::SPOT_TIME:
      SpotResponse.Time = Interpreter::ToInteger(value);
      break;
    default: break;
  }
}

/**
 * Function: GetTelemetry
 * Notes: See header file
 */
std::string ACTelemetry::GetTelemetry(ACTlmEnum::TlmTypeEnum type)
{
  std::string tlmValue = "";
  switch (type)
  {
    case ACTlmEnum::HS_CAR_NAME:
      for (size_t i = 0; i < MAX_CHAR_LEN - 1; ++i)
      {
        if ('%' == Response.CarName[i])
          break;
        tlmValue += static_cast<char>(Response.CarName[i]);
      }
      break;
    case ACTlmEnum::HS_DRIVER_NAME:
      for (size_t i = 0; i < MAX_CHAR_LEN - 1; ++i)
      {
        if ('%' == Response.DriverName[i])
          break;
        tlmValue += static_cast<char>(Response.DriverName[i]);
      }
      break;
    case ACTlmEnum::HS_TRACK_NAME:
      for (size_t i = 0; i < MAX_CHAR_LEN - 1; ++i)
      {
        if ('%' == Response.TrackName[i])
          break;
        tlmValue += static_cast<char>(Response.TrackName[i]);
      }
      break;
    case ACTlmEnum::HS_TRACK_CONFIG:
      for (size_t i = 0; i < MAX_CHAR_LEN - 1; ++i)
      {
        if ('%' == Response.TrackConfig[i])
          break;
        tlmValue += static_cast<char>(Response.TrackConfig[i]);
      }
      break;
    case ACTlmEnum::UPDATE_SPEED_KMH:
      tlmValue = std::to_string(UpdateResponse.speed_Kmh);
      break;
    case ACTlmEnum::UPDATE_SPEED_MPH:
      tlmValue = std::to_string(UpdateResponse.speed_Mph);
      break;
    case ACTlmEnum::UPDATE_SPEED_MS:
      tlmValue = std::to_string(UpdateResponse.speed_Ms);
      break;
    case ACTlmEnum::UPDATE_ABS_ENABLED:
      tlmValue = std::to_string(UpdateResponse.isAbsEnabled);
      break;
    case ACTlmEnum::UPDATE_ABS_ACTIVE:
      tlmValue = std::to_string(UpdateResponse.isAbsInAction);
      break;
    case ACTlmEnum::UPDATE_TC_ACTIVE:
      tlmValue = std::to_string(UpdateResponse.isTcInAction);
      break;
    case ACTlmEnum::UPDATE_TC_ENABLED:
      tlmValue = std::to_string(UpdateResponse.isTcEnabled);
      break;
    case ACTlmEnum::UPDATE_IN_PITS:
      tlmValue = std::to_string(UpdateResponse.isInPit);
      break;
    case ACTlmEnum::UPDATE_ENGINE_LIMITER_ON:
      tlmValue = std::to_string(UpdateResponse.isEngineLimiterOn);
      break;
    case ACTlmEnum::UPDATE_ACCG_VERTICAL:
      tlmValue = std::to_string(UpdateResponse.accG_vertical);
      break;
    case ACTlmEnum::UPDATE_ACCG_HORIZONTAL:
      tlmValue = std::to_string(UpdateResponse.accG_horizontal);
      break;
    case ACTlmEnum::UPDATE_ACCG_FRONTAL:
      tlmValue = std::to_string(UpdateResponse.accG_frontal);
      break;
    case ACTlmEnum::UPDATE_LAP_TIME:
      tlmValue = std::to_string(UpdateResponse.lapTime);
      break;
    case ACTlmEnum::UPDATE_LAST_LAP:
      tlmValue = std::to_string(UpdateResponse.lastLap);
      break;
    case ACTlmEnum::UPDATE_BEST_LAP:
      tlmValue = std::to_string(UpdateResponse.bestLap);
      break;
    case ACTlmEnum::UPDATE_LAP_COUNT:
      tlmValue = std::to_string(UpdateResponse.lapCount);
      break;
    case ACTlmEnum::UPDATE_GAS:
      tlmValue = std::to_string(UpdateResponse.gas);
      break;
    case ACTlmEnum::UPDATE_BRAKE:
      tlmValue = std::to_string(UpdateResponse.brake);
      break;
    case ACTlmEnum::UPDATE_CLUTCH:
      tlmValue = std::to_string(UpdateResponse.clutch);
      break;
    case ACTlmEnum::UPDATE_RPM:
      tlmValue = std::to_string(UpdateResponse.engineRPM);
      break;
    case ACTlmEnum::UPDATE_STEER:
      tlmValue = std::to_string(UpdateResponse.steer);
      break;
    case ACTlmEnum::UPDATE_GEAR:
      tlmValue = std::to_string(UpdateResponse.gear);
      break;
    case ACTlmEnum::UPDATE_CG_HEIGHT:
      tlmValue = std::to_string(UpdateResponse.cgHeight);
      break;
    case ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_FL:
    case ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_FR:
    case ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_RL:
    case ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_RR:
      tlmValue = std::to_string(UpdateResponse.wheelAngularSpeed[type - ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_FL]);
      break;
    case ACTlmEnum::UPDATE_SLIP_ANGLE_FL:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_FR:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_RL:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_RR:
      tlmValue = std::to_string(UpdateResponse.slipAngle[type - ACTlmEnum::UPDATE_SLIP_ANGLE_FL]);
      break;
    case ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_FL:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_FR:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_RL:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_RR:
      tlmValue = std::to_string(UpdateResponse.slipAngle_ContactPatch[type - ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_FL]);
      break;
    case ACTlmEnum::UPDATE_SLIP_RATIO_FL:
    case ACTlmEnum::UPDATE_SLIP_RATIO_FR:
    case ACTlmEnum::UPDATE_SLIP_RATIO_RL:
    case ACTlmEnum::UPDATE_SLIP_RATIO_RR:
      tlmValue = std::to_string(UpdateResponse.slipRatio[type - ACTlmEnum::UPDATE_SLIP_RATIO_FL]);
      break;
    case ACTlmEnum::UPDATE_TIRE_SLIP_FL:
    case ACTlmEnum::UPDATE_TIRE_SLIP_FR:
    case ACTlmEnum::UPDATE_TIRE_SLIP_RL:
    case ACTlmEnum::UPDATE_TIRE_SLIP_RR:
      tlmValue = std::to_string(UpdateResponse.tireSlip[type - ACTlmEnum::UPDATE_TIRE_SLIP_FL]);
      break;
    case ACTlmEnum::UPDATE_ND_SLIP_FL:
    case ACTlmEnum::UPDATE_ND_SLIP_FR:
    case ACTlmEnum::UPDATE_ND_SLIP_RL:
    case ACTlmEnum::UPDATE_ND_SLIP_RR:
      tlmValue = std::to_string(UpdateResponse.ndSlip[type - ACTlmEnum::UPDATE_ND_SLIP_FL]);
      break;
    case ACTlmEnum::UPDATE_LOAD_FL:
    case ACTlmEnum::UPDATE_LOAD_FR:
    case ACTlmEnum::UPDATE_LOAD_RL:
    case ACTlmEnum::UPDATE_LOAD_RR:
      tlmValue = std::to_string(UpdateResponse.load[type - ACTlmEnum::UPDATE_LOAD_FL]);
      break;
    case ACTlmEnum::UPDATE_DY_FL:
    case ACTlmEnum::UPDATE_DY_FR:
    case ACTlmEnum::UPDATE_DY_RL:
    case ACTlmEnum::UPDATE_DY_RR:
      tlmValue = std::to_string(UpdateResponse.Dy[type - ACTlmEnum::UPDATE_DY_FL]);
      break;
    case ACTlmEnum::UPDATE_MZ_FL:
    case ACTlmEnum::UPDATE_MZ_FR:
    case ACTlmEnum::UPDATE_MZ_RL:
    case ACTlmEnum::UPDATE_MZ_RR:
      tlmValue = std::to_string(UpdateResponse.Mz[type - ACTlmEnum::UPDATE_MZ_FL]);
      break;
    case ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_FL:
    case ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_FR:
    case ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_RL:
    case ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_RR:
      tlmValue = std::to_string(UpdateResponse.tireDirtyLevel[type - ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_FL]);
      break;
    case ACTlmEnum::UPDATE_CAMBER_RAD_FL:
    case ACTlmEnum::UPDATE_CAMBER_RAD_FR:
    case ACTlmEnum::UPDATE_CAMBER_RAD_RL:
    case ACTlmEnum::UPDATE_CAMBER_RAD_RR:
      tlmValue = std::to_string(UpdateResponse.camberRAD[type - ACTlmEnum::UPDATE_CAMBER_RAD_FL]);
      break;
    case ACTlmEnum::UPDATE_TIRE_RADIUS_FL:
    case ACTlmEnum::UPDATE_TIRE_RADIUS_FR:
    case ACTlmEnum::UPDATE_TIRE_RADIUS_RL:
    case ACTlmEnum::UPDATE_TIRE_RADIUS_RR:
      tlmValue = std::to_string(UpdateResponse.tireRadius[type - ACTlmEnum::UPDATE_TIRE_RADIUS_FL]);
      break;
    case ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_FL:
    case ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_FR:
    case ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_RL:
    case ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_RR:
      tlmValue = std::to_string(UpdateResponse.tireLoadedRadius[type - ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_FL]);
      break;
    case ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_FL:
    case ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_FR:
    case ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_RL:
    case ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_RR:
      tlmValue = std::to_string(UpdateResponse.suspensionHeight[type - ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_FL]);
      break;
    case ACTlmEnum::UPDATE_CAR_POS_NORM:
      tlmValue = std::to_string(UpdateResponse.carPositionNormalized);
      break;
    case ACTlmEnum::UPDATE_CAR_SLOPE:
      tlmValue = std::to_string(UpdateResponse.carSlope);
      break;
    case ACTlmEnum::UPDATE_CAR_COORD_X:
    case ACTlmEnum::UPDATE_CAR_COORD_Y:
    case ACTlmEnum::UPDATE_CAR_COORD_Z:
      tlmValue = std::to_string(UpdateResponse.carCoordinates[type - ACTlmEnum::UPDATE_CAR_COORD_X]);
      break;
    case ACTlmEnum::SPOT_CAR_ID:
      tlmValue = std::to_string(SpotResponse.CarId);
      break;
    case ACTlmEnum::SPOT_LAP:
      tlmValue = std::to_string(SpotResponse.Lap);
      break;
    case ACTlmEnum::SPOT_DRIVER_NAME:
      for (size_t i = 0; i < MAX_CHAR_LEN - 1; ++i)
      {
        if ('%' == SpotResponse.DriverName[i])
          break;
        tlmValue += static_cast<char>(SpotResponse.DriverName[i]);
      }
      break;
    case ACTlmEnum::SPOT_CAR_NAME:
      for (size_t i = 0; i < MAX_CHAR_LEN - 1; ++i)
      {
        if ('%' == SpotResponse.CarName[i])
          break;
        tlmValue += static_cast<char>(SpotResponse.CarName[i]);
      }
      break;
    case ACTlmEnum::SPOT_TIME:
      tlmValue = std::to_string(SpotResponse.Time);
      break;
    default: break;
  }

  return tlmValue;
}

/**
 * Function: SetTelemetryFunction
 * Notes: See header file
 */
void ACTelemetry::SetTelemetryFunction(ACTlmEnum::TlmTypeEnum type, std::string value)
{
  Interpreter::Function function = Interpreter::Translate(value);
  
  TelemetryFunctions[type].Type       = function.Type;
  TelemetryFunctions[type].Parameters = function.Parameters;
  TelemetryFunctions[type].Modifier   = function.Modifier;

  // This is here because i get a weird string assignment error on first run...
  TelemetryFunctions[type].ModifierValue.resize(function.ModifierValue.length() + 1, 0);
  memset(TelemetryFunctions[type].ModifierValue.data(), 
         0, 
         TelemetryFunctions[type].ModifierValue.size());
  memcpy(TelemetryFunctions[type].ModifierValue.data(),
        function.ModifierValue.data(),
        function.ModifierValue.length());

  if (Interpreter::VALUE == TelemetryFunctions[type].Type)
  {
    SetTelemetry(type, value);
  }
}

/**
 * Function: GetTelemetryFunction
 * Notes: See header file
 */
void ACTelemetry::GetTelemetryFunction(ACTlmEnum::TlmTypeEnum type, Interpreter::Function& function)
{
  function.Type = TelemetryFunctions[type].Type;
  function.Parameters = TelemetryFunctions[type].Parameters;
  function.Modifier = TelemetryFunctions[type].Modifier;
  function.ModifierValue = TelemetryFunctions[type].ModifierValue;
}

/**
 * Function: GetTelemetryFunctionString
 * Notes: See header file
 */
std::string ACTelemetry::GetTelemetryFunctionString(ACTlmEnum::TlmTypeEnum type)
{
  std::string function = Interpreter::GetFunctionString(TelemetryFunctions[type].Type) + "(";
  
  if (Interpreter::VALUE != TelemetryFunctions[type].Type)
  {
    for (size_t i = 0; i < TelemetryFunctions[type].Parameters.size(); ++i)
    {
      function += TelemetryFunctions[type].Parameters[i] + ",";
    }

    function.back() = ')';
    
    if (Interpreter::MAX_MODIFIER_TYPE != TelemetryFunctions[type].Modifier)
    {
      function += Interpreter::GetModifierString(TelemetryFunctions[type].Modifier) +
                  TelemetryFunctions[type].ModifierValue;
    }
  }
  else
  {
    function = TelemetryFunctions[type].Parameters[0];
  }

  return function;
}

/**
 * Function: ACTelemetry
 * Notes: See header file
 */
ACTelemetry::ACTelemetry() :
  Executable("DummyExecutable.exe"),
  Profile("Default Profile")
{
  // Zero out
  memset(&Response, 0, sizeof(Response));
  memset(&UpdateResponse, 0, sizeof(UpdateResponse));
  memset(&SpotResponse, 0, sizeof(SpotResponse));
  memset(&TelemetryFunctions, 0, sizeof(TelemetryFunctions));
  for (size_t i = ACTlmEnum::HS_CAR_NAME; i < ACTlmEnum::MAX_TLM_TYPE; ++i)
  {
    TelemetryFunctions[i].Parameters.push_back("0");
    TelemetryFunctions[i].Modifier = Interpreter::MAX_MODIFIER_TYPE;
    TelemetryFunctions[i].ModifierValue = "";
  }

  // Prepopulate "fixed" telemetry values
  char* idPtr = reinterpret_cast<char*>(&(UpdateResponse.id));
  idPtr[0] = 'a';
  idPtr[1] = 'l';
  idPtr[2] = 's';
  idPtr[3] = 'e';
  UpdateResponse.size = UPDATE_RESPONSE_SIZE;
  UpdateResponse.unknown0 = 'M';
  UpdateResponse.unknown1 = 'a';

  // Populate initial telemetry values
  SetTelemetryFunction(ACTlmEnum::HS_CAR_NAME,     "SimRacingUtils");
  SetTelemetryFunction(ACTlmEnum::HS_DRIVER_NAME,  "SimRacer");
  SetTelemetryFunction(ACTlmEnum::HS_TRACK_NAME,   "Mt. Panorama");
  SetTelemetryFunction(ACTlmEnum::HS_TRACK_CONFIG, "Default");
}

/**
 * Function: ~ACTelemetry
 * Notes: See header file
 */
ACTelemetry::~ACTelemetry()
{

}

/**
 * Function: GetMnemonic
 * Notes: See header file
 */
std::string ACTelemetry::GetMnemonic(ACTlmEnum::TlmTypeEnum type)
{
  std::string wheel_fmt[] = { "(FL)", "(FR)", "(RL)", "(RR)" };
  std::string mnemonic = "(" + std::to_string(type) + ") ";
  switch (type)
  {
    case ACTlmEnum::HS_CAR_NAME:
      mnemonic += "Car Name";
      break;
    case ACTlmEnum::HS_DRIVER_NAME:
      mnemonic += "Driver Name";
      break;
    case ACTlmEnum::HS_TRACK_NAME:
      mnemonic += "Track Name";
      break;
    case ACTlmEnum::HS_TRACK_CONFIG:
      mnemonic += "Track Configuration";
      break;
    case ACTlmEnum::UPDATE_SPEED_KMH:
      mnemonic += "Speed (KMH)";
      break;
    case ACTlmEnum::UPDATE_SPEED_MPH:
      mnemonic += "Speed (MPH)";
      break;
    case ACTlmEnum::UPDATE_SPEED_MS:
      mnemonic += "Speed (MS)";
      break;
    case ACTlmEnum::UPDATE_ABS_ENABLED:
      mnemonic += "ABS Enabled";
      break;
    case ACTlmEnum::UPDATE_ABS_ACTIVE:
      mnemonic += "ABS Active";
      break;
    case ACTlmEnum::UPDATE_TC_ACTIVE:
      mnemonic += "TC Active";
      break;
    case ACTlmEnum::UPDATE_TC_ENABLED:
      mnemonic += "TC Enabled";
      break;
    case ACTlmEnum::UPDATE_IN_PITS:
      mnemonic += "In Pits";
      break;
    case ACTlmEnum::UPDATE_ENGINE_LIMITER_ON:
      mnemonic += "Engine Limiter Active";
      break;
    case ACTlmEnum::UPDATE_ACCG_VERTICAL:
      mnemonic += "Acc. G (Vertical)";
      break;
    case ACTlmEnum::UPDATE_ACCG_HORIZONTAL:
      mnemonic += "Acc. G (Horizontal)";
      break;
    case ACTlmEnum::UPDATE_ACCG_FRONTAL:
      mnemonic += "Acc. G. (Frontal)";
      break;
    case ACTlmEnum::UPDATE_LAP_TIME:
      mnemonic += "Lap Time";
      break;
    case ACTlmEnum::UPDATE_LAST_LAP:
      mnemonic += "Last Lap";
      break;
    case ACTlmEnum::UPDATE_BEST_LAP:
      mnemonic += "Best Lap";
      break;
    case ACTlmEnum::UPDATE_LAP_COUNT:
      mnemonic += "Lap Count";
      break;
    case ACTlmEnum::UPDATE_GAS:
      mnemonic += "Gas";
      break;
    case ACTlmEnum::UPDATE_BRAKE:
      mnemonic += "Brake";
      break;
    case ACTlmEnum::UPDATE_CLUTCH:
      mnemonic += "Clutch";
      break;
    case ACTlmEnum::UPDATE_RPM:
      mnemonic += "Engine RPM";
      break;
    case ACTlmEnum::UPDATE_STEER:
      mnemonic += "Steer";
      break;
    case ACTlmEnum::UPDATE_GEAR:
      mnemonic += "Transmission Gear";
      break;
    case ACTlmEnum::UPDATE_CG_HEIGHT:
      mnemonic += "Cg. Height";
      break;
    case ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_FL:
    case ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_FR:
    case ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_RL:
    case ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_RR:
      mnemonic += "Wheel Angular Speed " + wheel_fmt[type - ACTlmEnum::UPDATE_WHEEL_ANG_SPEED_FL];
      break;
    case ACTlmEnum::UPDATE_SLIP_ANGLE_FL:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_FR:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_RL:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_RR:
      mnemonic += "Slip Angle " + wheel_fmt[type - ACTlmEnum::UPDATE_SLIP_ANGLE_FL];
      break;
    case ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_FL:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_FR:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_RL:
    case ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_RR:
      mnemonic += "Slip Angle Contact Patch " + wheel_fmt[type - ACTlmEnum::UPDATE_SLIP_ANGLE_CONT_PATCH_FL];
      break;
    case ACTlmEnum::UPDATE_SLIP_RATIO_FL:
    case ACTlmEnum::UPDATE_SLIP_RATIO_FR:
    case ACTlmEnum::UPDATE_SLIP_RATIO_RL:
    case ACTlmEnum::UPDATE_SLIP_RATIO_RR:
      mnemonic += "Slip Ratio " + wheel_fmt[type - ACTlmEnum::UPDATE_SLIP_RATIO_FL];
      break;
    case ACTlmEnum::UPDATE_TIRE_SLIP_FL:
    case ACTlmEnum::UPDATE_TIRE_SLIP_FR:
    case ACTlmEnum::UPDATE_TIRE_SLIP_RL:
    case ACTlmEnum::UPDATE_TIRE_SLIP_RR:
      mnemonic += "Tire Slip " + wheel_fmt[type - ACTlmEnum::UPDATE_TIRE_SLIP_FL];
      break;
    case ACTlmEnum::UPDATE_ND_SLIP_FL:
    case ACTlmEnum::UPDATE_ND_SLIP_FR:
    case ACTlmEnum::UPDATE_ND_SLIP_RL:
    case ACTlmEnum::UPDATE_ND_SLIP_RR:
      mnemonic += "Nd. Slip " + wheel_fmt[type - ACTlmEnum::UPDATE_ND_SLIP_FL];
      break;
    case ACTlmEnum::UPDATE_LOAD_FL:
    case ACTlmEnum::UPDATE_LOAD_FR:
    case ACTlmEnum::UPDATE_LOAD_RL:
    case ACTlmEnum::UPDATE_LOAD_RR:
      mnemonic += "Load " + wheel_fmt[type - ACTlmEnum::UPDATE_LOAD_FL];
      break;
    case ACTlmEnum::UPDATE_DY_FL:
    case ACTlmEnum::UPDATE_DY_FR:
    case ACTlmEnum::UPDATE_DY_RL:
    case ACTlmEnum::UPDATE_DY_RR:
      mnemonic += "Dy " + wheel_fmt[type - ACTlmEnum::UPDATE_DY_FL];
      break;
    case ACTlmEnum::UPDATE_MZ_FL:
    case ACTlmEnum::UPDATE_MZ_FR:
    case ACTlmEnum::UPDATE_MZ_RL:
    case ACTlmEnum::UPDATE_MZ_RR:
      mnemonic += "Mz " + wheel_fmt[type - ACTlmEnum::UPDATE_MZ_FL];
      break;
    case ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_FL:
    case ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_FR:
    case ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_RL:
    case ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_RR:
      mnemonic += "Tire Dirty Level " + wheel_fmt[type - ACTlmEnum::UPDATE_TIRE_DIRTY_LEVEL_FL];
      break;
    case ACTlmEnum::UPDATE_CAMBER_RAD_FL:
    case ACTlmEnum::UPDATE_CAMBER_RAD_FR:
    case ACTlmEnum::UPDATE_CAMBER_RAD_RL:
    case ACTlmEnum::UPDATE_CAMBER_RAD_RR:
      mnemonic += "Camber Radius " + wheel_fmt[type - ACTlmEnum::UPDATE_CAMBER_RAD_FL];
      break;
    case ACTlmEnum::UPDATE_TIRE_RADIUS_FL:
    case ACTlmEnum::UPDATE_TIRE_RADIUS_FR:
    case ACTlmEnum::UPDATE_TIRE_RADIUS_RL:
    case ACTlmEnum::UPDATE_TIRE_RADIUS_RR:
      mnemonic += "Tire Radius " + wheel_fmt[type - ACTlmEnum::UPDATE_TIRE_RADIUS_FL];
      break;
    case ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_FL:
    case ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_FR:
    case ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_RL:
    case ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_RR:
      mnemonic += "Tire Loaded Radius " + wheel_fmt[type - ACTlmEnum::UPDATE_TIRE_LOADED_RADIUS_FL];
      break;
    case ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_FL:
    case ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_FR:
    case ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_RL:
    case ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_RR:
      mnemonic += "Suspension Height " + wheel_fmt[type - ACTlmEnum::UPDATE_SUSPENSION_HEIGHT_FL];
      break;
    case ACTlmEnum::UPDATE_CAR_POS_NORM:
      mnemonic += "Car Position (normalized)";
      break;
    case ACTlmEnum::UPDATE_CAR_SLOPE:
      mnemonic += "Car Slope";
      break;
    case ACTlmEnum::UPDATE_CAR_COORD_X:
      mnemonic += "Car Coordinates (X)";
      break;
    case ACTlmEnum::UPDATE_CAR_COORD_Y:
      mnemonic += "Car Coordinates (Y)";
      break;
    case ACTlmEnum::UPDATE_CAR_COORD_Z:
      mnemonic += "Car Coordinates (Z)";
      break;
    case ACTlmEnum::SPOT_CAR_ID:
      mnemonic += "Car Identifier";
      break;
    case ACTlmEnum::SPOT_LAP:
      mnemonic += "Lap";
      break;
    case ACTlmEnum::SPOT_DRIVER_NAME:
      mnemonic += "Driver Name";
      break;
    case ACTlmEnum::SPOT_CAR_NAME:
      mnemonic += "Car Name";
      break;
    case ACTlmEnum::SPOT_TIME:
      mnemonic += "Time";
      break;
    default: 
      mnemonic += "INVALID TELEMETRY";
    break;
  }

  return mnemonic;
}

/**
 * Function: GetHandshakeQuerySize
 * Notes: See header file
 */
size_t ACTelemetry::GetHandshakeQuerySize()
{
  return HANDSHAKE_QUERY_SIZE;
}

/**
 * Function: GetHandshakeResponseSize
 * Notes: See header file
 */
size_t ACTelemetry::GetHandshakeResponseSize()
{
  return HANDSHAKE_RESPONSE_SIZE;
}

/**
 * Function: GetUpdateResponseSize
 * Notes: See header file
 */
size_t ACTelemetry::GetUpdateResponseSize()
{
  return UPDATE_RESPONSE_SIZE;
}

/**
 * Function: GetSpotResponseSize
 * Notes: See header file
 */
size_t ACTelemetry::GetSpotResponseSize()
{
  return SPOT_RESPONSE_SIZE;
}