#include "AppEntryWidget.h"
#include "GlobalConst.h"
#include "SRU_Main.h"
#include <QCheckBox>
#include <QLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QToolButton>

/**
 * Function: AddButtonClicked
 * Notes: See header file
 */
void AppEntryWidget::AddButtonClicked()
{
  emit AddAppSignal();
}

/**
 * Function: Expand
 * Notes: See header file
 */
void AppEntryWidget::Expand()
{
  setFixedSize(QSize(BaseWidth, BaseHeight * 3));

  QPixmap pixmap(QSize(BaseWidth, BaseHeight * 3));
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
  painter.setBrush(QBrush(GlobalConst::BlueGray, Qt::SolidPattern));

  QPainterPath path;
  path.addRoundedRect(QRectF(5, 0, BaseWidth - 5, (BaseHeight * 3) - 5), 10, 10);
  painter.fillPath(path, painter.brush());
  setPixmap(pixmap);

  PosLabel->setVisible(true);
  PosXEdit->setVisible(true);
  PosYEdit->setVisible(true);
  DimLabel->setVisible(true);
  DimXEdit->setVisible(true);
  DimYEdit->setVisible(true);
  ResizeCBox->setVisible(true);
  RemoveCBox->setVisible(true);
  AddButton->setVisible(true);
  IsCollapsed = false;
}

/**
 * Function: Collapse
 * Notes: See header file
 */
void AppEntryWidget::Collapse()
{
  setFixedSize(QSize(BaseWidth, BaseHeight));
  
  QPixmap pixmap(QSize(BaseWidth, BaseHeight));
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
  painter.setBrush(QBrush(GlobalConst::BlueGray, Qt::SolidPattern));

  QPainterPath path;
  path.addRoundedRect(QRectF(5, 0, BaseWidth - 5, BaseHeight), 10, 10);
  painter.fillPath(path, painter.brush());
  setPixmap(pixmap);

  PosLabel->setVisible(false);
  PosXEdit->setVisible(false);
  PosYEdit->setVisible(false);
  DimLabel->setVisible(false);
  DimXEdit->setVisible(false);
  DimYEdit->setVisible(false);
  ResizeCBox->setVisible(false);
  RemoveCBox->setVisible(false);
  AddButton->setVisible(false);
  IsCollapsed = true;
}

/**
 * Function: AppEntryWidget
 * Notes: See header file
 */
AppEntryWidget::AppEntryWidget(uint32_t widgetW,
                               uint32_t widgetH,
                               const char* windowStr,
                               const char* exeStr,
                               QWidget* parent) :
  QLabel(parent),
  IsCollapsed(true),
  BaseWidth(widgetW),
  BaseHeight(widgetH),
  Filename(exeStr),
  WindowTitle(windowStr)
{
  AppLayout = new QGridLayout(this);
  AppLayout->setContentsMargins({15, 5, 10, 10});

  QFont font = this->font();
  font.setFamily("Lucida Console");
  font.setPointSize(8);
  font.setBold(true);

  QLabel* appLabel = new QLabel(this);
  appLabel->setText(exeStr);
  appLabel->setFont(font);
  appLabel->setStyleSheet("background-color: transparent; color: white");
  AppLayout->addWidget(appLabel, 0, 0, 1, 3);

  QLabel* wndLabel = new QLabel(this);
  std::string window_str = windowStr;
  if (30 < window_str.length()) window_str = window_str.substr(0, 30);
  wndLabel->setText(window_str.c_str());
  wndLabel->setFont(font);
  wndLabel->setStyleSheet("background-color: transparent; color: white");
  AppLayout->addWidget(wndLabel, 1, 0, 1, 4);

  font.setPointSize(7);
  PosLabel = new QLabel(this);
  PosLabel->setText("Pos:");
  PosLabel->setFont(font);
  PosLabel->setStyleSheet("background-color: transparent; color: white");
  PosLabel->hide();
  AppLayout->addWidget(PosLabel, 2, 0, 1, 1);

  PosXEdit = new QLineEdit(this);
  PosXEdit->setFont(font);
  PosXEdit->setFixedWidth(50);
  PosXEdit->setFixedHeight(20);
  PosXEdit->setStyleSheet("border-style: solid; border-width: 1px; border-color: white");
  QPalette palette = PosXEdit->palette();
  palette.setColor(PosXEdit->backgroundRole(), Qt::white);
  palette.setColor(PosXEdit->foregroundRole(), Qt::white);
  PosXEdit->setPalette(palette);
  PosXEdit->hide();
  AppLayout->addWidget(PosXEdit, 2, 1, 1, 1, Qt::AlignTop);

  PosYEdit = new QLineEdit(this);
  PosYEdit->setFont(font);
  PosYEdit->setFixedWidth(50);
  PosYEdit->setFixedHeight(20);
  PosYEdit->setStyleSheet("border-style: solid; border-width: 1px; border-color: white");
  PosYEdit->setPalette(palette);
  PosYEdit->hide();
  AppLayout->addWidget(PosYEdit, 2, 2, 1, 1, Qt::AlignTop);

  DimLabel = new QLabel(this);
  DimLabel->setText("Size:");
  DimLabel->setFont(font);
  DimLabel->setStyleSheet("background-color: transparent; color: white");
  DimLabel->hide();
  AppLayout->addWidget(DimLabel, 3, 0, 1, 1);
  DimXEdit = new QLineEdit(this);
  DimXEdit->setFont(font);
  palette = DimXEdit->palette();
  palette.setColor(DimXEdit->backgroundRole(), Qt::white);
  palette.setColor(DimXEdit->foregroundRole(), Qt::white);
  DimXEdit->setFixedWidth(50);
  DimXEdit->setFixedHeight(20);
  DimXEdit->setStyleSheet("border-style: solid; border-width: 1px; border-color: white");
  DimXEdit->setPalette(palette);
  DimXEdit->hide();
  AppLayout->addWidget(DimXEdit, 3, 1, 1, 1, Qt::AlignTop);
  DimYEdit = new QLineEdit(this);
  DimYEdit->setFont(font);
  DimYEdit->setFixedWidth(50);
  DimYEdit->setFixedHeight(20);  
  DimYEdit->setStyleSheet("border-style: solid; border-width: 1px; border-color: white");
  DimYEdit->setPalette(palette);
  DimYEdit->hide();
  AppLayout->addWidget(DimYEdit, 3, 2, 1, 1, Qt::AlignTop);

  ResizeCBox = new QCheckBox(this);
  ResizeCBox->setFont(font);
  ResizeCBox->setStyleSheet("background-color: transparent; color: white");
  ResizeCBox->setText("Auto Resize");
  ResizeCBox->hide();
  AppLayout->addWidget(ResizeCBox, 4, 0, 1, 2, Qt::AlignRight | Qt::AlignTop);

  RemoveCBox = new QCheckBox(this);
  RemoveCBox->setFont(font);
  RemoveCBox->setStyleSheet("background-color: transparent; color: white");
  RemoveCBox->setText("Remove Frame");
  RemoveCBox->hide();
  AppLayout->addWidget(RemoveCBox, 4, 2, 1, 3, Qt::AlignTop);

  AddButton = new QToolButton(this);
  AddButton->setFixedSize(QSize(20, 20));
  AddButton->setIcon(QIcon(":/SimRU/resources/add.png"));
  AddButton->hide();
  //  AddWndButton->setStyleSheet("border: 1px solid white;");
  connect(AddButton, &QToolButton::clicked, this, &AppEntryWidget::AddButtonClicked);
  AppLayout->addWidget(AddButton, 0, 4, 1, 1, Qt::AlignHCenter | Qt::AlignTop);

  Collapse();
}

/**
 * Function: ~AppEntryWidget
 * Notes: See header file
 */
AppEntryWidget::~AppEntryWidget()
{

}
