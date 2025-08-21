#pragma once

struct QTimer;

#include <QDateTime>
#include <QList>
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
  void setDelaySeconds(int delaySeconds);
  int timeToDispense();
  static void closeRelay();

  // TODO move this declaration out of Logic class and remove pragma
  struct Event {
    QDateTime timeDispensed;
    std::optional<QDateTime> timeEaten;
  };
  const QList<Event>& events() const;
};
