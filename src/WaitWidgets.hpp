#pragma once

struct QLabel;
struct DeltaDial;
#include <QWidget>

struct WaitWidgets : public QWidget {
  WaitWidgets(int delaySeconds);

  DeltaDial* delayDial = nullptr;

  void connect();
  void setDelay(int delaySeconds);
  void setTimeToDispense(int seconds);
};
