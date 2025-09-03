#pragma once

struct QTimer;
struct Logs;

#include <functional>
#include <optional>

struct Logic {
  Logic(Logs& logs, const double& weightThresholdGrams);

  static bool hasGPIO;

  QTimer* timerAllowManualDispense = nullptr;
  QTimer* timerUpdate = nullptr;

  int delaySeconds();
  struct Callbacks {
    std::function<void(int seconds)> updateGuiDelay;
    std::function<void(bool doTare)> onDispense;
  };
  void connect(const Callbacks& callbacks);
  void manualDispense();
  void update(std::optional<double> weightTarred,  //
              bool isWeightBelowThreshold,         //
              bool& justAte);
  void changeDelay(int delta);
  void setDelaySeconds(int delaySeconds);

  std::optional<int> timeToDispenseSeconds() const;

  static void closeRelay();
};
