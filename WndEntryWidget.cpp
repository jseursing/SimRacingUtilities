#include "GlobalConst.h"
#include "WindowManager.h"
#include "WndEntryWidget.h"
#include <QColor>
#include <QPainter>
#include <QPainterPath>

/**
 * Function: WndEntryWidget
 * Notes: See header file
 */
WndEntryWidget::WndEntryWidget(uint32_t widgetW, 
                               uint32_t widgetH,
                               const char* exeStr, 
                               int32_t x, 
                               int32_t y, 
                               uint32_t w, 
                               uint32_t h, 
                               bool reSize, 
                               bool stripBorders,
                               QWidget* parent) :
  QLabel(parent),
  Filename(exeStr)
{
  setFixedSize(QSize(widgetW, widgetH));
  setToolTip("Double-Click to apply\n"
             "Shift + Double-Click to delete");
  
  QPixmap pixmap(QSize(widgetW, widgetH));
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
  painter.setBrush(QBrush(GlobalConst::BlueGray, Qt::SolidPattern));

  QPainterPath path;
  path.addRoundedRect(QRectF(5, 0, widgetW - 5, widgetH), 10, 10);
  painter.fillPath(path, painter.brush());

  painter.setPen(QPen(Qt::white, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

  QFont appFont("Lucida Console", 11);
  appFont.setBold(true);
  painter.setFont(appFont);

  QString str = "App: " + QString(exeStr);
  painter.drawText(QPoint(20, 20), str);
  appFont.setPointSize(8);
  painter.setFont(appFont);
  str = "Position: (" + QString::number(x) + ", " + QString::number(y) + ")";
  painter.drawText(QPoint(20, 35), str);
  str = "Size: (" + QString::number(w) + ", " + QString::number(h) + ")";
  painter.drawText(QPoint(20, 50), str);
  str = "Auto Resize: " + QString(true == reSize ? "YES" : "NO");
  painter.drawText(QPoint(20, 65), str);
  str = "Remove Borders & Title: " + QString(true == stripBorders ? "YES" : "NO");
  painter.drawText(QPoint(20, 80), str);

  setPixmap(pixmap);
}

/**
 * Function: ~WndEntryWidget
 * Notes: See header file
 */
WndEntryWidget::~WndEntryWidget()
{

}