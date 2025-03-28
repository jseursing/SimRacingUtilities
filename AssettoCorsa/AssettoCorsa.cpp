// DummyApplication.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <windows.h>

std::string DriverNames[5] =
{
  "Mad Driver",
  "Midnight Cinderella",
  "Jack Knife",
  "Your Momma",
  "Weenie"
};

std::string CarNames[5] =
{
  "Honda S2000",
  "Honda Civic",
  "Subaru WRX STi",
  "Lexus LFA",
  "Ford Mustang"
};

std::string TrackNames[5] =
{
  "Mt. Panorama",
  "Brands Hatch (GP)",
  "Suzuka Speedway",
  "Laguna Seca",
  "Drifting Map"
};

std::string TrackConfigs[5] =
{
  "Default",
  "Short",
  "Practice",
  "Drag",
  "Free Roam"
};

float PedalPositions[5] =
{
  37.0f,
  80.0f,
  100.0f,
  -50.0f,
  -100.0f
};

struct Engine
{
  float rpm = 0.0;
  float speed_kmh = 0.0;
  float speed_mph = 0.0;
  float speed_ms = 0.0;
  float gas = 0.0;
  float brake = 0.0;
  float clutch = 0.0;
};

Engine engine;
char   DriverName[50];
char   VehicleName[50];
char   TrackName[50];
char   TrackConfig[50];

int main(int argc, char* argv[])
{
  srand(0);

  Engine* engine2 = new Engine;
  uintptr_t baseAddress = (uintptr_t)GetModuleHandleA(0);

  size_t frame = 0;
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (1 < argc)
    {
      // Update pedal positions
      float pedal_pos = PedalPositions[rand() % 5];
      if (0.0f > pedal_pos)
      {
        engine.brake = pedal_pos;
        engine.gas = 0.0f;
        engine.clutch = 100.0f;
      }
      else
      {
        engine.brake = 0.0f;
        engine.gas = pedal_pos;
        engine.clutch = 0.0f;
      }

      pedal_pos = PedalPositions[rand() % 5];
      if (0.0f > pedal_pos)
      {
        engine2->brake = pedal_pos;
        engine2->gas = 0.0f;
        engine2->clutch = 100.0f;
      }
      else
      {
        engine2->brake = 0.0f;
        engine2->gas = pedal_pos;
        engine2->clutch = 0.0f;
      }

      // Update engine statistics
      if (engine.brake)
      {
        engine.rpm = 800;
        engine.speed_kmh -= 0.1 * engine.brake;
      }
      else
      {
        engine.rpm += engine.gas;
        if (engine.rpm >= 8000.0f)
        {
          engine.rpm = 8000.0f;
        }

        engine.speed_kmh += 0.1 * engine.gas;
      }
      if (engine2->brake)
      {
        engine2->rpm = 800;
        engine2->speed_kmh -= 0.1 * engine2->brake;
      }
      else
      {
        engine2->rpm += engine2->gas;
        if (engine2->rpm >= 8000.0f)
        {
          engine2->rpm = 8000.0f;
        }

        engine2->speed_kmh += 0.1 * engine2->gas;
      }

      engine.speed_mph = engine.speed_kmh * 0.621f;
      engine.speed_ms = (engine.speed_kmh * 1000.0f) / 3600.0f;
      engine2->speed_mph = engine2->speed_kmh * 0.621f;
      engine2->speed_ms = (engine2->speed_kmh * 1000.0f) / 3600.0f;

      if (0 == frame % 10)
      {
        std::string val = DriverNames[rand() % 5];
        memset(DriverName, 0, sizeof(DriverName));
        memcpy(DriverName, val.c_str(), val.length());

        val = CarNames[rand() % 5];
        memset(VehicleName, 0, sizeof(VehicleName));
        memcpy(VehicleName, val.c_str(), val.length());

        val = TrackNames[rand() % 5];
        memset(TrackName, 0, sizeof(TrackName));
        memcpy(TrackName, val.c_str(), val.length());

        val = TrackConfigs[rand() % 5];
        memset(TrackConfig, 0, sizeof(TrackConfig));
        memcpy(TrackConfig, val.c_str(), val.length());
      }

      // Display update every second
      printf("===========================================\n"
        "Addresses:\n"
        "  Engine: (rpm, speed_kmh, speed_mph, speed_ms, gas, brake, clutch)\n"
        "    [%f, %f, %f, %f, %f, %f, %f]\n"
        "  Engine_Pointer: (rpm, speed_kmh, speed_mph, speed_ms, gas, brake, clutch)\n"
        "    [%f, %f, %f, %f, %f, %f, %f]\n"
        "  DriverName: [%s]\n"
        "  CarName: [%s]\n"
        "  TrackName: [%s]\n"
        "  TrackConfig: [%s]\n",
        engine.rpm, engine.speed_kmh, engine.speed_mph, engine.speed_ms, engine.gas, engine.brake, engine.clutch,
        engine2->rpm, engine2->speed_kmh, engine2->speed_mph, engine2->speed_ms, engine2->gas, engine2->brake, engine2->clutch,
        DriverName,
        VehicleName,
        TrackName,
        TrackConfig);

      while (0 == GetAsyncKeyState(VK_SPACE))
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }

      ++frame;
    }
  }

  return 0;
}

