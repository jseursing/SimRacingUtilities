#include "FramelessInterface.h"
#include <QLayout>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QTimer>
#include <Windows.h>

/**
 * Function:  SetTitle
 * Notes: See header file
 */
void FramelessInterface::SetTitle(const char* title)
{
  TaskLabel->setText(title);
}

/**
 * Function: SetBackgroundImage
 * Notes: See header file
 */
void FramelessInterface::SetBackgroundImage(const char* path)
{
  QPixmap backgroundPx(path);
  backgroundPx = backgroundPx.scaled(QSize(width(), height()), Qt::IgnoreAspectRatio);

  QPalette palette;
  palette.setBrush(QPalette::Window, backgroundPx);
  setPalette(palette);

  // Rounded Corners
  HWND hWnd = reinterpret_cast<HWND>(winId());
  HRGN hRegion = CreateRoundRectRgn(0, 0, width(), height(), 11, 11);
  SetWindowRgn(hWnd, hRegion, TRUE);
}

/**
 * Function: SetMinimizeIcon
 * Notes: See header file
 */
void FramelessInterface::SetMinimizeIcon(const char* path)
{
  SetButtonIcon(FramelessInterface::TB_MIN, path);
}

/**
 * Function: SetMaximizeIcon
 * Notes: See header file
 */
void FramelessInterface::SetMaximizeIcon(const char* path)
{
  SetButtonIcon(FramelessInterface::TB_MAX, path);
}

/**
 * Function: SetRestoreIcon
 * Notes: See header file
 */
void FramelessInterface::SetRestoreIcon(const char* path)
{
  SetButtonIcon(FramelessInterface::TB_RESTORE, path);
}

/**
 * Function: SetCloseIcon
 * Notes: See header file
 */
void FramelessInterface::SetCloseIcon(const char* path)
{
  SetButtonIcon(FramelessInterface::TB_CLOSE, path);
}

/**
 * Function: GetTaskButton
 * Notes: See header file
 */
QPushButton* FramelessInterface::GetTaskButton(TaskButtonEnum tbType)
{
  if (tbType < NUM_TB_BTNS)
  {
    return TaskButtons[tbType];
  }

  return nullptr;
}

/**
 * Function: InitializeMainInterface
 * Notes: See header file
 */
void FramelessInterface::InitializeMainInterface(QMainWindow* mainInterface, 
                                                 const char* iconPath, 
                                                 std::vector<QAction*> trayActions)
{
  VLayout->addWidget(mainInterface);

  QWidget* centralWidget = new QWidget(this);
  centralWidget->setObjectName("CentralWidget");
  centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  centralWidget->setLayout(VLayout);
  centralWidget->setMouseTracking(true);
  setCentralWidget(centralWidget);

  HWND hWnd = reinterpret_cast<HWND>(winId());
  HRGN hRegion = CreateRoundRectRgn(0, 0, width(), height(), 11, 11);
  SetWindowRgn(hWnd, hRegion, TRUE);

  if (true == OverwriteClose)
  {
    QPixmap trayIcon(iconPath);
    QIcon qIcon(trayIcon);
    TrayIcon = new QSystemTrayIcon(qIcon);
    TrayIcon->setVisible(false);

    TrayMenu = new QMenu();
    for (size_t i = 0; i < trayActions.size(); ++i)
    {
      TrayMenu->addAction(trayActions[i]);
    }

    QAction* showAction = new QAction("Show", this);
    connect(showAction, &QAction::triggered, this,
            [this]() {
              TrayIcon->hide();
              slotRestored();
            });
    TrayMenu->addAction(showAction);

    QAction* exitAction = new QAction("Exit", this);
    connect(exitAction, &QAction::triggered, this, 
            [this]() {
              OverwriteClose = false;
              slotClose();
            });
    TrayMenu->addSeparator();
    TrayMenu->addAction(exitAction);
    TrayIcon->setContextMenu(TrayMenu);
  }

  AnimationTimer = new QTimer(this);
  connect(AnimationTimer, &QTimer::timeout, this, &FramelessInterface::slotAnimate);
  AnimationTimer->start(100);
}

/**
 * Function: FramelessInterface
 * Notes: See header file
 */
FramelessInterface::FramelessInterface(uint8_t taskBarFlags, bool overwriteClose, QWidget* parent) :
  QMainWindow(parent, Qt::CustomizeWindowHint),
  TaskBar(nullptr),
  TaskLabel(nullptr),
  TaskButtons{nullptr, nullptr, nullptr, nullptr},
  Maximized(false),
  Moving(false),
  OverwriteClose(overwriteClose)
{
  memset(AnimationAngles, 0, sizeof(AnimationAngles));
  memset(EnableAnimation, false, sizeof(EnableAnimation));
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

  VLayout = new QVBoxLayout();
  VLayout->setSpacing(0);
  VLayout->setContentsMargins(0, 0, 0, 0);

  QHBoxLayout* horizontalLayout = new QHBoxLayout();
  horizontalLayout->setSpacing(0);
  horizontalLayout->setContentsMargins(3, 3, 3, 3);

  TaskBar = new QWidget(this);
  TaskBar->setMinimumHeight(30);
  TaskBar->setObjectName("TaskBarWidget");
  TaskBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  TaskBar->setLayout(horizontalLayout);

  TaskLabel = new QLabel(TaskBar);
  TaskLabel->setObjectName("TaskLabelWidget");

  QFont font = TaskLabel->font();
  font.setPixelSize(16);
  font.setBold(true);
  TaskLabel->setFont(font);

  QPalette palette = TaskLabel->palette();
  palette.setColor(this->backgroundRole(), Qt::white);
  palette.setColor(this->foregroundRole(), Qt::white);
  TaskLabel->setPalette(palette);

  horizontalLayout->addWidget(TaskLabel);
  horizontalLayout->addStretch(1);

  if (taskBarFlags | TBF_MIN)
  {
    TaskButtons[TB_MIN] = new QPushButton(TaskBar);
    TaskButtons[TB_MIN]->setFlat(true);
    TaskButtons[TB_MIN]->setObjectName("MinimizeBtn");
    TaskButtons[TB_MIN]->installEventFilter(this);
    horizontalLayout->addWidget(TaskButtons[TB_MIN]);
    connect(TaskButtons[TB_MIN], SIGNAL(clicked()), this, SLOT(slotMinimized()));
  }

  if (taskBarFlags | TBF_MAX)
  {
    TaskButtons[TB_RESTORE] = new QPushButton(TaskBar);
    TaskButtons[TB_RESTORE]->setObjectName("RestoreBtnWidget");
    TaskButtons[TB_RESTORE]->setFlat(true);
    TaskButtons[TB_RESTORE]->setVisible(false);
    TaskButtons[TB_RESTORE]->installEventFilter(this);
    horizontalLayout->addWidget(TaskButtons[TB_RESTORE]);
    connect(TaskButtons[TB_RESTORE], SIGNAL(clicked()), this, SLOT(slotRestored()));

    TaskButtons[TB_MAX] = new QPushButton(TaskBar);
    TaskButtons[TB_MAX]->setObjectName("MaximizeBtnWidget");
    TaskButtons[TB_MAX]->setFlat(true);
    TaskButtons[TB_MAX]->installEventFilter(this);
    horizontalLayout->addWidget(TaskButtons[TB_MAX]);
    connect(TaskButtons[TB_MAX], SIGNAL(clicked()), this, SLOT(slotMaximized()));
  }

  if (taskBarFlags | TBF_CLOSE)
  {
    TaskButtons[TB_CLOSE] = new QPushButton(TaskBar);
    TaskButtons[TB_CLOSE]->setObjectName("CloseBtnWidget");
    TaskButtons[TB_CLOSE]->setFlat(true);
    TaskButtons[TB_CLOSE]->installEventFilter(this);
    horizontalLayout->addWidget(TaskButtons[TB_CLOSE]);
    connect(TaskButtons[TB_CLOSE], SIGNAL(clicked()), this, SLOT(slotClose()));
  }

  VLayout->addWidget(TaskBar);
}

/**
 * Function: mouseMoveEvent
 * Notes: See header file
 */
void FramelessInterface::mouseMoveEvent(QMouseEvent* event)
{
  if (true == Moving)
  {
    move(WindowPosition + (event->globalPos() - LastMousePosition));
  }
}

/**
 * Function: mousePressEvent
 * Notes: See header file
 */
void FramelessInterface::mousePressEvent(QMouseEvent* event)
{
  bool taskBarSelected = TaskBar->underMouse() || TaskLabel->underMouse();
  if (true == taskBarSelected)
  {
    if (Qt::LeftButton == event->button())
    {
      LastMousePosition = event->globalPos();
      WindowPosition = pos();
      Moving = true;
    }
  }
}

/**
 * Function: mouseReleaseEvent
 * Notes: See header file
 */
void FramelessInterface::mouseReleaseEvent(QMouseEvent* event)
{
  if (Qt::LeftButton == event->button())
  {
    Moving = false;
  }
}

/**
 * Function: mouseDoubleClickEvent
 * Notes: See header file
 */
void FramelessInterface::mouseDoubleClickEvent(QMouseEvent* event)
{
  if ((false == TaskBar->underMouse()) &&
      (false == TaskLabel->underMouse()))
  {
    return;
  }

  Maximized = !Maximized;
  switch (Maximized)
  {
  case false:
    slotRestored();
    break;
  case true:
    slotMaximized();
    break;
  }
}

/**
 * Function: eventFilter
 * Notes: See header file
 */
bool FramelessInterface::eventFilter(QObject* obj, QEvent* event)
{
  for (size_t i = 0; i < NUM_TB_BTNS; ++i)
  {
    if (reinterpret_cast<QPushButton*>(obj) == TaskButtons[i])
    {
      switch (event->type())
      {
      case QEvent::Enter:
        EnableAnimation[i] = true;
        AnimationAngles[i] = 0;
        break;
      case QEvent::Leave:
      {
        EnableAnimation[i] = false;
        TaskButtons[i]->setIcon(QIcon(ButtonPixmaps[i]));
      } 
        break;
      default:
        return false;
      }

      break;
    }
  }

  return true;
}

/**
 * Function: slotMinimized
 * Notes: See header file
 */
void FramelessInterface::slotMinimized()
{
  if (nullptr != TaskButtons[TB_MIN])
  {
    setWindowState(Qt::WindowMinimized);
  }
}

/**
 * Function: slotMaximized
 * Notes: See header file
 */
void FramelessInterface::slotMaximized()
{
  if (nullptr != TaskButtons[TB_MAX])
  {
    TaskButtons[TB_RESTORE]->setVisible(true);
    TaskButtons[TB_MAX]->setVisible(false);
    setWindowState(Qt::WindowMaximized);
  }
}

/**
 * Function: slotRestored
 * Notes: See header file
 */
void FramelessInterface::slotRestored()
{
  if (nullptr != TaskButtons[TB_MAX])
  {
    TaskButtons[TB_RESTORE]->setVisible(false);
    TaskButtons[TB_MAX]->setVisible(true);
    setWindowState(Qt::WindowNoState);
  }
}

/**
 * Function: slotClose
 * Notes: See header file
 */
void FramelessInterface::slotClose()
{
  if (TaskButtons[TB_CLOSE] != sender())
  {
    close();
  }

  if (nullptr != TaskButtons[TB_CLOSE])
  {
    if (false == OverwriteClose)
    {
      close();
    }
    else
    {
      if (QMessageBox::No ==
          QMessageBox::question(this, 
                                TaskLabel->text(), 
                                "Minimize to tray instead?",
                                QMessageBox::Yes, 
                                QMessageBox::No))
      {
        close();
      }
      else
      {
        if (true == OverwriteClose)
        {
          TrayIcon->setVisible(true);
        }
    
        slotMinimized();
      }
    }
  }
}

/**
 * Function: slotAnimate
 * Notes: See header file
 */
void FramelessInterface::slotAnimate()
{
  for (size_t i = 0; i < NUM_TB_BTNS; ++i)
  {
    if (true == EnableAnimation[i])
    {
      AnimationAngles[i] = (AnimationAngles[i] + 10) % 360;
      
      QPixmap p(ButtonPixmaps[i]);
      QPixmap p2 = p.transformed(QTransform().rotate(AnimationAngles[i]));
      TaskButtons[i]->setIcon(QIcon(p2));
      break;
    }
  }
}

/**
 * Function: SetButtonIcon
 * Notes: See header file
 */
void FramelessInterface::SetButtonIcon(TaskButtonEnum tbType, const char* path)
{
  assert(tbType < NUM_TB_BTNS);
  if (nullptr != TaskButtons[tbType])
  {
    ButtonPixmaps[tbType] = QPixmap::fromImage(QImage(path));
    TaskButtons[tbType]->setIcon(QIcon(ButtonPixmaps[tbType]));
  }
}