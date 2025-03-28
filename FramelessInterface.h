#pragma once
#include <QEvent>
#include <QLabel>
#include <QMainWindow>
#include <vector>

class QMenu;
class QVBoxLayout;
class QPushButton;
class QSystemTrayIcon;
class QTimer;

/**
 * Class: FramelessInterface
 * Brief: Class used to construct a non-standard Qt graphical interface.
 */
class FramelessInterface : public QMainWindow
{
Q_OBJECT

public:
  enum TaskBarFlagEnum
  {
    TBF_MIN = 1,
    TBF_MAX = 2,
    TBF_CLOSE = 4
  };

  enum TaskButtonEnum
  {
    TB_MIN = 0,
    TB_MAX = 1,
    TB_RESTORE = 2,
    TB_CLOSE = 3,
    NUM_TB_BTNS = 4
  };

  FramelessInterface(uint8_t taskBarFlags, bool overwriteClose, QWidget* parent = nullptr);
  void InitializeMainInterface(QMainWindow* mainInterface, 
                               const char* iconPath,
                               std::vector<QAction*> trayActions);

  void SetTitle(const char* title);
  void SetBackgroundImage(const char* path);
  void SetMinimizeIcon(const char* path);
  void SetMaximizeIcon(const char* path);
  void SetRestoreIcon(const char* path);
  void SetCloseIcon(const char* path);
  QPushButton* GetTaskButton(TaskButtonEnum tbType);

protected:
  void mouseMoveEvent(QMouseEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void mouseDoubleClickEvent(QMouseEvent* event);
  bool eventFilter(QObject* obj, QEvent* event);

private slots:
  void slotMinimized();
  void slotMaximized();
  void slotRestored();
  void slotClose();
  void slotAnimate();

private:

  // This is the user defined gui
  QMainWindow*     MainWindow;

  // Tray icon components
  QSystemTrayIcon* TrayIcon;
  QMenu*           TrayMenu;

  // Task bar components
  QWidget*         TaskBar;
  QLabel*          TaskLabel;
  QPushButton*     TaskButtons[NUM_TB_BTNS];
  uint16_t         AnimationAngles[NUM_TB_BTNS];
  bool             EnableAnimation[NUM_TB_BTNS];
  QPixmap          ButtonPixmaps[NUM_TB_BTNS];
  QTimer* AnimationTimer;

  // Misc.
  QPoint           WindowPosition;
  QPoint           LastMousePosition;
  bool             Maximized;
  bool             Moving;
  bool             OverwriteClose;

  // Private functions to be accessed by helper class
  void SetButtonIcon(TaskButtonEnum tbType, const char* path);
  QVBoxLayout*  VLayout;
};

