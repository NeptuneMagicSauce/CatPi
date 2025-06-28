#pragma once

#include <QWidget>

struct QAbstractButton;
struct QWidget;

struct CentralWidget : public QWidget {
  CentralWidget(QWidget* weight, QWidget* calibration, QWidget* waitwidgets);

  QAbstractButton* dispenseButton();

  enum struct Page { Main, Calibration };
  void setPage(Page page);

  void statusMessage(const QString&);
};
