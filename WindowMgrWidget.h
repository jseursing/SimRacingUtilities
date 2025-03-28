#pragma once
#include <QWidget>
#include <vector>

class QLabel;
class QPixmap;
class QToolButton;
class QTableWidget;

/**
  * Class: WindowMgrWidget
  * Brief: This widget represents the window manager tab.
  */
class WindowMgrWidget : public QWidget
{
  Q_OBJECT

public:
  void RefreshWindows();
  void ImportWindowSettings();
  WindowMgrWidget(QWidget* parent = nullptr);
  ~WindowMgrWidget();


public slots:
  void AddApplicationClicked();
  void ProcessAppSlot(int row, int col);
  void ProcessEntrySlot(int row, int col);
  bool eventFilter(QObject* obj, QEvent* event);

private:

  QTableWidget* WindowTable;
  QTableWidget* ApplicationTable;
  QLabel* MonitorDisplay;
  QWidget* WindowContainer;
  QToolButton* RefreshButton;
  QPixmap* BaseMonPixmap;
  std::vector<QRect>  MonDrawPos;
  std::vector<bool>   MonSelected;
  static const size_t APP_DRAW_HEIGHT = 95;
  static const size_t WND_DRAW_HEIGHT = 40;
};

