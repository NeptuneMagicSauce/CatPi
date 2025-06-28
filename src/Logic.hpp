#pragma once

struct QTimer;
#include <QString>
#include <optional>

struct Logic {
  Logic(bool hasGPIO);

  QTimer* timerEndDispense();

  void connect();
  void manualDispense();
  QString timeToDispense();
  void update(std::optional<double> weightGrams, double tare);
};
