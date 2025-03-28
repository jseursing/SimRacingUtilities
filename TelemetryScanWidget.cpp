#include "Interpreter.h"
#include "MemUtils.h"
#include "SRU_Main.h"
#include "TelemetryManager.h"
#include "TelemetryScanWidget.h"
#include "WindowManager.h"
#include "Win32Sem.h"
#include <QComboBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QProgressBar>
#include <QTableWidget>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QTreeWidget>

TelemetryScanWidget* TelemetryScanWidget::ThisInstance = nullptr;

/**
 * Function: TelemetryScanWidget
 * Notes: See header file
 */
TelemetryScanWidget::TelemetryScanWidget(QWidget* parent) :
  QWidget(parent),
  Semaphore(new Win32Sem(1, 1))
{
  ThisInstance = this;
  MemoryUtils = new MemUtils();
  MemoryUtils->SetStatusCallback(SRU_Main::PrintStatus);

  QGridLayout* gLayout = new QGridLayout(this);
  gLayout->setContentsMargins({ 5, 15, 5, 5 });
  gLayout->setSpacing(5);
  setLayout(gLayout);

  TargetComboBox = new QComboBox(this);
  TargetComboBox->setFixedSize(QSize(220, 20));
  gLayout->addWidget(TargetComboBox, 0, 0, 1, 1);

  RefreshButton = new QToolButton(this);
  RefreshButton->setFixedSize(QSize(25, 20));
  RefreshButton->installEventFilter(this);
//  RefreshButton->setText("Refresh");
  RefreshButton->setIcon(QIcon(":/SimRU/resources/refresh.png"));
  RefreshButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  RefreshButton->setLayoutDirection(Qt::RightToLeft);
  connect(RefreshButton, &QToolButton::clicked, this, &TelemetryScanWidget::RefreshTargetsClicked);
  gLayout->addWidget(RefreshButton, 0, 1, 1, 1);

  ValueLabel = new QLabel(this);
  ValueLabel->setFixedWidth(150);
  ValueLabel->setText("    Search Value:");
  gLayout->addWidget(ValueLabel, 0, 2, 1, 1);

  ValueEdit = new QLineEdit(this);
  ValueEdit->setText("");
  gLayout->addWidget(ValueEdit, 0, 3, 1, 1);

  AttachButton = new QToolButton(this);
  AttachButton->setFixedSize(QSize(25, 20));
  AttachButton->installEventFilter(this);
//  AttachButton->setText("Attach");
//  AttachButton->setFixedWidth(100);
  AttachButton->setIcon(QIcon(":/SimRU/resources/inspect.png"));
  AttachButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  AttachButton->setLayoutDirection(Qt::RightToLeft);
  connect(AttachButton, &QToolButton::clicked, this, &TelemetryScanWidget::AttachToTargetClicked);
  gLayout->addWidget(AttachButton, 1, 1, 1, 1);

  ScanProgress = new QProgressBar(this);
  ScanProgress->setFixedSize(QSize(220, 20)); 
  ScanProgress->setTextVisible(false);
  ScanProgress->setStyleSheet("QProgressBar { border: 2px solid white; border-radius: 2px; background: transparent; }"
                              "QProgressBar::chunk { background-color: white; width: 15px;}");
  gLayout->addWidget(ScanProgress, 1, 0, 1, 1);

  QFont font = this->font();
  font.setFamily("Lucida Console");
  font.setBold(true);

  ModuleTree = new QTreeWidget(this);
  ModuleTree->setFont(font);
  ModuleTree->setColumnCount(1);
  ModuleTree->setHeaderHidden(true);
  ModuleTree->setFixedWidth(250);
  gLayout->addWidget(ModuleTree, 2, 0, 2, 2);

  ValueTypeComboBox = new QComboBox(this);
  ValueTypeComboBox->addItem("int8_t"); 
  ValueTypeComboBox->addItem("int16_t");
  ValueTypeComboBox->addItem("int32_t"); 
  ValueTypeComboBox->addItem("float"); 
  ValueTypeComboBox->addItem("string");
  connect(ValueTypeComboBox, &QComboBox::currentIndexChanged, 
          this, &TelemetryScanWidget::ValueTypeIndexChanged);
  gLayout->addWidget(ValueTypeComboBox, 1, 2, 1, 1);

  ModifierComboBox = new QComboBox(this);
  ModifierComboBox->addItem("Exact Value");
  ModifierComboBox->addItem("Changed Value");
  ModifierComboBox->addItem("Unchanged Value");
  ModifierComboBox->addItem("Increased Value");
  ModifierComboBox->addItem("Decreased Value");
  ModifierComboBox->addItem("Unknown Value");
  connect(ModifierComboBox, &QComboBox::currentIndexChanged,
          this, &TelemetryScanWidget::ModifierIndexChanged);
  gLayout->addWidget(ModifierComboBox, 1, 3, 1, 1);

  InitialScanButton = new QToolButton(this);
  InitialScanButton->installEventFilter(this);
  InitialScanButton->setFixedWidth(150);
  InitialScanButton->setText("Initial Scan");
  InitialScanButton->setIcon(QIcon(":/SimRU/resources/search.png"));
  InitialScanButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  InitialScanButton->setLayoutDirection(Qt::RightToLeft);
  connect(InitialScanButton, &QToolButton::clicked,
          this, &TelemetryScanWidget::InitialScanClicked);
  gLayout->addWidget(InitialScanButton, 2, 2, 1, 1);

  NextScanButton = new QToolButton(this);
  NextScanButton->installEventFilter(this);
  NextScanButton->setFixedWidth(150);
  NextScanButton->setText("Filter Results");
  NextScanButton->setIcon(QIcon(":/SimRU/resources/filter.png"));
  NextScanButton->setEnabled(false);
  NextScanButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  NextScanButton->setLayoutDirection(Qt::RightToLeft);
  connect(NextScanButton, &QToolButton::clicked,
          this, &TelemetryScanWidget::NextScanClicked);
  gLayout->addWidget(NextScanButton, 2, 3, 1, 1);

  ResultTable = new QTableWidget(this);
  ResultTable->setFont(font);
  ResultTable->setColumnCount(2);
  ResultTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Address"));
  ResultTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
  ResultTable->horizontalHeader()->setDefaultSectionSize(150);
  ResultTable->horizontalHeader()->setStretchLastSection(true);
  ResultTable->verticalHeader()->setVisible(false);
  ResultTable->verticalHeader()->setDefaultSectionSize(20);
  ResultTable->setShowGrid(false);
  ResultTable->setContextMenuPolicy(Qt::CustomContextMenu);

  QMenu* resultMenu = new QMenu(this);
  QAction* ptrScanAction = new QAction("Scan for pointer");
  connect(ptrScanAction, &QAction::triggered, this, [=]()
    {
      QString strAddress = "";
      if (0 != ResultTable->selectedItems().size())
      {
        size_t row = ResultTable->selectedItems()[0]->row();
        strAddress = ResultTable->item(row, 0)->text();
      }
        
      strAddress = QInputDialog::getText(this, 
                                         "Sim Racing Utilities", 
                                         "Pointer scan for the following address:",
                                         QLineEdit::Normal,
                                         strAddress);
      if (0 == strAddress.length())
      {
        return;
      }
      
      uintptr_t address = strAddress.toULongLong(nullptr, 16);
      std::string module;
      uintptr_t moduleAddr;
      MemoryUtils->GetModule(address, module, moduleAddr);
      if (0 != moduleAddr)
      {
        size_t lastPos = module.find_last_of("\\");
        if (std::string::npos != lastPos)
        {
          module = module.substr(lastPos + 1);
        }

        char strStatus[256] = {0};
        sprintf_s(strStatus, "[INFO] Module: %s (0x%llx) + 0x%04x", 
                  module.c_str(),
                  moduleAddr,
                  address - moduleAddr);
        StatusBox->append(strStatus);
        return;
      }

      Semaphore->Take(-1);
      ScanProgress->setValue(0);
      if (true == MemoryUtils->PointerScan(address, 1024, 2, PointerScanComplete))
      {
        InitialScanButton->setEnabled(false);
        NextScanButton->setEnabled(false);
        ValueEdit->setEnabled(false);
        ValueTypeComboBox->setEnabled(false);
        ModifierComboBox->setEnabled(false);
        ResultTimer->stop();
      }
      else
      {
        Semaphore->Give();
      }
    });
  resultMenu->addAction(ptrScanAction);
  
  connect(ResultTable, &QTableWidget::customContextMenuRequested, this, 
    [=](const QPoint& pos)
    {
      resultMenu->exec(ResultTable->mapToGlobal(pos));
    });
  gLayout->addWidget(ResultTable, 3, 2, 1, -1);

  StatusBox = new QTextEdit(this);
  StatusBox->setFixedHeight(100);
  StatusBox->setFont(font);
  gLayout->addWidget(StatusBox, 4, 0, 1, -1);

  CommandEdit = new QLineEdit(this);
  CommandEdit->setFixedHeight(20);
  CommandEdit->setFont(font);
  CommandEdit->setFrame(true);
  connect(CommandEdit, &QLineEdit::returnPressed, this, &TelemetryScanWidget::OnCommandReturnPressed);
  gLayout->addWidget(CommandEdit, 5, 0, 1, -1);

  connect(this, &TelemetryScanWidget::ScanCompleteSignal, 
          this, &TelemetryScanWidget::ScanCompleteSlot);
  connect(this, &TelemetryScanWidget::PointerScanCompleteSignal, 
          this, &TelemetryScanWidget::PointerScanCompleteSlot);

  ResultTimer = new QTimer();
  ResultTimer->setInterval(1000);
  connect(ResultTimer, &QTimer::timeout, this, &TelemetryScanWidget::ResultTimerElapsed);
}

/**
 * Function: ~TelemetryScanWidget
 * Notes: See header file
 */
TelemetryScanWidget::~TelemetryScanWidget()
{

}

/**
 * Function: PrintStatusSlot
 * Notes: See header file
 */
void TelemetryScanWidget::PrintStatusSlot(char* buf)
{
  std::string status = (buf);
  delete[] buf;
  StatusBox->append(status.c_str());
}

/**
 * Function: RefreshTargetsClicked
 * Notes: See header file
 */
void TelemetryScanWidget::RefreshTargetsClicked()
{
  WindowManager* wMgr = WindowManager::Instance();
  wMgr->RefreshApplications();

  TargetComboBox->blockSignals(true);
  TargetComboBox->clear();

  std::vector<std::string> applications = wMgr->GetAppList();
  for (std::string app : applications)
  {
    TargetComboBox->addItem(app.c_str());
  }

  TargetComboBox->blockSignals(false);
  Beep(100, 10);
}

/**
 * Function: AttachToTargetClicked
 * Notes: See header file
 */
void TelemetryScanWidget::AttachToTargetClicked()
{
  void* handle = MemoryUtils->GetProcessHandle(TargetComboBox->currentText().toStdString());
  if (INVALID_HANDLE_VALUE != handle)
  {
    // Stop telemetry so we don't fight over ProcScanner
    emit StopTelemetryServerSignal();
    if (true == MemoryUtils->SetActiveProcess(handle))
    {
      StatusBox->append("[INFO] Successfully attached to target");

      std::vector<MemUtils::ModuleEntry> modules;
      MemoryUtils->GetModuleList(modules);
      
      ModuleTree->clear();
      for (size_t i = 0; i < modules.size(); ++i)
      {
        std::string trunc_str = modules[i].ModuleName;
        size_t last_pos = trunc_str.find_last_of("\\");
        if (std::string::npos != last_pos)
        {
          trunc_str = trunc_str.substr(last_pos + 1);
        }

        QTreeWidgetItem* topNode = new QTreeWidgetItem(ModuleTree);
        topNode->setText(0, trunc_str.c_str());

        QTreeWidgetItem* node = new QTreeWidgetItem(topNode);
        node->setText(0, "Base: 0x" + QString::number(modules[i].BaseAddress, 16));
        node = new QTreeWidgetItem(topNode);
        node->setText(0, "Size: 0x" + QString::number(modules[i].ModuleSize, 16));
        ModuleTree->addTopLevelItem(topNode);
      }

      ScanProgress->setMaximum(4);
    }
    else
    {
      StatusBox->append("[ERROR] Failed attaching to target");
    }
  }
}

/**
 * Function: ValueTypeIndexChanged
 * Notes: See header file
 */
void TelemetryScanWidget::ValueTypeIndexChanged(int index)
{
  ValueEdit->setText("");
}

/**
 * Function: ModifierIndexChanged
 * Notes: See header file
 */
void TelemetryScanWidget::ModifierIndexChanged(int index)
{
  switch (index)
  {
    case MemUtils::EXACT_VALUE:
      ValueEdit->setEnabled(true);
    break; // Do nothing
    default:
      ValueEdit->setText("");
      ValueEdit->setEnabled(false);
    break;
  }
}

/**
 * Function: InitialScanClicked
 * Notes: See header file
 */
void TelemetryScanWidget::InitialScanClicked()
{
  if ((MemUtils::EXACT_VALUE == ModifierComboBox->currentIndex()) &&
      (0 == ValueEdit->text().length()))
  {
    return;
  }

  std::vector<char> value_buf;
  std::string strValue = ValueEdit->text().toStdString();
  switch (ValueTypeComboBox->currentIndex())
  {
    case MemUtils::SCAN_CHAR:
    {
      value_buf.resize(1, 0);
      uint8_t value = static_cast<uint8_t>(strtoul(strValue.c_str(), 0, 10));
      memcpy(value_buf.data(), &value, sizeof(value));
    }
    break;
    case MemUtils::SCAN_SHORT:
    {
      value_buf.resize(2, 0);
      uint16_t value = static_cast<uint16_t>(strtoul(strValue.c_str(), 0, 10));
      memcpy(value_buf.data(), &value, sizeof(value));
    }
    break;
    case MemUtils::SCAN_INT:
    {
      value_buf.resize(4, 0);
      uint32_t value = static_cast<uint8_t>(strtoul(strValue.c_str(), 0, 10));
      memcpy(value_buf.data(), &value, sizeof(value));
    }
    break;
    case MemUtils::SCAN_FLOAT:
    {
      value_buf.resize(4, 0);
      float value = strtof(strValue.c_str(), nullptr);
      memcpy(value_buf.data(), &value, sizeof(value));
    }
    break;
    case MemUtils::SCAN_ARRAY:
    {
      value_buf.resize(ValueEdit->text().length(), 0);
      memcpy(value_buf.data(), strValue.c_str(), value_buf.size());
      break;
    }
    default: break;
  }

  Semaphore->Take(-1);
  ScanProgress->setValue(0);
  if (true == MemoryUtils->NewScan(static_cast<MemUtils::ScanTypeEnum>(ValueTypeComboBox->currentIndex()),
                                   static_cast<MemUtils::ScanModifierEnum>(ModifierComboBox->currentIndex()),
                                   value_buf.data(),
                                   value_buf.size(),
                                   ScanComplete))
  {
    ResultTable->setRowCount(0);
    InitialScanButton->setEnabled(false);
    NextScanButton->setEnabled(false);
    ValueEdit->setEnabled(false);
    ValueTypeComboBox->setEnabled(false);
    ModifierComboBox->setEnabled(false);
    ResultTimer->stop();
  }
  else
  {
    Semaphore->Give();
  }
}

/**
 * Function: NextScanClicked
 * Notes: See header file
 */
void TelemetryScanWidget::NextScanClicked()
{
  if ((MemUtils::EXACT_VALUE == ModifierComboBox->currentIndex()) &&
      (0 == ValueEdit->text().length()))
  {
    return;
  }

  std::vector<char> value_buf;
  std::string strValue = ValueEdit->text().toStdString();
  if (MemUtils::EXACT_VALUE == ModifierComboBox->currentIndex())
  {
    switch (ValueTypeComboBox->currentIndex())
    {
      case MemUtils::SCAN_CHAR:
      {
        value_buf.resize(1, 0);
        uint8_t value = static_cast<uint8_t>(strtoul(strValue.c_str(), 0, 10));
        memcpy(value_buf.data(), &value, sizeof(value));
      }
      break;
      case MemUtils::SCAN_SHORT:
      {
        value_buf.resize(2, 0);
        uint16_t value = static_cast<uint16_t>(strtoul(strValue.c_str(), 0, 10));
        memcpy(value_buf.data(), &value, sizeof(value));
      }
      break;
      case MemUtils::SCAN_INT:
      {
        value_buf.resize(4, 0);
        uint32_t value = static_cast<uint8_t>(strtoul(strValue.c_str(), 0, 10));
        memcpy(value_buf.data(), &value, sizeof(value));
      }
      break;
      case MemUtils::SCAN_FLOAT:
      {
        value_buf.resize(4, 0);
        float value = strtof(ValueEdit->text().toStdString().c_str(), nullptr);
        memcpy(value_buf.data(), &value, sizeof(value));
      }
      break;
      case MemUtils::SCAN_ARRAY:
      {
        value_buf.resize(ValueEdit->text().length(), 0);
        memcpy(value_buf.data(), strValue.c_str(), value_buf.size());
        break;
      }
      default: break;
    }
  }

  Semaphore->Take(-1);
  ScanProgress->setValue(0);
  if (true == MemoryUtils->NextScan(static_cast<MemUtils::ScanModifierEnum>(ModifierComboBox->currentIndex()),
                                    value_buf.data(),
                                    value_buf.size(),
                                    ScanComplete))
  {
    ResultTable->setRowCount(0);
    InitialScanButton->setEnabled(false);
    NextScanButton->setEnabled(false);
    ValueEdit->setEnabled(false);
    ValueTypeComboBox->setEnabled(false);
    ModifierComboBox->setEnabled(false);
    ResultTimer->stop();
  }
  else
  {
    Semaphore->Give();
  }
}

/**
 * Function: OnCommandReturnPressed
 * Notes: See header file
 */
void TelemetryScanWidget::OnCommandReturnPressed()
{
  std::string command = CommandEdit->text().toStdString();
  CommandEdit->clear();
  StatusBox->append(command.c_str());

  Interpreter::Function function = Interpreter::Translate(command);
  switch (function.Type)
  {
    case Interpreter::SET_FLAG:
      if ("dev" == function.Parameters[0])
      {
        Interpreter::InvokeFunction(function, MemoryUtils);
        StatusBox->append("Developer mode: " + 
                          QString(true == Interpreter::DeveloperMode ? "enabled" : "disabled"));
      }
      break;
    case Interpreter::HELP:
      StatusBox->append((true == Interpreter::DeveloperMode ? Interpreter::HelpPromptDev.c_str() : 
                                                              Interpreter::HelpPrompt.c_str()));
      break;
    default:
    {
      char outputStr[512] = {0};

      Interpreter::ReturnType fRet = Interpreter::InvokeFunction(function, MemoryUtils);
      switch (function.Type)
      {
        case Interpreter::READ_8:
          sprintf_s(outputStr, "  Result: %u (%02X)", fRet.u8Ret, fRet.u8Ret);
          break;
        case Interpreter::READ_8I:
          sprintf_s(outputStr, "  Result: %d (%02X)", fRet.i8Ret, fRet.i8Ret);
          break;
        case Interpreter::READ_16:
          sprintf_s(outputStr, "  Result: %u (%04X)", fRet.u16Ret, fRet.u16Ret);
          break;
        case Interpreter::READ_16I:
          sprintf_s(outputStr, "  Result: %d (%04X)", fRet.i16Ret, fRet.i16Ret);
          break;
        case Interpreter::READ_32:
          sprintf_s(outputStr, "  Result: %u (%04X)", fRet.u32Ret, fRet.u32Ret);
          break;
        case Interpreter::READ_32I:
          sprintf_s(outputStr, "  Result: %d (%04X)", fRet.i32Ret, fRet.i32Ret);
          break;
        case Interpreter::READ_FLOAT:
          sprintf_s(outputStr, "  Result: %f (%08X)", fRet.fRet, *(uint32_t*)(&(fRet.fRet)));
          break;
        case Interpreter::READ_ARRAY:
          sprintf_s(outputStr, "  Result: %s", fRet.arrRet.data());
          break;
        case Interpreter::WRITE_8:
        case Interpreter::WRITE_8I:
        case Interpreter::WRITE_16:
        case Interpreter::WRITE_16I:
        case Interpreter::WRITE_32:
        case Interpreter::WRITE_32I:
        case Interpreter::WRITE_FLOAT:
        case Interpreter::WRITE_ARRAY:
          sprintf_s(outputStr, "  Result: %d", fRet.bRet);
          break;
        default:
          sprintf_s(outputStr, "  Unknown function");
      }

      StatusBox->append(outputStr);
    }
  }
}

/**
 * Function: ResultTimerElapsed
 * Notes: See header file
 */
void TelemetryScanWidget::ResultTimerElapsed()
{
  if (0 != ResultTable->rowCount())
  {
    Semaphore->Take(-1);

    std::vector<uintptr_t> addresses;
    std::vector<std::string> values;
    MemoryUtils->GetScanResults(addresses, values);

    if (1000 >= addresses.size())
    {
      for (size_t i = 0; i < addresses.size(); ++i)
      {
        ResultTable->item(i, 1)->setText(values[i].c_str());
      }
    }

    Semaphore->Give();
  }
}

/** 
 * Function: ScanComplete
 * Notes: See header file
 */
void TelemetryScanWidget::ScanComplete(int progress)
{
  emit Instance()->ScanCompleteSignal(progress);
}

/**
 * Function: PointerScanComplete
 * Notes: See header file
 */
void TelemetryScanWidget::PointerScanComplete(int progress)
{
  emit Instance()->PointerScanCompleteSignal(progress);
}

/**
 * Function: ScanCompleteSlot
 * Notes: See header file
 */
void TelemetryScanWidget::ScanCompleteSlot(int progress)
{
  ScanProgress->setValue(progress);
  if (4 != progress)
  {
    return;
  }

  std::vector<uintptr_t> addresses; 
  std::vector<std::string> values;
  MemoryUtils->GetScanResults(addresses, values);
  if (1000 >= addresses.size())
  {
    QFont font = ResultTable->font();
    font.setItalic(true);

    ResultTable->setRowCount(addresses.size());
    
    for (size_t i = 0; i < addresses.size(); ++i)
    {
      char temp[20] = {0};
      sprintf_s(temp, "0x%llx", addresses[i]);
      ResultTable->setItem(i, 0, new QTableWidgetItem(temp));
      ResultTable->setItem(i, 1, new QTableWidgetItem(values[i].c_str()));
  
      std::string module;
      uintptr_t moduleAddr;
      MemoryUtils->GetModule(addresses[i], module, moduleAddr);
      if (0 != moduleAddr)
      {
        ResultTable->item(i, 0)->setFont(font);
      }
    }
  }
  else
  {
    StatusBox->append("[INFO] Results exceed 1000, not displaying");
  }

  InitialScanButton->setEnabled(true);
  NextScanButton->setEnabled(true);
  ValueEdit->setEnabled(true);
  ValueTypeComboBox->setEnabled(true);
  ModifierComboBox->setEnabled(true);
  ResultTimer->start();
  Semaphore->Give();
}

/**
 * Function: PointerScanCompleteSlot
 * Notes: See header file
 */
void TelemetryScanWidget::PointerScanCompleteSlot(int progress)
{
  ScanProgress->setValue(progress);
  if (4 != progress)
  {
    return;
  }
  
  std::vector<uintptr_t> addresses;
  std::vector<uintptr_t> offsets;
  MemoryUtils->GetPointerResults(addresses, offsets);

  QTableWidget* resultTable = new QTableWidget();
  resultTable->setFixedWidth(350);
  resultTable->setColumnCount(2);
  resultTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Address"));
  resultTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Offset"));
  resultTable->verticalHeader()->setVisible(false);
  resultTable->horizontalHeader()->setDefaultSectionSize(250);
  resultTable->setColumnWidth(1, 100);
  resultTable->setRowCount(addresses.size());
  
  for (size_t i = 0; i < addresses.size(); ++i)
  {
    uintptr_t address = addresses[i];
    
    QString addressStr = "0x" + QString::number(addresses[i], 16);
    std::string moduleName;
    uintptr_t moduleAddr = 0; 
    MemoryUtils->GetModule(address, moduleName, moduleAddr);
    if (0 != moduleAddr)
    {
      size_t lastPos = moduleName.find_last_of("\\");
      if (std::string::npos != lastPos)
      {
        moduleName = moduleName.substr(lastPos + 1);
      }
      addressStr = QString(QString::number(addresses[i], 16) + "(" + moduleName.c_str()) + "+0x" +
                           QString::number(address - moduleAddr, 16) + ")";
    }
    
    resultTable->setItem(i, 0, new QTableWidgetItem(addressStr));
    resultTable->setItem(i, 1, new QTableWidgetItem("0x" + QString::number(offsets[i], 16)));
  }

  InitialScanButton->setEnabled(true);
  NextScanButton->setEnabled(true);
  ValueEdit->setEnabled(true);
  ValueTypeComboBox->setEnabled(true);
  ModifierComboBox->setEnabled(true);
  ResultTimer->start();
  ResultTimer->start();
  Semaphore->Give();
  resultTable->show();
}

/**
 * Function: Instance
 * Notes: See header file
 */
TelemetryScanWidget* TelemetryScanWidget::Instance()
{
  return ThisInstance;
}