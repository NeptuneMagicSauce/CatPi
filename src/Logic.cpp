#include "Logic.hpp"

#include <QDateTime>
#include <QFile>
#include <QStandardPaths>
#include <QTimer>
#include <iostream>
#include <vector>

#include "PinCtrl.hpp"
#include "System.hpp"

using namespace std;
using PinCtrl::pinctrl;

struct LogicImpl {
  LogicImpl();

  QTimer* timerEndDispense = new QTimer;
  optional<QDateTime> previousDispense;
  QFile logFile;
  const QDateTime startTime = QDateTime::currentDateTime();

  void dispense();
  void logEvent(QString const& event);
};
namespace {
LogicImpl* impl = nullptr;
}

Logic::Logic() {
  AssertSingleton();
  impl = new LogicImpl;
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

  timerEndDispense = new QTimer;
  timerEndDispense->setSingleShot(true);
  timerEndDispense->setInterval(4000);
}

void Logic::connect() {
  QObject::connect(impl->timerEndDispense, &QTimer::timeout, [&] { pinctrl("set 17 op dl"); });
}

void Logic::manualDispense() { impl->dispense(); }

void LogicImpl::dispense() {
  // std::cout << "released" << std::endl;
  // pinctrl("-p");
  pinctrl("set 17 op dh");
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

  auto weightAboveThreshold = weightGrams.has_value() ? (weightGrams > 1.0) : false;
  auto& start = dispenseTime.has_value() ? *dispenseTime : impl->startTime;
  auto timeAboveThreshold = start.secsTo(now) > 50;

  auto log = QString{};
  for (auto toLog : vector<QString>{
           now.time().toString(),
           weightGrams.has_value() ? QString::number(*weightGrams) : QString{"no weight"},
           QString{"tare: "} + QString::number(tare),
           QString{"dispense: "} + (dispenseTime.has_value() ? dispenseTime->time().toString() : QString{""}),
           QString{"elapsed: "} + QString::number(start.secsTo(now)),
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
