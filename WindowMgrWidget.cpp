#include "AppEntryWidget.h"
#include "GlobalConst.h"
#include "SRU_Main.h"
#include "WindowManager.h"
#include "WindowMgrWidget.h"
#include "WndEntryWidget.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMovie>
#include <QPainter>
#include <QPainterPath>
#include <QToolButton>
#include <QTableWidget>

/**
 * Function: RefreshWindows
 * Notes: See header file
 */
void WindowMgrWidget::RefreshWindows()
{
  WindowManager* wMgr = WindowManager::Instance();
  std::vector<WindowManager::AppEntry> windows = wMgr->GetWindowList();

  ApplicationTable->setRowCount(0);
  ApplicationTable->setRowCount(windows.size() + 1);

  for (size_t i = 0; i < windows.size(); ++i)
  {
    AppEntryWidget* appWidget = new AppEntryWidget(ApplicationTable->width() - 10,
                                                   WND_DRAW_HEIGHT,
                                                   windows[i].WindowTitle.c_str(),
                                                   windows[i].Filename.c_str(),
                                                   ApplicationTable);
    connect(appWidget, &AppEntryWidget::AddAppSignal, this, &WindowMgrWidget::AddApplicationClicked);
    ApplicationTable->setCellWidget(i, 0, appWidget);
  }
 
  // Collect and display monitors
  wMgr->RefreshDisplayDevices();
  std::vector<WindowManager::MonitorEntry> monitors = wMgr->GetMonitorList();
  MonDrawPos.clear();
  MonSelected.resize(monitors.size(), false);

  const size_t spacing = 10;
  size_t drawWidth     = MonitorDisplay->width() - ((1 + monitors.size()) * spacing);
  size_t drawMonWidth  = drawWidth / monitors.size();
  size_t drawMonHeight = MonitorDisplay->height() - 15;

  if (nullptr != BaseMonPixmap)
  {
    delete BaseMonPixmap;
  }

  BaseMonPixmap = new QPixmap(MonitorDisplay->width(), MonitorDisplay->height());
  BaseMonPixmap->fill(Qt::transparent);

  QPainter painter(BaseMonPixmap);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

  for (size_t i = 0; i < monitors.size(); ++i)
  {
    int32_t x = spacing + (i * drawMonWidth) + (i * spacing);
    int32_t y = spacing;

    QPainterPath path;
    MonDrawPos.push_back(QRect(x, y, drawMonWidth, drawMonHeight));
    path.addRoundedRect(QRectF(x, y, drawMonWidth, drawMonHeight), 10, 10);
    painter.setPen(QPen(GlobalConst::BlueGray, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QBrush brush(GlobalConst::BlueGray);
    painter.fillPath(path, brush);

    QFont appFont("Lucida Console", 8);
    appFont.setBold(true);
    painter.setFont(appFont);
    painter.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    painter.drawText(QPoint(x + 10, y + 25), monitors[i].MonitorName.c_str());
    QString str = "Pos: " + QString::number(monitors[i].X) + ", " + QString::number(monitors[i].Y);
    painter.drawText(QPoint(x + 10, y + 45), str);
    str = "Dim: " + QString::number(monitors[i].Width) + ", " + QString::number(monitors[i].Height);
    painter.drawText(QPoint(x + 10, y + 65), str);
  }
  
  MonitorDisplay->setPixmap(*BaseMonPixmap);
}

/**
 * Function: ImportWindowSettings
 * Notes: See header file
 */
void WindowMgrWidget::ImportWindowSettings()
{
  WindowTable->setRowCount(0);

  std::map<std::string, WindowManager::ResizeEntry>::const_iterator begin;
  std::map<std::string, WindowManager::ResizeEntry>::const_iterator end;
  WindowManager::Instance()->GetResizeIterators(begin, end);
  std::map<std::string, WindowManager::ResizeEntry>::const_iterator itr = begin;

  for (size_t row = 0; itr != end; ++itr, ++row)
  {
    WindowTable->setRowCount(row + 1);
    WindowTable->setCellWidget(row, 0, new WndEntryWidget(WindowTable->width() - 10,
                                                          APP_DRAW_HEIGHT,
                                                          itr->first.c_str(),
                                                          itr->second.X,
                                                          itr->second.Y,
                                                          itr->second.W,
                                                          itr->second.H,
                                                          itr->second.Resize,
                                                          itr->second.StripBorders,
                                                          WindowTable));
  }

  RefreshButton->setFixedWidth(100);
}

/**
 * Function: WindowMgrWidget
 * Notes: See header file
 */
WindowMgrWidget::WindowMgrWidget(QWidget* parent) :
  QWidget(parent)
{
  // This page will be split in upper and lower halves.
  // The upper half will display monitor configuration while the bottom
  // displays window management-by-process.
  QVBoxLayout* vLayout = new QVBoxLayout(this);
  setLayout(vLayout);
  vLayout->setSpacing(1);

  MonitorDisplay = new QLabel(this);
  MonitorDisplay->setMouseTracking(true);
  BaseMonPixmap  = nullptr;
  MonitorDisplay->setFixedWidth(GlobalConst::FIXED_WIDTH - 30);
  MonitorDisplay->setFixedHeight(100);
  MonitorDisplay->installEventFilter(this);
  vLayout->addWidget(MonitorDisplay);

  WindowContainer = new QWidget(this);
  vLayout->addWidget(WindowContainer);
  
  QGridLayout* appLayout = new QGridLayout(WindowContainer);
  appLayout->setSpacing(5);
  appLayout->setVerticalSpacing(5);
  WindowContainer->setLayout(appLayout);

  QFont font = this->font();
  font.setBold(true);

  ApplicationTable = new QTableWidget();
  ApplicationTable->setFont(font);
  ApplicationTable->setColumnCount(1);
  ApplicationTable->setStyleSheet("QTableWidget { border: none; }");
  ApplicationTable->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
  ApplicationTable->horizontalHeader()->setVisible(false);
  ApplicationTable->horizontalHeader()->setDefaultSectionSize(ApplicationTable->width() - 20);
  ApplicationTable->setVerticalHeaderItem(0, new QTableWidgetItem(""));
  ApplicationTable->verticalHeader()->setVisible(false);
  ApplicationTable->verticalHeader()->setDefaultSectionSize(WND_DRAW_HEIGHT + 3);
  ApplicationTable->setShowGrid(false);
  ApplicationTable->setSelectionMode(QAbstractItemView::NoSelection);
  ApplicationTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ApplicationTable->setFocusPolicy(Qt::NoFocus);
  ApplicationTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  connect(ApplicationTable, &QTableWidget::cellClicked, this, &WindowMgrWidget::ProcessAppSlot);
  appLayout->addWidget(ApplicationTable, 0, 0, 1, 1);

  RefreshButton = new QToolButton(this);
  RefreshButton->installEventFilter(this);
  RefreshButton->setText("Refresh");
  RefreshButton->setIcon(QIcon(":/SimRU/resources/refresh.png"));
  RefreshButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  RefreshButton->setLayoutDirection(Qt::RightToLeft);
  connect(RefreshButton, &QToolButton::clicked, SRU_Main::GetInstance(), &SRU_Main::RefreshProcsClicked);
  appLayout->addWidget(RefreshButton, 1, 0, 1, 1, Qt::AlignHCenter);

  WindowTable = new QTableWidget();
  WindowTable->setFont(font);
  WindowTable->setColumnCount(1);
  WindowTable->setStyleSheet("border: none;");
  WindowTable->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
  WindowTable->horizontalHeader()->setVisible(false);
  WindowTable->horizontalHeader()->setDefaultSectionSize(WindowTable->width() - 20);
  WindowTable->setVerticalHeaderItem(0, new QTableWidgetItem(""));
  WindowTable->verticalHeader()->setVisible(false);
  WindowTable->verticalHeader()->setDefaultSectionSize(APP_DRAW_HEIGHT + 5);
  WindowTable->setShowGrid(false);
  WindowTable->setSelectionMode(QAbstractItemView::NoSelection);
  WindowTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  WindowTable->setFocusPolicy(Qt::NoFocus);
  WindowTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  connect(WindowTable, &QTableWidget::cellDoubleClicked, this, &WindowMgrWidget::ProcessEntrySlot);
  appLayout->addWidget(WindowTable, 0, 1, -1, 1);
}

/**
 * Function: ~WindowMgrWidget
 * Notes: See header file
 */
WindowMgrWidget::~WindowMgrWidget()
{

}

/**
 * Function: AddApplicationClicked
 * Notes: See header file
 */
void WindowMgrWidget::AddApplicationClicked()
{
  AppEntryWidget* appEntry = dynamic_cast<AppEntryWidget*>(sender());
  if (nullptr == appEntry)
  {
    return;
  }

  if ((0 == appEntry->PosXEdit->text().length()) ||
      (0 == appEntry->PosYEdit->text().length()) ||
      (0 == appEntry->DimXEdit->text().length()) ||
      (0 == appEntry->DimYEdit->text().length()))
  {
    Beep(100, 10);
    return;
  }

  std::vector<WindowManager::AppEntry> apps = WindowManager::Instance()->GetWindowList();
  std::string filename = appEntry->Filename;
  size_t newLinePos = filename.find('\n');
  if (std::string::npos != newLinePos)
  {
    filename = filename.substr(0, newLinePos);
  }

  // Look for an existing entry first
  int row = WindowTable->rowCount();
  for (size_t i = 0; i < row; ++i)
  {
    WndEntryWidget* w = dynamic_cast<WndEntryWidget*>(WindowTable->cellWidget(i, 0));
    if (0 == w->Filename.compare(filename.c_str()))
    {
      WindowManager::Instance()->RemoveResizeApp(w->Filename);
      WindowTable->removeRow(row);
      row = WindowTable->rowCount() - 1;
      break;
    }
  }

  WindowTable->setRowCount(row + 1);
  WindowTable->setCellWidget(row, 0, new WndEntryWidget(WindowTable->width() - 10,
                                                        APP_DRAW_HEIGHT,
                                                        filename.c_str(),
                                                        appEntry->PosXEdit->text().toInt(),
                                                        appEntry->PosYEdit->text().toInt(),
                                                        appEntry->DimXEdit->text().toUInt(),
                                                        appEntry->DimYEdit->text().toUInt(),
                                                        appEntry->ResizeCBox->isChecked(),
                                                        appEntry->RemoveCBox->isChecked(),
                                                        WindowTable));

  WindowManager::Instance()->AddResizeApp(filename.c_str(),
                                          appEntry->PosXEdit->text().toInt(),
                                          appEntry->PosYEdit->text().toInt(),
                                          appEntry->DimXEdit->text().toUInt(),
                                          appEntry->DimYEdit->text().toUInt(),
                                          appEntry->ResizeCBox->isChecked(),
                                          appEntry->RemoveCBox->isChecked());

  // Save changes
  SRU_Main::GetInstance()->ExportSettings();
}

/**
 * Function: ProcessAppSlot
 * Notes: See header file
 */
void WindowMgrWidget::ProcessAppSlot(int row, int col)
{
  AppEntryWidget* w = dynamic_cast<AppEntryWidget*>(ApplicationTable->cellWidget(row, 0));
  if (nullptr != w)
  {
    for (size_t i = 0; i < ApplicationTable->rowCount(); ++i)
    {
      if (i != row)
      {
        AppEntryWidget* wC = dynamic_cast<AppEntryWidget*>(ApplicationTable->cellWidget(i, 0));
        if (nullptr != wC)
        {
          ApplicationTable->setRowHeight(i, WND_DRAW_HEIGHT + 3);
          wC->Collapse();
        }
      }
    }

    ApplicationTable->setRowHeight(row, WND_DRAW_HEIGHT * 3);
    w->Expand();
  }
}

/**
 * Function: ProcessEntrySlot
 * Notes: See header file
 */
void WindowMgrWidget::ProcessEntrySlot(int row, int col)
{
  WndEntryWidget* w = dynamic_cast<WndEntryWidget*>(WindowTable->cellWidget(row, 0));
  if (nullptr != w)
  {
    if (0 != GetAsyncKeyState(VK_LSHIFT))
    {
      WindowManager::Instance()->RemoveResizeApp(w->Filename);
      WindowTable->removeRow(row);

      // Save changes
      SRU_Main::GetInstance()->ExportSettings();
    }
    else
    {
      WindowManager::Instance()->SynchronizeWindow(w->Filename);
    }
  }
}

/**
 * Function: eventFilter
 * Notes: See header file
 */
bool WindowMgrWidget::eventFilter(QObject* obj, QEvent* event)
{
  if (nullptr == BaseMonPixmap)
  {
    return true;
  }

  // Monitor animations
  if (reinterpret_cast<QLabel*>(obj) == MonitorDisplay)
  {
    switch (event->type())
    {
    case QEvent::MouseMove:
    {
      QMouseEvent* mEvent = static_cast<QMouseEvent*>(event);
      int32_t mX = mEvent->pos().x();
      int32_t mY = mEvent->pos().y();

      QPixmap pixmap(BaseMonPixmap->copy());

      QPainter painter(&pixmap);
      painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

      for (size_t i = 0; i < MonDrawPos.size(); ++i)
      {
        if ((mX >= MonDrawPos[i].left()) &&
            (mX <= MonDrawPos[i].right()))
        {
          if (true == MonSelected[i])
          {
            painter.setPen(QPen(Qt::white, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
          }
          else
          {
            painter.setPen(QPen(GlobalConst::BlueGray, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
          }
        }
        else
        {
          if (true == MonSelected[i])
          {
            painter.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
          }
          else
          {
            painter.setPen(QPen(GlobalConst::BlueGray, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
          }
        }

        QPainterPath path;
        path.addRoundedRect(MonDrawPos[i], 10, 10);
        painter.drawPath(path);
      }

      MonitorDisplay->setPixmap(pixmap);
    }
    break;
    case QEvent::Leave:
    {
      QPixmap pixmap(BaseMonPixmap->copy());

      QPainter painter(&pixmap);
      painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

      for (size_t i = 0; i < MonDrawPos.size(); ++i)
      {
        if (true == MonSelected[i])
        {
          painter.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        }
        else
        {
          painter.setPen(QPen(GlobalConst::BlueGray, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        }

        QPainterPath path;
        path.addRoundedRect(MonDrawPos[i], 10, 10);
        painter.drawPath(path);
      }

      MonitorDisplay->setPixmap(pixmap);
    }
    break;
    case QEvent::MouseButtonRelease:
    {
      std::vector<WindowManager::MonitorEntry> monitors = WindowManager::Instance()->GetMonitorList();

      QMouseEvent* mEvent = static_cast<QMouseEvent*>(event);
      int32_t mX = mEvent->pos().x();
      int32_t mY = mEvent->pos().y();

      QPixmap pixmap(BaseMonPixmap->copy());

      QPainter painter(&pixmap);
      painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

      // Update selection flags
      for (size_t i = 0; i < MonDrawPos.size(); ++i)
      {
        if ((mX >= MonDrawPos[i].left()) &&
            (mX <= MonDrawPos[i].right()))
        {
          MonSelected[i] = !MonSelected[i];
        }
      }

      // Update display and edit boxes
      AppEntryWidget* appWidget = nullptr;
      for (size_t i = 0; i < ApplicationTable->rowCount(); ++i)
      {
        AppEntryWidget* cAppWidget = dynamic_cast<AppEntryWidget*>(ApplicationTable->cellWidget(i, 0));
        if ((nullptr != cAppWidget) &&
            (false == cAppWidget->IsCollapsed))
        {
          appWidget = cAppWidget;
          break;
        }
      }
 
      if (nullptr != appWidget)
      {
        appWidget->PosXEdit->setText("");
        appWidget->PosYEdit->setText("");
        appWidget->DimXEdit->setText("");
        appWidget->DimYEdit->setText("");

        int32_t totalWidth = 0;
        int32_t totalHeight = 0;
        for (size_t i = 0; i < MonDrawPos.size(); ++i)
        {
          QPainterPath path;
          path.addRoundedRect(MonDrawPos[i], 10, 10);
          if (true == MonSelected[i])
          {
            painter.setPen(QPen(Qt::white, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            if (0 == appWidget->PosXEdit->text().length())
            {
              appWidget->PosXEdit->setText(QString::number(monitors[i].X));
              appWidget->PosYEdit->setText(QString::number(monitors[i].Y));
            }

            totalWidth += monitors[i].Width;
            totalHeight = monitors[i].Height;
          }
          else
          {
            painter.setPen(QPen(GlobalConst::BlueGray, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
          }

          painter.drawPath(path);
        }

        appWidget->DimXEdit->setText(QString::number(totalWidth));
        appWidget->DimYEdit->setText(QString::number(totalHeight));
      }

      MonitorDisplay->setPixmap(pixmap);
    }
    break;
    default:
      return false;
    }

    return true;
  }

  return true;
}