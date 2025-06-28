#pragma once

struct QLabel;
#include <QWidget>

struct WaitWidgets : public QWidget {
  WaitWidgets();

  QLabel* timeToDispense = nullptr;

  void connect();
};
