#include "Logic.hpp"

#include <QDateTime>
#include <QDir>
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

  // optional<QDateTime> timeOfDispense;
  // optional<QDateTime> timeOfEating;
  // double dispensedWeight = 0.0;
  // bool dispensedIsEaten = false;  // to remove
  // qint64 elapsedSeconds = 0;

  optional<int> timeToDispenseSeconds;

  QDir logDirectory;
  QFile logFile;
  int delaySeconds = 0;
  QList<Event> events;
  double weightThreshold = 0.4;

  enum struct DispenseMode { Automatic, Manual };

  void update(optional<double> weightTarred, bool& dispensed);
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
  Settings::load({"WeightThresholdEaten",
                  "Poids Minimum Mangé",
                  "Poids en dessous duquel on détecte que c'est mangé",
                  "Déci-Grammes",
                  4,
                  [&](QVariant v) { impl->weightThreshold = (double)v.toInt() / 10; },
                  {1, 20}});
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
  updateLogFile();
  cout << "Log: " << logFile.fileName().toStdString() << endl;

  auto logDirectory = logFile.filesystemFileName().parent_path();
  std::filesystem::create_directories(logDirectory);

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
  updateLogFile();
  auto& file = impl->logFile;
  file.open(QIODeviceBase::WriteOnly | QIODeviceBase::Append);
  file.write((event + "\n").toUtf8());
  file.close();
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
  // if (impl->timeOfEating.has_value() == false && impl->events.empty() == false) {
  //   // not eaten, and already dispensed -> no ETA
  //   return {};
  // }
  // auto start = impl->timeOfEating.value_or(impl->startTime);
  // return start.secsTo(QDateTime::currentDateTime());
}

// int Logic::timeToDispenseToRemove() {
//   return std::max(0, impl->delaySeconds - (int)impl->elapsedSeconds);
// }

void Logic::manualDispense() {
  impl->dispense(DispenseMode::Manual);
  impl->afterDispenseCallback();
}

void LogicImpl::dispense(DispenseMode mode) {
  // std::cout << __PRETTY_FUNCTION__ << std::endl;
  if (Logic::hasGPIO) {
    pinctrl("set 17 op dh");
  }

  auto now = QDateTime::currentDateTime();

  // timeOfDispense = now;
  // timeOfEating = {};
  // dispensedIsEaten = false;
  // dispensedWeight = 0;

  events.append({now, 0, {}});

  // when there is no weight sensor: consider it is eaten right away
  if (Logic::hasGPIO == false) {
    events.last().timeEaten = now;
  }

  auto log = now.toString();
  log += ", dispense, ";
  log += mode == DispenseMode::Automatic ? "automatic" : "manual";
  logEvent(log);
}

void Logic::update(std::optional<double> weightTarred, bool& dispensed) {
  impl->update(weightTarred, dispensed);
  if (dispensed) {
    impl->afterDispenseCallback();
  }
}

void LogicImpl::update(std::optional<double> weightTarred, bool& dispensed) {
  // TODO frequencies, performance:
  // is load cell updating really only every 1000 milliseconds?
  // how is increasing the logic frequency making a more accurate weight measure then?
  // do not update the GUI as frequently, it only needs every 1 second
  // <-> do not update the Logic as frequently

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

  // compute if weight is below threshold
  auto weightBelowThreshold = false;
  if (weightTarred.has_value()) {
    // if weight is available
    weightBelowThreshold = weightTarred.value() < weightThreshold;
  } else if (Logic::hasGPIO == false) {
    // if there is no weight sensor
    weightBelowThreshold = true;
  }

  // to remove
  // auto start = timeOfDispense.value_or(startTime);
  // elapsedSeconds = start.secsTo(now);

  // // time threshold should not depend only on time since dispense
  // // because then it will re-dispense right after eating
  // // time threshold should depend on time since eating
  // auto timeAboveThreshold = false;
  // if (events.empty() == false && events.last().timeEaten.has_value()) {
  //   auto elapsedSecondsSinceEating = events.last().timeEaten.value().secsTo(now);
  //   timeAboveThreshold = elapsedSecondsSinceEating > delaySeconds;
  // } else {
  //   timeAboveThreshold = elapsedSeconds > delaySeconds;
  // }

  if (timeOfDispense.has_value()) {
    // time since dispense
    auto timeSinceDispenseSeconds = timeOfDispense.value().secsTo(now);

    auto& lastEvent = events.last();

    // dispensed weight
    if (timeSinceDispenseSeconds < 10 && weightTarred.has_value()) {
      auto& dispensedWeight = lastEvent.grams;
      dispensedWeight = std::max(weightTarred.value(), dispensedWeight);
    }

    // just ate ?
    if (lastEvent.timeEaten.has_value() == false &&                                     //
        timeSinceDispenseSeconds > (durationDispenseRelayMilliseconds / 1000) + 2.0 &&  //
        weightBelowThreshold) {
      // yes
      lastEvent.timeEaten = now;
      logEvent(now.toString() + ", eat");
    }
  }

  // compute the dispensed weight
  // if (timeOfDispense.has_value() && elapsedSeconds < 10 && weightTarred.has_value()) {
  //   dispensedWeight = std::max(weightTarred.value(), dispensedWeight);
  //   if (events.empty() == false) {  // must be true
  //     events.last().grams = dispensedWeight;
  //   }
  // }

  // auto justAte = timeOfDispense.has_value() &&                                         //
  //                dispensedIsEaten == false &&                                          //
  //                elapsedSeconds > (durationDispenseRelayMilliseconds / 1000) + 2.0 &&  //
  //                weightBelowThreshold == true;
  // if (justAte) {
  //   timeOfEating = now;
  //   dispensedIsEaten = true;
  //   if (events.empty() == false) {  // must be true
  //     auto& event = events[events.size() - 1];
  //     event.timeEaten = now;
  //     logEvent(now.toString() + ", eat");
  //   }
  // }

  // auto log = QString{};
  // for (auto toLog : vector<QString>{
  //          now.time().toString(),
  //          weightTarred.has_value() ? QString::number(weightTarred.value()) : QString{"no
  //          weight"}, QString{"tare: "} + QString::number(tare), QString{"dispense: "} +
  //              (dispenseTime.has_value() ? dispenseTime.value().time().toString() : QString{""}),
  //          QString{"elapsed: "} + QString::number(elapsedSeconds),
  //          QString{"justAte: "} + QString::number((int)justAte),
  //      }) {
  //   log += toLog + ", ";
  // }
  // logEvent(log);

  // if (timeAboveThreshold && weightBelowThreshold) {
  if (timeToDispenseSeconds.value_or(1) <= 0 &&  // time below threshold
      weightBelowThreshold) {
    dispense(DispenseMode::Automatic);
    dispensed = true;
  }

  // remove events older than 24 hours
  while (events.isEmpty() == false && events.first().timeDispensed.secsTo(now) > 60 * 60 * 24) {
    events.removeFirst();
  }
}
