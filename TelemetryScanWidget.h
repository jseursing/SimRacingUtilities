#pragma once
#include <mutex>
#include <QEvent>
#include <QWidget>

class MemUtils;
class QComboBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QToolButton;
class QTableWidget;
class QTextEdit;
class QTreeWidget;
class Win32Sem;

/**
  * Class: TelemetryScanWidget
  * Brief: This widget represents the Memory Utilities tab.
  */
class TelemetryScanWidget : public QWidget
{
  Q_OBJECT

public:
  TelemetryScanWidget(QWidget* parent = nullptr);
  ~TelemetryScanWidget();

signals:
  void ScanCompleteSignal(int progress);
  void PointerScanCompleteSignal(int progress);
  void StopTelemetryServerSignal();

public slots:
  void PrintStatusSlot(char* buf);
  void RefreshTargetsClicked();
  void AttachToTargetClicked();
  void ValueTypeIndexChanged(int index);
  void ModifierIndexChanged(int index);
  void InitialScanClicked();
  void NextScanClicked();
  void ScanCompleteSlot(int progress);
  void PointerScanCompleteSlot(int progress);
  void OnCommandReturnPressed();
  void ResultTimerElapsed();

private:
  static TelemetryScanWidget* Instance();
  static void ScanComplete(int progress);
  static void PointerScanComplete(int progress);

  static        TelemetryScanWidget* ThisInstance;
  QComboBox*    TargetComboBox;
  QToolButton*  RefreshButton;
  QToolButton*  AttachButton;
  QProgressBar* ScanProgress;
  QLabel*       ValueLabel;
  QLineEdit*    ValueEdit;
  QComboBox*    ValueTypeComboBox;
  QComboBox*    ModifierComboBox;
  QToolButton*  InitialScanButton;
  QToolButton*  NextScanButton;
  QTreeWidget*  ModuleTree;
  QTableWidget* ResultTable;
  QTextEdit*    StatusBox;
  QLineEdit*    CommandEdit;
  QTimer*       ResultTimer;
  Win32Sem*     Semaphore;
  MemUtils*     MemoryUtils;
};

