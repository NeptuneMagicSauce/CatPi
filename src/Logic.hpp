#pragma once

struct QTimer;
#include <QString>
#include <optional>

struct Logic {
  Logic(bool hasGPIO);

  QTimer* timerEndDispense();
  int delaySeconds();

  void connect();
  void manualDispense();
  void update(std::optional<double> weightGrams, double tare);
  void changeDelay(int delta);

  int timeToDispense();
};
