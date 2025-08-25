#include "Logic.hpp"

#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QStandardPaths>
#include <QTimer>
#include <iostream>

#include "PinCtrl.hpp"
#include "Settings.hpp"
#include "System.hpp"

using namespace std;
using PinCtrl::pinctrl;

bool Logic::hasGPIO = false;

struct LogicImpl {
  LogicImpl();

  const QDateTime startTime = QDateTime::currentDateTime();
  const QString delayKey = "Delay";
  const int durationDispenseRelayMilliseconds = 4000;

  optional<int> timeToDispenseSeconds;

  QDir logDirectory;
  QFile logFile;
  int delaySeconds = 0;
  QList<Event> events;
  QTimer timerDispensedWeight;

  enum struct DispenseMode { Automatic, Manual };

  void update(std::optional<double> weightTarred,  //
              bool isWeightBelowThreshold,         //
              bool& dispensed,                     //
              bool& justAte);
  void dispense(DispenseMode mode);
  void logEvent(QString const& event);

  function<void(int)> updateGuiCallback = nullptr;
  function<void()> afterDispenseCallback = nullptr;

  void updateLogFile();
};

using DispenseMode = LogicImpl::DispenseMode;

namespace {
  LogicImpl* impl = nullptr;
}

Logic::Logic() {
  AssertSingleton();
  impl = new LogicImpl;
  impl->afterDispenseCallback = [&] { timerEndDispense->start(); };
  timerEndDispense = new QTimer;
  timerEndDispense->setSingleShot(true);
  timerEndDispense->setInterval(impl->durationDispenseRelayMilliseconds);
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
}

int Logic::delaySeconds() { return impl->delaySeconds; }

void LogicImpl::updateLogFile() {
  auto const fileName = QDateTime::currentDateTime().date().toString(Qt::ISODate);
  auto const path = logDirectory.path() + "/" + fileName + ".txt";
  if (logFile.fileName().isEmpty() == false && logFile.fileName() != path) {
    logFile.close();
  }
  logFile.setFileName(path);
}

LogicImpl::LogicImpl() {
  logDirectory =
      QStandardPaths::standardLocations(QStandardPaths::StandardLocation::AppLocalDataLocation)
          .first() +
      "/logs";
  std::filesystem::create_directories(logDirectory.path().toStdString());
  cout << "Logs: " << logDirectory.path().toStdString() << "/" << endl;

  timerDispensedWeight.setSingleShot(true);
  timerDispensedWeight.setInterval(8 * 1000);  // 8 seconds

  QObject::connect(&timerDispensedWeight, &QTimer::timeout, [&] {
    ostringstream ss;
    ss << fixed << setprecision(1) << events.last().grams;
    logEvent("DispensedWeight: " + QString::fromStdString(ss.str()) + " grams");
  });

  Settings::load({delayKey,
                  "Attente Ouverture",
                  "Temps d'attente pour une ouverture après que ça été mangé",
                  "Secondes",
                  15 * 60,
                  [&](QVariant v) {
                    delaySeconds = v.toInt();
                    if (updateGuiCallback != nullptr) {
                      updateGuiCallback(delaySeconds);
                    }
                  },
                  {10, {}}});

  logEvent("=== boot === " + QDateTime::currentDateTime().toString());
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

void LogicImpl::logEvent(QString const& event) {
  // cout << log.toStdString() << endl;

  // print log event to file
  updateLogFile();
  logFile.open(QIODeviceBase::WriteOnly | QIODeviceBase::Append);
  logFile.write((event + "\n").toUtf8());
  logFile.close();

  // print log event to console
  cout << event.toStdString() << endl;
}

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

const QList<Event>& Logic::events() const { return impl->events; }

optional<int> Logic::timeToDispenseSeconds() const {
  if (impl->timeToDispenseSeconds.value_or(1) < 0) {
    return 0;
  }

  return impl->timeToDispenseSeconds;
}

void Logic::manualDispense() {
  impl->dispense(DispenseMode::Manual);
  impl->afterDispenseCallback();
}

void LogicImpl::dispense(DispenseMode mode) {
  // std::cout << __PRETTY_FUNCTION__ << std::endl;
  if (Logic::hasGPIO) {
    pinctrl("set 17 op dh");
  }

  timerDispensedWeight.start();

  auto now = QDateTime::currentDateTime();

  events.append({now, 0, {}});

  // when there is no weight sensor: consider it is eaten right away
  if (Logic::hasGPIO == false) {
    events.last().timeEaten = now;
  }

  logEvent(now.toString() + ", dispense, " +
           (mode == DispenseMode::Automatic ? "automatic" : "manual"));
}

void Logic::logWeights(const QString& weights) const { impl->logEvent(weights); }

void Logic::update(std::optional<double> weightTarred, bool isWeightBelowThreshold, bool& dispensed,
                   bool& justAte) {
  impl->update(weightTarred, isWeightBelowThreshold, dispensed, justAte);
  if (dispensed) {
    impl->afterDispenseCallback();
  }
}

void LogicImpl::update(std::optional<double> weightTarred,  //
                       bool isWeightBelowThreshold,         //
                       bool& dispensed,                     //
                       bool& justAte) {
  auto now = QDateTime::currentDateTime();

  auto timeOfDispense = optional<QDateTime>{};
  if (events.empty() == false) {
    timeOfDispense = events.last().timeDispensed;
  }

  // compute time to dispense
  if (timeOfDispense.has_value() == false) {
    // never dispensed anything
    timeToDispenseSeconds = delaySeconds - startTime.secsTo(now);
  } else if (events.last().timeEaten.has_value() == false) {
    // dispensed something, it is not eaten
    timeToDispenseSeconds = {};
  } else {
    // dispensed something, it was eaten
    timeToDispenseSeconds = delaySeconds - events.last().timeEaten.value().secsTo(now);
  }

  if (timeOfDispense.has_value()) {
    // time since dispense

    auto& lastEvent = events.last();

    // dispensed weight
    if (timerDispensedWeight.isActive() && weightTarred.has_value()) {
      auto& dispensedWeight = lastEvent.grams;
      dispensedWeight = std::max(weightTarred.value(), dispensedWeight);
    }

    // just ate ?
    if (lastEvent.timeEaten.has_value() == false &&                                               //
        timeOfDispense.value().secsTo(now) > (durationDispenseRelayMilliseconds / 1000) + 2.0 &&  //
        isWeightBelowThreshold) {
      // yes
      lastEvent.timeEaten = now;
      logEvent(now.toString() + ", eat");
      justAte = true;
    }
  }

  // dispense if it is time
  // do not check that weight is below threshold
  // because when it is time, dispensed has been eaten
  if (timeToDispenseSeconds.value_or(1) <= 0) {
    dispense(DispenseMode::Automatic);
    dispensed = true;
  }

  // remove events older than 24 hours
  while (events.isEmpty() == false && events.first().timeDispensed.secsTo(now) > 60 * 60 * 24) {
    events.removeFirst();
  }
}
