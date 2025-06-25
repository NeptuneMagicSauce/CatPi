#pragma once

#include <QWidget>

struct QPushButton;
struct Instance;
struct CentralWidgetImpl;

struct CentralWidget : public QWidget {
  CentralWidget(Instance*);

  QPushButton* widget();

  enum struct Page { Main, Calibration };
  void setPage(Page page);

 private:
  CentralWidgetImpl* impl;
};
