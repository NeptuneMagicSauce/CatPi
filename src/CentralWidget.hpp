#pragma once

#include <QWidget>

struct Instance;
struct CentralWidgetImpl;

struct CentralWidget : public QWidget {
  CentralWidget(Instance*);
  enum struct Page { Main, Calibration };
  void setPage(Page page);

 private:
  CentralWidgetImpl* impl;
};
