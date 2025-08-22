#pragma once

struct QTimer;

#include <optional>

#include "Event.hpp"

struct Logic {
  Logic();

  static bool hasGPIO;

  QTimer* timerEndDispense = nullptr;
  QTimer* timerUpdate = nullptr;

  int delaySeconds();
  void connect(std::function<void(int)> updateGuiCallback);
  void manualDispense();
  void update(std::optional<double> weightTarred, bool& dispensed);
  void changeDelay(int delta);
  void setDelaySeconds(int delaySeconds);

  std::optional<int> timeToDispenseSeconds() const;

  static void closeRelay();
  const QList<Event>& events() const;
};
