#pragma once
#include "ACTlmEnum.h"
#include <QTreeWidgetItem>

class QTreeWidget;

/**
  * Class: TlmNodeWidget
  * Brief: This widget represents an individual tree node for each telemetry measurand.
  */
class TlmNodeWidget : public QTreeWidgetItem
{

public:

  ACTlmEnum::TlmTypeEnum GetTlmType() const;
  TlmNodeWidget(QTreeWidget* treeView, ACTlmEnum::TlmTypeEnum tlmType);
  TlmNodeWidget(QTreeWidgetItem* parentNode, ACTlmEnum::TlmTypeEnum tlmType);
  ~TlmNodeWidget();

private:

  ACTlmEnum::TlmTypeEnum TlmType;
};

