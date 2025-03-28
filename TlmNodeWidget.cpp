#include "TlmNodeWidget.h"


/**
 * Function: GetTlmType
 * Notes: See header file
 */
ACTlmEnum::TlmTypeEnum TlmNodeWidget::GetTlmType() const
{
  return TlmType;
}

/**
 * Function: TlmNodeWidget
 * Notes: See header file
 */
TlmNodeWidget::TlmNodeWidget(QTreeWidget* treeView, ACTlmEnum::TlmTypeEnum tlmType) :
  QTreeWidgetItem(treeView),
  TlmType(tlmType)
{

}

/**
 * Function: TlmNodeWidget
 * Notes: See header file
 */
TlmNodeWidget::TlmNodeWidget(QTreeWidgetItem* parentNode, ACTlmEnum::TlmTypeEnum tlmType) :
  QTreeWidgetItem(parentNode),
  TlmType(tlmType)
{

}

/**
 * Function: ~TlmNodeWidget
 * Notes: See header file
 */
TlmNodeWidget::~TlmNodeWidget()
{

}