#pragma once

struct QTimer;
#include <QString>
#include <optional>

struct Logic {
  Logic();

  bool hasGPIO = false;

  QTimer* timerEndDispense = nullptr;

  int delaySeconds();
  void connect(std::function<void(int)> updateGuiCallback);
  void manualDispense();
  void update(std::optional<double> weightTarred, double tare, bool& dispensed);
  void changeDelay(int delta);
  void closeRelay();
  int timeToDispense();
};
