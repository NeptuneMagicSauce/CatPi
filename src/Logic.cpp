#include "Logic.hpp"

#include <QDateTime>
#include <QElapsedTimer>
#include <QTimer>
#include <iostream>
#include <map>

#include "Logs.hpp"
#include "PinCtrl.hpp"
#include "Settings.hpp"
#include "System.hpp"

using namespace std;
using PinCtrl::pinctrl;

bool Logic::hasGPIO = false;

struct LogicImpl {
  LogicImpl(Logs& logs, const double& weightThresholdGrams);

  const QDateTime startTime = QDateTime::currentDateTime();
  const QString delayKey = "Delay";

  Logs& logs;
  const double& weightThresholdGrams;  // reference from owner class

  int durationDispenseRelayMilliseconds = 4000;
  optional<int> timeToDispenseSeconds;

  int delaySeconds = 0;
  QTimer timerEndDispense;          // close relay
  QTimer timerDetectDispensed;      // detect if and how much has been dispensed
  QTimer timerAllowManualDispense;  // notify GUI
  int numberOfDispenseRepeats = 0;

  Logic::Callbacks callbacks;

  enum struct Mode { Automatic, Manual, Repeat };
  const map<Mode, QString> modeNames = {
      {Mode::Automatic, "automatic"},
      {Mode::Manual, "manual"},
      {Mode::Repeat, "repeat"},
  };

  void update(std::optional<double> weightTarred,  //
              bool isWeightBelowThreshold,         //
              bool& justAte);
  void dispense(Mode mode);
  void endDetectDispense();
  bool needsRepeat() const;
  auto& events() { return logs.events.data; }
  auto& events() const { return logs.events.data; }
};

namespace {
  LogicImpl* impl = nullptr;
}

Logic::Logic(Logs& logs, const double& weightThresholdGrams) {
  AssertSingleton();
  impl = new LogicImpl{logs, weightThresholdGrams};

  timerAllowManualDispense = &impl->timerAllowManualDispense;

  timerUpdate = new QTimer;
  timerUpdate->setSingleShot(false);
  timerUpdate->start();

  Settings::load({.key = "LogicInterval",
                  .name = "Période Logique",
                  .prompt = "Période de rafraichissement de la boucle principale",
                  .unit = "Millisecondes",
                  .defaultValue = 1000,
                  .callback = [&](QVariant v) { timerUpdate->setInterval(v.toInt()); },
                  .limits = {.minimum = 20, .maximum = {}}});
}

LogicImpl::LogicImpl(Logs& logs, const double& weightThresholdGrams)
    : logs(logs), weightThresholdGrams(weightThresholdGrams) {
  timerAllowManualDispense.setSingleShot(true);
  timerAllowManualDispense.setInterval(0);

  timerDetectDispensed.setSingleShot(true);
  QObject::connect(&timerDetectDispensed, &QTimer::timeout, [&] { endDetectDispense(); });

  timerEndDispense.setSingleShot(true);
  QObject::connect(&timerEndDispense, &QTimer::timeout, [&] {
    // qDebug() << "CLOSE RELAY" << QDateTime::currentDateTime().time();
    Logic::closeRelay();
  });

  Settings::load({.key = delayKey,
                  .name = "Attente Ouverture",
                  .prompt = "Temps d'attente pour une ouverture après que ça été mangé",
                  .unit = "Secondes",
                  .defaultValue = 15 * 60,
                  .callback =
                      [&](QVariant v) {
                        delaySeconds = v.toInt();
                        if (callbacks.updateGuiDelay != nullptr) {
                          callbacks.updateGuiDelay(delaySeconds);
                        }
                      },
                  .limits = {.minimum = 10, .maximum = {}}});

  Settings::load(
      {.key = "DurationRelayImpulse",
       .name = "Durée Rotation Hélice",
       .prompt = "Durée pendant laquelle le moteur tourne l'hélice à chaque distribution",
       .unit = "Secondes",
       .defaultValue = 4,
       .callback =
           [&](QVariant v) {
             durationDispenseRelayMilliseconds = v.toInt() * 1000;
             timerEndDispense.setInterval(durationDispenseRelayMilliseconds);

             // duration of "detect dispense" period = duration of "active dispense" + 3 seconds
             timerDetectDispensed.setInterval(timerEndDispense.interval() + 3000);
           },
       .limits = {.minimum = 1, .maximum = 20}});

  logs.logEvent("=== boot === " + QDateTime::currentDateTime().toString());
}

void Logic::closeRelay() {
  if (hasGPIO) {
    pinctrl("set 17 op dl");
  }
}

int Logic::delaySeconds() { return impl->delaySeconds; }

void Logic::connect(const Callbacks& callbacks) {
  impl->callbacks = callbacks;

  impl->callbacks.updateGuiDelay(impl->delaySeconds);
  // call it right away because it was not available earlier, in the constructor
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

optional<int> Logic::timeToDispenseSeconds() const {
  if (impl->timeToDispenseSeconds.value_or(1) < 0) {
    return 0;
  }

  return impl->timeToDispenseSeconds;
}

void Logic::update(optional<double> weightTarred,  //
                   bool isWeightBelowThreshold,    //
                   bool& justAte) {
  impl->update(weightTarred, isWeightBelowThreshold, justAte);
}

void Logic::manualDispense() { impl->dispense(LogicImpl::Mode::Manual); }

void LogicImpl::dispense(Mode mode) {
  // std::cout << __PRETTY_FUNCTION__ << std::endl;
  // qDebug() << "OPEN RELAY" << QDateTime::currentDateTime().time();
  if (Logic::hasGPIO) {
    pinctrl("set 17 op dh");
  }

  callbacks.onDispense(mode != Mode::Repeat);  // do tare if not mode repeat in order to accumulate
  timerEndDispense.start();
  timerDetectDispensed.start();

  auto now = QDateTime::currentDateTime();

  if (mode == Mode::Repeat) {
    ++numberOfDispenseRepeats;
  } else {
    numberOfDispenseRepeats = 1;

    // do not create a new event on repeat
    // because the event owns "dispensed weight"
    // which must accumulate on repeats
    events().append({
        .timeDispensed = now,  //
        .grams = 0,            //
        .timeEaten = {}        //
    });
  }

  //   // when there is no weight sensor: consider it is eaten right away
  //   if (Logic::hasGPIO == false) {
  // #error "see next line"
  //     // update() assumes if detecting-dispense or needs-repeat, do not compute just-ate
  //     // so with no weight sensor, we can not set just-ate here
  //     // otherwise we would never go through the repeat code
  //     events().last().timeEaten = now;
  //   }

  logs.logEvent(now.toString() + ", dispense, " + modeNames.at(mode));
}

bool LogicImpl::needsRepeat() const {
  return events().last().grams < weightThresholdGrams &&  //
         numberOfDispenseRepeats < 3;
}

void LogicImpl::endDetectDispense() {
  auto dispensedWeight = events().last().grams;

  // log the dispensed weight
  ostringstream ss;
  ss << fixed << setprecision(3) << dispensedWeight;
  logs.logEvent("DispensedWeight: " + QString::fromStdString(ss.str()) + " grams");

  // if it's not enough, there may have been a mechanical issue
  // -> dispense again
  if (needsRepeat()) {
    dispense(Mode::Repeat);
  } else {
    timerAllowManualDispense.start();
  }
}

void LogicImpl::update(optional<double> weightTarred,  //
                       bool isWeightBelowThreshold,    //
                       bool& justAte) {
  auto now = QDateTime::currentDateTime();

  auto timeOfDispense = optional<QDateTime>{};
  if (events().empty() == false) {
    timeOfDispense = events().last().timeDispensed;

    if (timeOfDispense < startTime) {
      // we detected a dispense event from a previous run, in the logs
      // do not consider it for computing time to dispense
      // because we start from zero on boot
      timeOfDispense = {};
    }
  }

  // compute time to dispense
  if (timeOfDispense.has_value() == false) {
    // never dispensed anything
    timeToDispenseSeconds = delaySeconds - startTime.secsTo(now);
  } else if (events().last().timeEaten.has_value() == false) {
    // dispensed something, it is not eaten
    timeToDispenseSeconds = {};
  } else {
    // dispensed something, it was eaten
    timeToDispenseSeconds = delaySeconds - events().last().timeEaten.value().secsTo(now);
  }

  if (timeOfDispense.has_value()) {
    // time since dispense

    auto& lastEvent = events().last();

    // dispensed weight
    if (timerDetectDispensed.isActive() && weightTarred.has_value()) {
      auto& dispensedWeight = lastEvent.grams;
      dispensedWeight = std::max(weightTarred.value(), dispensedWeight);
    }

    // just ate ?
    // first detect needs-repeat, then detect just-ate, because it's the same test
    if (lastEvent.timeEaten.has_value() == false &&  // not yet detected just-ate
        timerDetectDispensed.isActive() == false &&  // not still detecting dispensed weight
        needsRepeat() == false &&                    // not still needing repeat dispense
        isWeightBelowThreshold) {                    //
      // yes
      lastEvent.timeEaten = now;
      logs.logEvent(now.toString() + ", eat");
      justAte = true;
    }
  }

  // dispense if it is time
  // do not check that weight is below threshold
  // because when it is time, dispensed has been eaten
  if (timeToDispenseSeconds.value_or(1) <= 0) {
    dispense(Mode::Automatic);
  }

  logs.update(now);
}
