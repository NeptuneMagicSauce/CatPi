#pragma once

#include <QWidget>

struct QPushButton;
struct QWidget;
struct CentralWidgetImpl;

struct CentralWidget : public QWidget {
  CentralWidget(QWidget* weight, QWidget* calibration);

  QPushButton* dispenseButton();

  enum struct Page { Main, Calibration };
  void setPage(Page page);

 private:
  CentralWidgetImpl* impl;
};
