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

  delaySeconds = Settings::instance().value(delayKey, 15 * 60).toInt();
  delaySeconds = 10;  // debug

  timerEndDispense = new QTimer;
  timerEndDispense->setSingleShot(true);
  timerEndDispense->setInterval(4000);
}

void Logic::connect() {
  QObject::connect(impl->timerEndDispense, &QTimer::timeout, [&] {
    if (impl->hasGPIO) {
      pinctrl("set 17 op dl");
    }
  });
}

void Logic::manualDispense() { impl->dispense(); }

void LogicImpl::dispense() {
  // std::cout << "released" << std::endl;
  // pinctrl("-p");
  if (impl->hasGPIO) {
    pinctrl("set 17 op dh");
  }
  timerEndDispense->start();

  auto now = QDateTime::currentDateTime();

  auto log = QString{">> Dispense, timeSinceDispense: "};
  log += QString::number(previousDispense.has_value() ? previousDispense->secsTo(now) : -1);
  logEvent(log);

  previousDispense = now;
}

void Logic::update(std::optional<double> weightGrams, double tare) {
  auto& dispenseTime = impl->previousDispense;
  auto now = QDateTime::currentDateTime();

  auto weightAboveThreshold = weightGrams.has_value() ? (weightGrams < 1.0) : false;
  auto& start = dispenseTime.has_value() ? *dispenseTime : impl->startTime;
  auto elapsed = start.secsTo(now);
  auto timeAboveThreshold = elapsed > impl->delaySeconds;
  impl->elapsed = elapsed;

  auto log = QString{};
  for (auto toLog : vector<QString>{
           now.time().toString(),
           weightGrams.has_value() ? QString::number(*weightGrams) : QString{"no weight"},
           QString{"tare: "} + QString::number(tare),
           QString{"dispense: "} + (dispenseTime.has_value() ? dispenseTime->time().toString() : QString{""}),
           QString{"elapsed: "} + QString::number(elapsed),
       }) {
    log += toLog + ", ";
  }
  impl->logEvent(log);

  // TODO
  // dispense: button set enabled false
  if (timeAboveThreshold && weightAboveThreshold) {
    impl->dispense();
  }
}

void LogicImpl::logEvent(QString const& event) {
  // cout << log.toStdString() << endl;
  auto& file = impl->logFile;
  file.open(QIODeviceBase::WriteOnly | QIODeviceBase::Append);
  file.write((event + "\n").toUtf8());
  file.close();
}

QString Logic::timeToDispense() {
  auto timeTo = std::max(0, impl->delaySeconds - (int)impl->elapsed);

  if (timeTo >= 60) {
    auto minutes = std::round((double)timeTo / 60);
    return QString::number(minutes) + " minute" + (minutes > 1 ? "s" : "");
  }

  return QString::number(timeTo) + " second" + (timeTo > 1 ? "s" : "");
}
