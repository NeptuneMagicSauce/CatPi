#pragma once

struct QTimer;
#include <optional>

struct Logic {
  Logic();

  QTimer* timerEndDispense();

  void connect();
  void manualDispense();
  void update(std::optional<double> weightGrams, double tare);
};
