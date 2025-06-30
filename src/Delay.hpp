#pragma once

struct QLabel;
struct DeltaDial;
#include <QWidget>

struct Delay : public QWidget {
  Delay();

  DeltaDial* delayDial = nullptr;

  void connect();
  void setDelay(int seconds);
  void setRemaining(int seconds);
};
