#pragma once
#include <QWidget>

class QComboBox;
class QLabel;
class QToolButton;
class QTextEdit;
class QTreeWidget;
class QTreeWidgetItem;

/**
  * Class: TelemetryMgrWidget
  * Brief: This widget represents the telemetry manager tab.
  */
class TelemetryMgrWidget : public QWidget
{
  Q_OBJECT

public:

  void RefreshApps();
  void UpdateTelemetryTree();
  void ImportTelemetrySettings();
  TelemetryMgrWidget(QWidget* parent = nullptr);
  ~TelemetryMgrWidget();


public slots:
  void PrintStatusSlot(char* buf);
  void StartTlmServerClicked();
  void TlmNodeItemDoubleClicked(QTreeWidgetItem* item, int column);
  void SaveProfileClicked();
  void DeleteProfileClicked();
  void TlmProfileChanged(int index);
  void TlmProcessChanged(int index);
  void StopTelemetryServer();

private:


  QToolButton* TlmRefreshButton;
  QComboBox* TlmProfileComboBox;
  QToolButton* SaveProfileButton;
  QToolButton* DeleteProfileButton;
  QTreeWidget* TelemetryTree;
  QComboBox* ProcessComboBox;
  QLabel* SubscriberLabel;
  QTextEdit* StatusBox;
  QToolButton* StartTlmButton;
};

