#pragma once

#include <QWidget>

struct QPushButton;
struct CentralWidgetImpl;

struct CentralWidget : public QWidget {
  CentralWidget();

  QPushButton* widget();

  enum struct Page { Main, Calibration };
  void setPage(Page page);

 private:
  CentralWidgetImpl* impl;
};
