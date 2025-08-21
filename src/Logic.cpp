#include "Logic.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTimer>
#include <iostream>
// #include <vector>

#include "PinCtrl.hpp"
#include "Settings.hpp"
#include "System.hpp"

using namespace std;
using PinCtrl::pinctrl;

bool Logic::hasGPIO = false;

struct LogicImpl {
  LogicImpl();

  optional<QDateTime> previousDispense;
  bool dispensedIsEaten = false;
  QDir logDirectory;
  QFile logFile;
  const QDateTime startTime = QDateTime::currentDateTime();
  const QString delayKey = "Delay";
  const int durationDispenseRelayMilliseconds = 4000;
  int delaySeconds = 0;
  qint64 elapsed = 0;
  QList<Event> events;
  double dispensedWeight = 0.0;
  double weightThreshold = 0.4;

  enum struct DispenseMode { Automatic, Manual };

  void dispense(bool hasGPIO, QTimer* timerEndDispense, DispenseMode mode);
  void logEvent(QString const& event);

  std::function<void(int)> updateGuiCallback = nullptr;

  void updateLogFile();
};

using DispenseMode = LogicImpl::DispenseMode;

namespace {
  LogicImpl* impl = nullptr;
}

Logic::Logic() {
  AssertSingleton();
  impl = new LogicImpl;
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

void Logic::manualDispense() { impl->dispense(hasGPIO, timerEndDispense, DispenseMode::Manual); }

void LogicImpl::dispense(bool hasGPIO, QTimer* timerEndDispense, DispenseMode mode) {
  // std::cout << __PRETTY_FUNCTION__ << std::endl;
  if (hasGPIO) {
    pinctrl("set 17 op dh");
  }
  timerEndDispense->start();

  auto now = QDateTime::currentDateTime();

  previousDispense = now;
  dispensedIsEaten = false;
  dispensedWeight = 0;

  events.append({now, 0, {}});

  auto log = now.toString();
  log += ", dispense, ";
  log += mode == DispenseMode::Automatic ? "automatic" : "manual";
  logEvent(log);
}

void Logic::update(std::optional<double> weightTarred, double /*tare*/, bool& dispensed) {
  auto& previousDispense = impl->previousDispense;
  auto& events = impl->events;

  auto now = QDateTime::currentDateTime();

  auto weightBelowThreshold = weightTarred.has_value()
                                  ? (weightTarred.value() < impl->weightThreshold)
                                  : (hasGPIO ? false : true);
  auto start = previousDispense.value_or(impl->startTime);
  auto elapsedSeconds = start.secsTo(now);
  auto timeAboveThreshold = elapsedSeconds > impl->delaySeconds;
  impl->elapsed = elapsedSeconds;

  if (previousDispense.has_value() && elapsedSeconds < 10 && weightTarred.has_value()) {
    // compute and store the dispensed weight
    impl->dispensedWeight = std::max(weightTarred.value(), impl->dispensedWeight);
    if (events.empty() == false) {  // must be true
      auto& event = events[events.size() - 1];
      event.grams = impl->dispensedWeight;
    }
  }

  auto justAte = previousDispense.has_value() &&                                             //
                 impl->dispensedIsEaten == false &&                                          //
                 elapsedSeconds > (impl->durationDispenseRelayMilliseconds / 1000) + 2.0 &&  //
                 weightBelowThreshold == true;
  if (justAte) {
    impl->dispensedIsEaten = true;
    if (events.empty() == false) {  // must be true
      auto& event = events[events.size() - 1];
      event.timeEaten = now;
      impl->logEvent(now.toString() + ", eat");
    }
  }

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
  // impl->logEvent(log);

  if (timeAboveThreshold && weightBelowThreshold) {
    impl->dispense(hasGPIO, timerEndDispense, DispenseMode::Automatic);
    dispensed = true;
  }

  // remove events older than 24 hours
  while (events.isEmpty() == false && events.first().timeDispensed.secsTo(now) > 60 * 60 * 24) {
    events.removeFirst();
  }
}

void LogicImpl::logEvent(QString const& event) {
  // cout << log.toStdString() << endl;
  updateLogFile();
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

const QList<Event>& Logic::events() const { return impl->events; }
