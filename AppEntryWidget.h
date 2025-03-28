#pragma once
#include <QMouseEvent>
#include <QLabel>

class QCheckBox;
class QGridLayout;
class QLabel;
class QLineEdit;
class QPushButton;

/**
  * Class: AppEntryWidget
  * Brief: This widget represents an applicable application with visible window
  *        to reposition and/or resize.
  */
class AppEntryWidget : public QLabel
{
  Q_OBJECT

signals:
  void AddAppSignal();

public slots:
  void AddButtonClicked();

public:

  void Expand();
  void Collapse();
  AppEntryWidget(uint32_t widgetW,
                 uint32_t widgetH,
                 const char* windowStr,
                 const char* exeStr,
                 QWidget* parent = nullptr);
  ~AppEntryWidget();

  bool         IsCollapsed;
  uint32_t     BaseWidth;
  uint32_t     BaseHeight;
  std::string  Filename;
  std::string  WindowTitle;
  QLabel*      PosLabel;
  QLineEdit*   PosXEdit;
  QLineEdit*   PosYEdit;
  QLabel*      DimLabel;
  QLineEdit*   DimXEdit;
  QLineEdit*   DimYEdit;
  QCheckBox*   ResizeCBox;
  QCheckBox*   RemoveCBox;
  QToolButton* AddButton;
  QGridLayout* AppLayout;
};

