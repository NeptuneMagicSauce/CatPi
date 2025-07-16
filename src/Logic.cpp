#include "Logic.hpp"

#include <QDateTime>
#include <QFile>
#include <QStandardPaths>
#include <QTimer>
#include <iostream>
#include <vector>

#include "PinCtrl.hpp"
#include "Settings.hpp"
#include "System.hpp"

using namespace std;
using PinCtrl::pinctrl;

bool Logic::hasGPIO = false;

struct LogicImpl {
  LogicImpl();

  optional<QDateTime> previousDispense;
  QFile logFile;
  const QDateTime startTime = QDateTime::currentDateTime();
  const QString delayKey = "Delay";
  int delaySeconds = 0;
  qint64 elapsed = 0;

  void dispense(bool hasGPIO, QTimer* timerEndDispense);
  void logEvent(QString const& event);
  std::function<void(int)> updateGuiCallback = nullptr;
};
namespace {
  LogicImpl* impl = nullptr;
}

Logic::Logic() {
  AssertSingleton();
  timerEndDispense = new QTimer;
  timerEndDispense->setSingleShot(true);
  timerEndDispense->setInterval(4000);
  timerUpdate = new QTimer;
  timerUpdate->setSingleShot(false);
  Settings::load({"LogicInterval",
                  "Période Logique",
                  "Période de rafraichissement de la boucle principale",
                  "Millisecondes",
                  1000,
                  [&](QVariant v) { timerUpdate->setInterval(v.toInt()); },
                  {20, {}}});
  timerUpdate->start();
  impl = new LogicImpl;
}

int Logic::delaySeconds() { return impl->delaySeconds; }

LogicImpl::LogicImpl()
    : logFile(
          QStandardPaths::standardLocations(QStandardPaths::StandardLocation::AppLocalDataLocation)
              .first() +
          "/logs.txt") {
  if (logFile.exists()) {
    logFile.remove();
  }
  cout << "Log: " << logFile.fileName().toStdString() << endl;

  auto logDirectory = logFile.filesystemFileName().parent_path();
  std::filesystem::create_directories(logDirectory);

  Settings::load({delayKey,
                  "Attente Ouverture",
                  "Temps d'attente entre chaque ouverture (basé sur la derniere ouverture ou "
                  "la dernière fois que c'était vide?)",
                  "Secondes",
                  15 * 60,
                  [&](QVariant v) {
                    delaySeconds = v.toInt();
                    if (updateGuiCallback != nullptr) {
                      updateGuiCallback(delaySeconds);
                    }
                  },
                  {10, {}}});
}

void Logic::closeRelay() {
  if (hasGPIO) {
    pinctrl("set 17 op dl");
  }
}

void Logic::connect(std::function<void(int)> updateGuiCallback) {
  impl->updateGuiCallback = updateGuiCallback;
  updateGuiCallback(impl->delaySeconds);
  // call it right away because it was not available earlier, in the constructor

  QObject::connect(timerEndDispense, &QTimer::timeout, [&] { closeRelay(); });
}

void Logic::manualDispense() { impl->dispense(hasGPIO, timerEndDispense); }

void LogicImpl::dispense(bool hasGPIO, QTimer* timerEndDispense) {
  // std::cout << __PRETTY_FUNCTION__ << std::endl;
  // pinctrl("-p");
  if (hasGPIO) {
    pinctrl("set 17 op dh");
  }
  timerEndDispense->start();

  auto now = QDateTime::currentDateTime();

  auto log = QString{">> Dispense, elapsed "};
  log += QString::number(previousDispense.has_value() ? previousDispense.value().secsTo(now) : -1);
  log += QString{", "} + now.toString();
  logEvent(log);

  previousDispense = now;
}

void Logic::update(std::optional<double> weightTarred, double tare, bool& dispensed) {
  auto& dispenseTime = impl->previousDispense;
  auto now = QDateTime::currentDateTime();

  auto weightAboveThreshold =
      weightTarred.has_value() ? (weightTarred.value() < 0.8) : (hasGPIO ? false : true);
  auto& start = dispenseTime.has_value() ? dispenseTime.value() : impl->startTime;
  auto elapsed = start.secsTo(now);
  auto timeAboveThreshold = elapsed > impl->delaySeconds;
  impl->elapsed = elapsed;

  auto log = QString{};
  for (auto toLog : vector<QString>{
           now.time().toString(),
           weightTarred.has_value() ? QString::number(weightTarred.value()) : QString{"no weight"},
           QString{"tare: "} + QString::number(tare),
           QString{"dispense: "} +
               (dispenseTime.has_value() ? dispenseTime.value().time().toString() : QString{""}),
           QString{"elapsed: "} + QString::number(elapsed),
       }) {
    log += toLog + ", ";
  }
  impl->logEvent(log);

  if (timeAboveThreshold && weightAboveThreshold) {
    impl->dispense(hasGPIO, timerEndDispense);
    dispensed = true;
  }
}

void LogicImpl::logEvent(QString const& event) {
  // cout << log.toStdString() << endl;
  auto& file = impl->logFile;
  file.open(QIODeviceBase::WriteOnly | QIODeviceBase::Append);
  file.write((event + "\n").toUtf8());
  file.close();
}

int Logic::timeToDispense() { return std::max(0, impl->delaySeconds - (int)impl->elapsed); }

void Logic::changeDelay(int delta) {
  if (impl->delaySeconds >= 60) {
    delta *= 10;
  }
  setDelaySeconds(impl->delaySeconds + delta);
}

void Logic::setDelaySeconds(int delaySeconds) {
  impl->delaySeconds = std::max(10, delaySeconds);
  Settings::set(impl->delayKey, impl->delaySeconds);
}
