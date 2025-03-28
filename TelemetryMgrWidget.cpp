#include "GlobalConst.h"
#include "SRU_Main.h"
#include "TelemetryManager.h"
#include "TelemetryMgrWidget.h"
#include "TlmNodeWidget.h"
#include "WindowManager.h"
#include <QComboBox>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QMovie>
#include <QToolButton>
#include <QTabWidget>
#include <QTextEdit>
#include <QTreeView>
#include <QTreeWidget>


/** 
 * Function: RefreshApps
 * Notes: See header file
 */
void TelemetryMgrWidget::RefreshApps()
{
  WindowManager* wMgr = WindowManager::Instance();

  ProcessComboBox->blockSignals(true);
  ProcessComboBox->clear();

  std::vector<std::string> applications = wMgr->GetAppList();
  for (std::string app : applications)
  {
    ProcessComboBox->addItem(app.c_str());
  }

  // Update executable box
  int32_t index = ProcessComboBox->findText(TelemetryManager::Instance()->GetTargetExecutable().c_str());
  if (-1 != index)
  {
    ProcessComboBox->setCurrentIndex(index);
  }
  else
  {
    ProcessComboBox->addItem(TelemetryManager::Instance()->GetTargetExecutable().c_str());
    ProcessComboBox->setCurrentIndex(ProcessComboBox->count() - 1);
  }

  ProcessComboBox->blockSignals(false);
}

/**
 * Function: UpdateTelemetryTree
 * Notes: See header file
 */
void TelemetryMgrWidget::UpdateTelemetryTree()
{
  TelemetryManager* tlmMgr = TelemetryManager::Instance();

  // Load telemetry values
  std::string profile = TlmProfileComboBox->currentText().toStdString().c_str();
  size_t topLevelNodes = TelemetryTree->topLevelItemCount();
  for (size_t i = 0; i < topLevelNodes; ++i)
  {
    TlmNodeWidget* topNode = static_cast<TlmNodeWidget*>(TelemetryTree->topLevelItem(i));
    size_t childNodes = topNode->childCount();
    for (size_t j = 0; j < childNodes; ++j)
    {
      TlmNodeWidget* node = static_cast<TlmNodeWidget*>(topNode->child(j));
      node->setText(1, tlmMgr->GetTelemetry(profile, node->GetTlmType()).c_str());
      node->setText(2, tlmMgr->GetTelemetryFunction(profile, node->GetTlmType()).c_str());
    }
  }

  TelemetryTree->resizeColumnToContents(0);
  TelemetryTree->resizeColumnToContents(1);
  TelemetryTree->resizeColumnToContents(2);
}

/**
 * Function: ImportTelemetrySettings
 * Notes: See header file
 */
void TelemetryMgrWidget::ImportTelemetrySettings()
{
  TelemetryManager* tlmMgr = TelemetryManager::Instance();

  // Build profiles...
  TlmProfileComboBox->blockSignals(true);
  TlmProfileComboBox->clear();
  std::vector<std::string> profiles = tlmMgr->GetProfiles();
  for (std::string profile : profiles)
  {
    TlmProfileComboBox->addItem(profile.c_str());
  }
  TlmProfileComboBox->blockSignals(false);
  
  TlmRefreshButton->setFocus();
  SaveProfileButton->setFocus();
  StartTlmButton->setFocus();
}

/**
 * Function: TelemetryMgrWidget
 * Notes: See header file
 */
TelemetryMgrWidget::TelemetryMgrWidget(QWidget* parent) :
  QWidget(parent)
{
  QGridLayout* gLayout = new QGridLayout(this);
  gLayout->setContentsMargins({5, 15, 5, 5});
  gLayout->setSpacing(5);
  setLayout(gLayout);

  TlmProfileComboBox = new QComboBox(this);
  TlmProfileComboBox->setFixedSize(QSize(400, 25));
  connect(TlmProfileComboBox, &QComboBox::currentIndexChanged, this, &TelemetryMgrWidget::TlmProfileChanged);
  gLayout->addWidget(TlmProfileComboBox, 0, 0, 1, 4);

  SaveProfileButton = new QToolButton(this);
  SaveProfileButton->setText("Save");
  SaveProfileButton->setFixedSize(QSize(70, 30));
  SaveProfileButton->setIcon(QIcon(":/SimRU/resources/save.png"));
  SaveProfileButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  SaveProfileButton->setLayoutDirection(Qt::RightToLeft);
  connect(SaveProfileButton, &QToolButton::clicked, this, &TelemetryMgrWidget::SaveProfileClicked);
  gLayout->addWidget(SaveProfileButton, 0, 4, 1, 1, Qt::AlignRight);

  DeleteProfileButton = new QToolButton(this);
  DeleteProfileButton->setText("Delete");
  DeleteProfileButton->setFixedSize(QSize(80, 30));
  DeleteProfileButton->setIcon(QIcon(":/SimRU/resources/delete.png"));
  DeleteProfileButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  DeleteProfileButton->setLayoutDirection(Qt::RightToLeft);
  connect(DeleteProfileButton, &QToolButton::clicked, this, &TelemetryMgrWidget::DeleteProfileClicked);
  gLayout->addWidget(DeleteProfileButton, 0, 5, 1, 1, Qt::AlignRight);

  QFont font = this->font();
  TelemetryTree = new QTreeWidget(this);
  TelemetryTree->setFont(font);
  TelemetryTree->setHeaderHidden(true);
  TelemetryTree->setFixedHeight(GlobalConst::FIXED_HEIGHT - 250);
  TelemetryTree->setColumnCount(3);
  connect(TelemetryTree, &QTreeWidget::itemDoubleClicked, this, &TelemetryMgrWidget::TlmNodeItemDoubleClicked);
  gLayout->addWidget(TelemetryTree, 1, 0, 1, 6);

  ProcessComboBox = new QComboBox(this);
  ProcessComboBox->setFixedSize(QSize(200, 25));
  connect(ProcessComboBox, &QComboBox::currentIndexChanged, this, &TelemetryMgrWidget::TlmProcessChanged);
  gLayout->addWidget(ProcessComboBox, 2, 0, 1, 2);

  TlmRefreshButton = new QToolButton(this);
  TlmRefreshButton->setFixedSize(QSize(90, 30));
  TlmRefreshButton->setText("Refresh");
  TlmRefreshButton->setIcon(QIcon(":/SimRU/resources/refresh.png"));
  TlmRefreshButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  TlmRefreshButton->setLayoutDirection(Qt::RightToLeft);
  connect(TlmRefreshButton, &QToolButton::clicked, SRU_Main::GetInstance(), &SRU_Main::RefreshProcsClicked);
  gLayout->addWidget(TlmRefreshButton, 2, 2, 1, 1);

  StartTlmButton = new QToolButton(this);
  StartTlmButton->setFixedSize(QSize(120, 30));
  StartTlmButton->setText("Activate");
  StartTlmButton->setIcon(QIcon(":/SimRU/resources/run.png"));
  StartTlmButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  StartTlmButton->setLayoutDirection(Qt::RightToLeft);
  connect(StartTlmButton, &QToolButton::clicked, this, &TelemetryMgrWidget::StartTlmServerClicked);
  gLayout->addWidget(StartTlmButton, 2, 3, 1, 1);

  SubscriberLabel = new QLabel(this);
  SubscriberLabel->setText("Subscribers: 0");
  SubscriberLabel->setFixedWidth(130);
  gLayout->addWidget(SubscriberLabel, 2, 4, 1, 2);

  StatusBox = new QTextEdit(this);
  StatusBox->setFont(font);
  gLayout->addWidget(StatusBox, 3, 0, 1, 6);
  
  // Build Telemetry Tree
  TlmNodeWidget* parentNode = new TlmNodeWidget(TelemetryTree, ACTlmEnum::MAX_TLM_TYPE);
  parentNode->setText(0, "New Session");

  TlmNodeWidget* node = new TlmNodeWidget(parentNode, ACTlmEnum::HS_CAR_NAME);
  node->setText(0, ACTelemetry::GetMnemonic(node->GetTlmType()).c_str());
  node = new TlmNodeWidget(parentNode, ACTlmEnum::HS_DRIVER_NAME);
  node->setText(0, ACTelemetry::GetMnemonic(node->GetTlmType()).c_str());
  node = new TlmNodeWidget(parentNode, ACTlmEnum::HS_TRACK_NAME);
  node->setText(0, ACTelemetry::GetMnemonic(node->GetTlmType()).c_str());
  node = new TlmNodeWidget(parentNode, ACTlmEnum::HS_TRACK_CONFIG);
  node->setText(0, ACTelemetry::GetMnemonic(node->GetTlmType()).c_str());
  TelemetryTree->addTopLevelItem(parentNode);

  parentNode = new TlmNodeWidget(TelemetryTree, ACTlmEnum::MAX_TLM_TYPE);
  parentNode->setText(0, "Car Update");
  for (size_t i = ACTlmEnum::UPDATE_SPEED_KMH; i <= ACTlmEnum::UPDATE_CAR_COORD_Z; ++i)
  {
    node = new TlmNodeWidget(parentNode, static_cast<ACTlmEnum::TlmTypeEnum>(i));
    node->setText(0, ACTelemetry::GetMnemonic(node->GetTlmType()).c_str());
  }
  TelemetryTree->addTopLevelItem(parentNode);

  parentNode = new TlmNodeWidget(TelemetryTree, ACTlmEnum::MAX_TLM_TYPE);
  parentNode->setText(0, "Spot Update");
  node = new TlmNodeWidget(parentNode, ACTlmEnum::SPOT_CAR_ID);
  node->setText(0, ACTelemetry::GetMnemonic(node->GetTlmType()).c_str());
  node = new TlmNodeWidget(parentNode, ACTlmEnum::SPOT_LAP);
  node->setText(0, ACTelemetry::GetMnemonic(node->GetTlmType()).c_str());
  node = new TlmNodeWidget(parentNode, ACTlmEnum::SPOT_DRIVER_NAME);
  node->setText(0, ACTelemetry::GetMnemonic(node->GetTlmType()).c_str());
  node = new TlmNodeWidget(parentNode, ACTlmEnum::SPOT_CAR_NAME);
  node->setText(0, ACTelemetry::GetMnemonic(node->GetTlmType()).c_str());
  node = new TlmNodeWidget(parentNode, ACTlmEnum::SPOT_TIME);
  node->setText(0, ACTelemetry::GetMnemonic(node->GetTlmType()).c_str());
  TelemetryTree->addTopLevelItem(parentNode);
}

/**
 * Function: ~TelemetryMgrWidget
 * Notes: See header file
 */
TelemetryMgrWidget::~TelemetryMgrWidget()
{

}

/**
 * Function: PrintStatusSlot
 * Notes: See header file
 */
void TelemetryMgrWidget::PrintStatusSlot(char* buf)
{
  std::string status = (buf);
  delete[] buf;
  StatusBox->append(status.c_str());
}

/**
 * Function: StartTlmServerClicked
 * Notes: See header file
 */
void TelemetryMgrWidget::StartTlmServerClicked()
{
  if (TelemetryManager::SERVER_ACTIVE != TelemetryManager::Instance()->GetStatus())
  { 
    std::string profile = TlmProfileComboBox->currentText().toStdString().c_str();
    TelemetryManager::Instance()->SetActiveTelemetry(profile);

    std::string exe = ProcessComboBox->currentText().toStdString();
    TelemetryManager::Instance()->SetTargetExecutable(exe);

    if (false == TelemetryManager::Instance()->StartTelemetry())
    {
      QMessageBox::critical(this, 
                            "Sim Racing Utilities - ERROR", 
                            "Failed starting telemetry server.",
                            QMessageBox::Ok);
      return;
    }

    ProcessComboBox->setEnabled(false);
    TlmProfileComboBox->setEnabled(false);
    StartTlmButton->setIcon(QIcon(":/SimRU/resources/stop.png"));
    StartTlmButton->setText("Deactivate");
  }
  else
  {
    TelemetryManager::Instance()->StopTelemetry();
    StartTlmButton->setIcon(QIcon(":/SimRU/resources/run.png"));
    StartTlmButton->setText("Activate");
    ProcessComboBox->setEnabled(true);
    TlmProfileComboBox->setEnabled(true);
  }
}

/**
 * Function: TlmNodeItemDoubleClicked
 * Notes: See header file
 */
void TelemetryMgrWidget::TlmNodeItemDoubleClicked(QTreeWidgetItem* item, int column)
{
  std::string originalStr = item->text(1).toStdString();
  QString newStr = QInputDialog::getText(this, 
                                         "Sim Racing Utilities - Override Telemetry",
                                         "Override telemetry value:",
                                         QLineEdit::Normal,
                                         originalStr.c_str());
  if (0 != newStr.length())
  {
    TlmNodeWidget* node = static_cast<TlmNodeWidget*>(item);
    item->setText(1, newStr);

    TelemetryManager::Instance()->SetTelemetryFunction(TlmProfileComboBox->currentText().toStdString(),
                                                       node->GetTlmType(), 
                                                       newStr.toStdString());
  }
}

/**
 * Function: SaveProfileClicked
 * Notes: See header file
 */
void TelemetryMgrWidget::SaveProfileClicked()
{
  if (0 == ProcessComboBox->currentText().length())
  {
    QMessageBox::critical(this,
                          "SimRacingUtilities",
                          "A target process must be selected.");
    return;
  }

  QString newStr = QInputDialog::getText(this, 
                                         "Sim Racing Utilities - Save Telemetry Profile",
                                         "Profile Name:",
                                         QLineEdit::Normal,
                                         TlmProfileComboBox->currentText());
  if (0 != newStr.length())
  {
    size_t index = TlmProfileComboBox->findText(newStr);
    if (-1 == index)
    {
      TlmProfileComboBox->addItem(newStr);
      index = TlmProfileComboBox->count();
    }

    TlmProfileComboBox->blockSignals(true);
    TlmProfileComboBox->setCurrentIndex(index);
    TlmProfileComboBox->setCurrentText(newStr);
    TlmProfileComboBox->blockSignals(false);

    std::string profile = newStr.toStdString();
    TelemetryManager::Instance()->SetActiveTelemetry(profile);

    size_t topLevelNodes = TelemetryTree->topLevelItemCount();
    for (size_t i = 0; i < topLevelNodes; ++i)
    {
      TlmNodeWidget* topNode = static_cast<TlmNodeWidget*>(TelemetryTree->topLevelItem(i));
      size_t childNodes = topNode->childCount();
      for (size_t j = 0; j < childNodes; ++j)
      {
        TlmNodeWidget* node = static_cast<TlmNodeWidget*>(topNode->child(j));
        if (0 == node->text(1).length()) continue;

        TelemetryManager::Instance()->SetTelemetry(profile.c_str(),
                                                   node->GetTlmType(),
                                                   node->text(1).toStdString());
      }
    }

    std::string exe = ProcessComboBox->currentText().toStdString();
    TelemetryManager::Instance()->SetTargetExecutable(exe);
    
    SRU_Main::GetInstance()->ExportSettings();
  }
}

/**
 * Function: DeleteProfileClicked
 * Notes: See header file
 */
void TelemetryMgrWidget::DeleteProfileClicked()
{
  if (1 >= TlmProfileComboBox->count())
  {
    QMessageBox::critical(this,
                          "SimRacingUtilities",
                          "There must be atleast one profile remaining after deletion");
    return;
  }

  if (QMessageBox::Yes != 
      QMessageBox::information(this, 
                               "Sim Racing Utilities",
                               "Delete profile: " + TlmProfileComboBox->currentText() + " ?",
                               QMessageBox::Yes,
                               QMessageBox::Cancel))
  {
    return;
  }

  TelemetryManager::Instance()->DeleteProfile(TlmProfileComboBox->currentText().toStdString().c_str());  
  SRU_Main::GetInstance()->ExportSettings();
  ImportTelemetrySettings();
  UpdateTelemetryTree();
}

/**
 * Function: TlmProfileChanged
 * Notes: See header file
 */
void TelemetryMgrWidget::TlmProfileChanged(int index)
{
  TelemetryManager* tlmMgr = TelemetryManager::Instance();

  std::string profile  = TlmProfileComboBox->currentText().toStdString().c_str();
  tlmMgr->SetActiveTelemetry(profile);

  // Load telemetry values
  size_t topLevelNodes = TelemetryTree->topLevelItemCount();
  for (size_t i = 0; i < topLevelNodes; ++i)
  {
    TlmNodeWidget* topNode = static_cast<TlmNodeWidget*>(TelemetryTree->topLevelItem(i));
    size_t childNodes = topNode->childCount();
    for (size_t j = 0; j < childNodes; ++j)
    {
      TlmNodeWidget* node = static_cast<TlmNodeWidget*>(topNode->child(j));
      node->setText(1, tlmMgr->GetTelemetry(profile.c_str(), node->GetTlmType()).c_str());
    }
  }
}

/**
 * Function: TlmProcessChanged
 * Notes: See header file
 */
void TelemetryMgrWidget::TlmProcessChanged(int index)
{
  std::string profile = TlmProfileComboBox->currentText().toStdString().c_str();
  TelemetryManager::Instance()->SetActiveTelemetry(profile);

  std::string exe = ProcessComboBox->currentText().toStdString();
  TelemetryManager::Instance()->SetTargetExecutable(exe);
}

/**
 * Function: StopTelemetryServer
 * Notes: See header file
 */
void TelemetryMgrWidget::StopTelemetryServer()
{
  if (true == TelemetryManager::Instance()->IsTelemetryServerRunning())
  {
    StartTlmButton->click();
  }
}