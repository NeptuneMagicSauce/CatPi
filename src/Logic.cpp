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

struct LogicImpl {
  LogicImpl();

  QTimer* timerEndDispense = new QTimer;
  optional<QDateTime> previousDispense;
  QFile logFile;
  const QDateTime startTime = QDateTime::currentDateTime();
  const QString delayKey = "Delay";
  int delaySeconds = 0;
  qint64 elapsed = 0;
  bool hasGPIO = false;

  void dispense();
  void logEvent(QString const& event);
};
namespace {
LogicImpl* impl = nullptr;
}

Logic::Logic(bool hasGPIO) {
  AssertSingleton();
  impl = new LogicImpl;
  impl->hasGPIO = hasGPIO;
}

QTimer* Logic::timerEndDispense() { return impl->timerEndDispense; }
int Logic::delaySeconds() { return impl->delaySeconds; }

LogicImpl::LogicImpl()
    : logFile(
          QStandardPaths::standardLocations(QStandardPaths::StandardLocation::AppLocalDataLocation).first() +
          "/logs.txt") {
  if (logFile.exists()) {
    logFile.remove();
  }
  cout << "Log: " << logFile.fileName().toStdString() << endl;

  auto logDirectory = logFile.filesystemFileName().parent_path();
  std::filesystem::create_directories(logDirectory);

  delaySeconds = Settings::getInt(delayKey, 15 * 60);

  timerEndDispense = new QTimer;
  timerEndDispense->setSingleShot(true);
  timerEndDispense->setInterval(4000);
}

void Logic::reset() {
  if (impl->hasGPIO) {
    pinctrl("set 17 op dl");
  }
}

void Logic::connect() {
  QObject::connect(impl->timerEndDispense, &QTimer::timeout, [&] { reset(); });
}

void Logic::manualDispense() { impl->dispense(); }

void LogicImpl::dispense() {
  // std::cout << __PRETTY_FUNCTION__ << std::endl;
  // pinctrl("-p");
  if (impl->hasGPIO) {
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

  auto weightAboveThreshold = weightTarred.has_value() ? (weightTarred.value() < 0.8) : false;
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
    impl->dispense();
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
  impl->delaySeconds += delta;
  impl->delaySeconds = std::max(10, impl->delaySeconds);
  Settings::setInt(impl->delayKey, impl->delaySeconds);
}
