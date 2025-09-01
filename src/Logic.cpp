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
  int durationDispenseRelayMilliseconds = 4000;

  optional<int> timeToDispenseSeconds;

  QDir logDirectory;
  QFile logFile;
  int delaySeconds = 0;
  QList<Event> events;
  QTimer timerDetectDispensed;  // fires after a dispense and a delay

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
  Settings::load(
      {.key = "DurationRelayImpulse",
       .name = "Durée Rotation Hélice",
       .prompt = "Durée pendant laquelle le moteur tourne l'hélice à chaque distribution",
       .unit = "Secondes",
       .defaultValue = 4,
       .callback =
           [&](QVariant v) {
             impl->durationDispenseRelayMilliseconds = v.toInt() * 1000;
             timerEndDispense->setInterval(impl->durationDispenseRelayMilliseconds);
           },
       .limits = {.minimum = 1, .maximum = 20}});

  timerUpdate = new QTimer;
  timerUpdate->setSingleShot(false);
  Settings::load({.key = "LogicInterval",
                  .name = "Période Logique",
                  .prompt = "Période de rafraichissement de la boucle principale",
                  .unit = "Millisecondes",
                  .defaultValue = 1000,
                  .callback = [&](QVariant v) { timerUpdate->setInterval(v.toInt()); },
                  .limits = {.minimum = 20, .maximum = {}}});

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

  timerDetectDispensed.setSingleShot(true);
  timerDetectDispensed.setInterval(8 * 1000);  // 8 seconds

  QObject::connect(&timerDetectDispensed, &QTimer::timeout, [&] {
    ostringstream ss;
    ss << fixed << setprecision(1) << events.last().grams;
    logEvent("DispensedWeight: " + QString::fromStdString(ss.str()) + " grams");
  });

  Settings::load({.key = delayKey,
                  .name = "Attente Ouverture",
                  .prompt = "Temps d'attente pour une ouverture après que ça été mangé",
                  .unit = "Secondes",
                  .defaultValue = 15 * 60,
                  .callback =
                      [&](QVariant v) {
                        delaySeconds = v.toInt();
                        if (updateGuiCallback != nullptr) {
                          updateGuiCallback(delaySeconds);
                        }
                      },
                  .limits = {.minimum = 10, .maximum = {}}});

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

  QObject::connect(timerEndDispense, &QTimer::timeout, [&] {
    // qDebug() << "CLOSE RELAY" << QDateTime::currentDateTime().time();
    closeRelay();
  });
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
  // qDebug() << "OPEN RELAY" << QDateTime::currentDateTime().time();
  if (Logic::hasGPIO) {
    pinctrl("set 17 op dh");
  }

  timerDetectDispensed.start();

  auto now = QDateTime::currentDateTime();

  events.append({
      .timeDispensed = now,  //
      .grams = 0,            //
      .timeEaten = {}        //
  });

  // when there is no weight sensor: consider it is eaten right away
  if (Logic::hasGPIO == false) {
    events.last().timeEaten = now;
  }

  logEvent(now.toString() + ", dispense, " +
           (mode == DispenseMode::Automatic ? "automatic" : "manual"));
}

void Logic::logWeights(const QString& weights) const { impl->logEvent(weights); }

void Logic::update(optional<double> weightTarred,  //
                   bool isWeightBelowThreshold,    //
                   bool& dispensed,                //
                   bool& justAte) {
  impl->update(weightTarred, isWeightBelowThreshold, dispensed, justAte);
  if (dispensed) {
    impl->afterDispenseCallback();
  }
}

void LogicImpl::update(optional<double> weightTarred,  //
                       bool isWeightBelowThreshold,    //
                       bool& dispensed,                //
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
    if (timerDetectDispensed.isActive() && weightTarred.has_value()) {
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
