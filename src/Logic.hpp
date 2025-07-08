#pragma once

struct QTimer;
#include <QString>
#include <optional>

struct Logic {
  Logic();

  static bool hasGPIO;

  QTimer* timerEndDispense = nullptr;
  QTimer* timerUpdate = nullptr;

  int delaySeconds();
  void connect(std::function<void(int)> updateGuiCallback);
  void manualDispense();
  void update(std::optional<double> weightTarred, double tare, bool& dispensed);
  void changeDelay(int delta);
  int timeToDispense();
  static void closeRelay();
};
