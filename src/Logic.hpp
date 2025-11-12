#pragma once

struct QTimer;
struct Logs;

#include <QTimer>
#include <functional>
#include <optional>

struct Logic {
  Logic(Logs& logs, const double& weightThresholdGrams);

  static bool hasGPIO;

  QTimer eventAllowManualDispense;  // notify GUI
  QTimer* timerUpdate = nullptr;

  int delaySeconds();
  struct Callbacks {
    std::function<void(int seconds)> updateGuiDelay;
    std::function<void(bool doTare)> onDispense;
    std::function<void()> onEndDetectWeight;
  };
  void connect(const Callbacks& callbacks);
  void manualDispense();
  void update(std::optional<double> weightTarred,  //
              bool isWeightBelowThreshold,         //
              bool& justAte);                      //
                                                   // bool& detectingDispensedWeight);
  void changeDelay(int delta);
  void setDelaySeconds(int delaySeconds);

  std::optional<int> timeToDispenseSeconds() const;

  static void closeRelay();
};
