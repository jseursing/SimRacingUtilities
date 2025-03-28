#pragma once
#include "GlobalConst.h"
#include <QMainWindow>

class FramelessInterface;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMovie;
class QToolButton;
class QTabWidget;
class QTableWidget;
class QTextEdit;
class QTimer;
class QTreeWidget;
class QTreeWidgetItem;
class TelemetryScanWidget;
class TelemetryMgrWidget;
class WindowMgrWidget;

class SRU_Main : public QMainWindow
{
  Q_OBJECT

public:
  static SRU_Main* GetInstance();
  void ImportSettings();
  void ExportSettings();
  static void PrintStatus(StatusDestEnum dest, std::string status);
  SRU_Main(FramelessInterface* framelessInterface);
  ~SRU_Main();

signals:
  void PrintTlmStatusSignal(char* buf);
  void PrintScanStatusSignal(char* buf);

public slots:
  void RefreshProcsClicked();
  void TimedEventTimeout();
  
private:

  static SRU_Main*    Instance;
  FramelessInterface* Frame;
  QTabWidget*         TabWidget;

  // Timed events
  QTimer*             EventTimer;

  // Window Mgr Tab
  WindowMgrWidget*     WindowMgr;
  TelemetryMgrWidget*  TelemetryMgr;
  TelemetryScanWidget* TelemetryScanMgr;
};

