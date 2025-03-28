#pragma once
#include <string>

class QColor;

class GlobalConst
{
public:
  static QColor BlueGray;  
  static QColor Magenta;
  static const size_t FIXED_WIDTH  = 600;
  static const size_t FIXED_HEIGHT = 500;

  enum MgrEnum
  {
    WINDOW_MGR    = 1,
    TELEMETRY_MGR = 2
  };
};

enum StatusDestEnum
{
  TLM_DEST  = 1,
  SCAN_DEST = 2,
};

typedef void(*fnStatusCallback)(StatusDestEnum, std::string);
typedef void(*fnScanCompleteCallback)(int);