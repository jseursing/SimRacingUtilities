#include "AppEntryWidget.h"
#include "FramelessInterface.h"
#include "GlobalConst.h"
#include "MemUtils.h"
#include "SRU_Main.h"
#include "TelemetryScanWidget.h"
#include "TelemetryManager.h"
#include "TelemetryMgrWidget.h"
#include "WindowManager.h"
#include "WindowMgrWidget.h"
#include <fstream>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMovie>
#include <QPainter>
#include <QPainterPath>
#include <QToolButton>
#include <QTableWidget>
#include <QTabWidget>
#include <QTimer>
#include <Windows.h>

SRU_Main* SRU_Main::Instance = nullptr;

SRU_Main* SRU_Main::GetInstance()
{
  return Instance;
}

void SRU_Main::ImportSettings()
{
  std::ifstream input_stream(".\\SRUGlobals.dat");
  if (true == input_stream.good())
  {
    std::vector<char> data;
    while (false == input_stream.eof())
    {
      char buf[512] = {0};
      input_stream.read(buf, 1);
      data.clear();

      switch (buf[0])
      {
      case GlobalConst::WINDOW_MGR:
      {
        input_stream.read(buf, 2); // Read total size

        short dataSize = *reinterpret_cast<short*>(buf);
        if (0 >= dataSize)
        {
          return;
        }

        data.resize(dataSize, 0);
        input_stream.read(data.data(), data.size());
        WindowManager::Instance()->Import(data);
      }
      break;
      case GlobalConst::TELEMETRY_MGR:
      {
        input_stream.read(buf, 2); // Read total size

        short dataSize = *reinterpret_cast<short*>(buf);
        if (0 >= dataSize)
        {
          return;
        }

        data.resize(dataSize, 0);
        input_stream.read(data.data(), data.size());
        TelemetryManager::Instance()->Import(data);
      }
      break;
      }
    }

    input_stream.close();
  }
}

void SRU_Main::ExportSettings()
{
  std::vector<char> data;
  WindowManager::Instance()->Export(data);
  TelemetryManager::Instance()->Export(data);
  
  std::ofstream output_stream(".\\SRUGlobals.dat");
  if (true == output_stream.good())
  {
    output_stream.write(data.data(), data.size());
    output_stream.close();
  }
}

void SRU_Main::PrintStatus(StatusDestEnum destination, std::string status)
{
  char* sigBuf = new char[status.length() + 1];
  memcpy(sigBuf, status.c_str(), status.length());
  sigBuf[status.length()] = 0;

  if ((destination & TLM_DEST) && (destination & SCAN_DEST))
  {
    emit Instance->PrintTlmStatusSignal(sigBuf);

    sigBuf = new char[status.length() + 1];
    memcpy(sigBuf, status.c_str(), status.length());
    sigBuf[status.length()] = 0;

    emit Instance->PrintScanStatusSignal(sigBuf);
  }
  else
  {
    if (destination & TLM_DEST)
    {
      emit Instance->PrintTlmStatusSignal(sigBuf);
    }
    else
    {
      emit Instance->PrintScanStatusSignal(sigBuf);
    }
  }
}

SRU_Main::SRU_Main(FramelessInterface* framelessInterface) :
  QMainWindow(nullptr),
  Frame(framelessInterface)
{
  Instance = this;

  // Initialize globals
  GlobalConst::BlueGray = QColor(QRgb(0x1D232A));
  GlobalConst::Magenta  = QColor(QRgb(0x690F98));

  //
  EventTimer = new QTimer(this);
  connect(EventTimer, &QTimer::timeout, this, &SRU_Main::TimedEventTimeout);
  EventTimer->start(500);

  // Set window parameters
  QFont mainFont = this->font();
  mainFont.setFamily("Lucida Console");
  mainFont.setBold(true);
  this->setFont(mainFont);

  Frame->SetTitle("  Sim Racing Utilities");
  Frame->setMinimumWidth(GlobalConst::FIXED_WIDTH);
  Frame->setMaximumWidth(GlobalConst::FIXED_WIDTH);
  Frame->setMinimumHeight(GlobalConst::FIXED_HEIGHT);
  Frame->setMaximumHeight(GlobalConst::FIXED_HEIGHT);
  Frame->SetBackgroundImage(":/SimRU/resources/background.png");
  Frame->SetMinimizeIcon(":/SimRU/resources/minimize.png");
  Frame->SetMaximizeIcon(":/SimRU/resources/maximize.png");
  Frame->SetRestoreIcon(":/SimRU/resources/restore.png");
  Frame->SetCloseIcon(":/SimRU/resources/close.png");
  
  // Build Tab interface
  TabWidget = new QTabWidget(this);
  TabWidget->setStyleSheet("QTableWidget {background:transparent} QTabWidget::pane{border: 1px;border-color:white;background-color: transparent;}"
                           "QTabBar::tab {background-color: transparent; color: white; width: 185px; height:24px; font-size:12px}"
                           "QTabBar::tab:hover{ background-color: transparent; color: black; }"
                           "QTabBar::tab:selected{ background-color: transparent; color: #black; }"
                           "QTreeWidget {background:transparent}"
                           "QToolButton {background:transparent}"
                           "QTextEdit {background:transparent}"
                           "QToolButton {color: white}"
                           "QToolButton:hover {color: black}"
                           "QComboBox {background-color: transparent; color: white; border: 1px solid white;}"
                           "QTreeWidget {color: white}"
                           "QLineEdit {background-color: transparent; color: white; border-width: 1px; border-color:white;}"
                           "QTextEdit {background-color: transparent; color: white; border-style: solid; border-width: 1px; border-color:white;}"
                           "QLabel {background-color: transparent; color: white}"
                           "QInputDialog {border-image:url(:/SimRU/resources/background.png) 0 0 0 0 stretch stretch; background-color: transparent}");
  setContentsMargins(QMargins(5, 15, 5, 5));
  setCentralWidget(TabWidget);

  WindowMgr = new WindowMgrWidget(this);
  TabWidget->addTab(WindowMgr, "Window Management");
  TelemetryMgr = new TelemetryMgrWidget(this);
  TabWidget->addTab(TelemetryMgr, "Telemetry Management");
  TelemetryScanMgr = new TelemetryScanWidget(this);
  TabWidget->addTab(TelemetryScanMgr, "Memory Utilities");

  connect(this, &SRU_Main::PrintTlmStatusSignal, 
          TelemetryMgr, &TelemetryMgrWidget::PrintStatusSlot); // this
  connect(this, &SRU_Main::PrintScanStatusSignal, 
          TelemetryScanMgr, &TelemetryScanWidget::PrintStatusSlot); // this
  connect(TelemetryScanMgr, &TelemetryScanWidget::StopTelemetryServerSignal,
          TelemetryMgr, &TelemetryMgrWidget::StopTelemetryServer);
  TelemetryManager::Instance()->SetStatusCallback(SRU_Main::PrintStatus);

  // Import last session
  ImportSettings();
}

SRU_Main::~SRU_Main()
{

}

void SRU_Main::RefreshProcsClicked()
{
  WindowManager* wMgr = WindowManager::Instance();
  
  // Collect and display applications
  wMgr->RefreshApplications();
  WindowMgr->RefreshWindows();
  TelemetryMgr->RefreshApps();

  Beep(100, 10);
}


void SRU_Main::TimedEventTimeout()
{
  static size_t timer_frame = 1;
  static bool   init_call = true;

  if (0 == timer_frame % 6) // Init and check windows every 3 seconds
  {
    if (true == init_call)
    {
      init_call = false;
      RefreshProcsClicked();
      WindowMgr->ImportWindowSettings();
      TelemetryMgr->ImportTelemetrySettings();
      TelemetryScanMgr->RefreshTargetsClicked();
    }

    WindowManager::Instance()->SynchronizeWindows();
  }

  TelemetryMgr->UpdateTelemetryTree();
  ++timer_frame;
}