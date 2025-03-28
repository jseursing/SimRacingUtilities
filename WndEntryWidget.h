#pragma once
#include <QMouseEvent>
#include <QLabel>

class QLabel;

/**
  * Class: WndEntryWidget
  * Brief: This widget represents the object which displays application information
  *        and provides and interface for the user to register an application
  *        for window management. 
  */
class WndEntryWidget : public QLabel
{
  Q_OBJECT

public:

  WndEntryWidget(uint32_t widgetW, 
                 uint32_t widgetH, 
                 const char* exeStr, 
                 int32_t x, 
                 int32_t y, 
                 uint32_t w, 
                 uint32_t h, 
                 bool reSize, 
                 bool stripBorders,
                 QWidget* parent = nullptr);
  ~WndEntryWidget();

  std::string Filename;
};

