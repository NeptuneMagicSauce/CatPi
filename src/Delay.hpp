#pragma once

struct QLabel;
struct DeltaDial;
#include <QWidget>

struct Delay : public QWidget {
  Delay(int delaySeconds);

  DeltaDial* delayDial = nullptr;

  void connect();
  void setDelay(int seconds);
  void setRemaining(int seconds);
};
