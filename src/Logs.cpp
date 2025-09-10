#include "Logs.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSet>
#include <iostream>
#include <map>
#include <optional>

#include "Compressor.hpp"
#include "System.hpp"

using namespace std;

using Type = Logs::Event::Type;

const QSet<Type> Logs::Event::dispenseTypes = {
    Type::DispenseAuto,  //
    Type::DispenseManual,
};

struct LogsImpl {
  Events& events;

  QDir logDirectory;
  QFile logFile;
  struct Data {
    [[nodiscard]] auto const& at(const auto& day);
    [[nodiscard]] auto mustLoad(const auto& day) const;
    void insert(const auto& events, const auto& day, const auto& today);
    void purge(const auto& day);

   private:
    map<qint64, QList<Logs::Event>> events;
    map<qint64, bool> isFinal;
  } data;

  LogsImpl(Events& events, QString const& dataDirectory);
  void updateLogFile(bool& logFileChanged);
  void logEvent(QString const& event, bool& logFileChanged);
  [[nodiscard]] bool hasHistoricalData(QDate const& day) const;
  void readHistoricalData(auto const& day, auto const& today);
  optional<Logs::Event*> findLastDispense(auto& day);
  void populateHistoricalEvents(auto const& day, auto const& now);

  QString dateToFilePath(const QDate& date) const;
  static QString dateToFileName(const QDate& date);
  static optional<QDate> fileNameToDate(const QString& fileName);

  static bool eventIsLessThan24HoursOld(const QDateTime& event, const QDateTime& now);
};

namespace {
  LogsImpl* impl = nullptr;
}

Logs::Logs(QString const& dataDirectory) {
  AssertSingleton();
  eventLogFileChanged.setSingleShot(true);
  eventLogFileChanged.setInterval(0);
  impl = new LogsImpl{events, dataDirectory};
}

void Logs::logEvent(QString const& event) {
  auto logFileChanged = false;
  impl->logEvent(event, logFileChanged);
  if (logFileChanged) {
    eventLogFileChanged.start();
  }
}

auto const& LogsImpl::Data::at(const auto& day) {
  if (events.contains(day) == false) {
    insert(QList<Logs::Event>{}, day, QDate::currentDate().toJulianDay());
  }
  return events.at(day);
}

void LogsImpl::Data::insert(const auto& events, const auto& day, const auto& today) {
  // qDebug() << "Loaded" << QDate::fromJulianDay(day).toString() << "events" << events.size();
  this->events[day] = events;
  isFinal[day] = day < today;
}

void LogsImpl::Data::purge(const auto& day) {
  auto it = events.find(day);
  if (it != events.end()) {
    events.erase(it);
  }
}

auto LogsImpl::Data::mustLoad(const auto& day) const {
  if (events.contains(day) == false) {
    return true;
  }
  if (isFinal.at(day) == false) {
    return true;
  }
  return false;
}

LogsImpl::LogsImpl(Events& events, QString const& dataDirectory) : events(events) {
  logDirectory = dataDirectory + "/logs";

  std::filesystem::create_directories(logDirectory.path().toStdString());
  cout << "Logs: " << logDirectory.path().toStdString() << "/" << endl;

  auto now = QDateTime::currentDateTime();
  auto today = now.date();
  auto yesterday = today.addDays(-1);

  for (auto const& dayToLoad : QList{yesterday, today}) {
    readHistoricalData(dayToLoad, today);
    populateHistoricalEvents(dayToLoad, now);
  }

  // qDebug() << "DATA:";
  // for (auto& d : data[today.toJulianDay()]) {
  //   qDebug() << (int)d.type << d.time << d.weight;
  // }
  // qDebug() << "EVENTS:";
  // for (auto& e : events.data) {
  //   qDebug() << e.grams << "GRAMS" << e.timeDispensed.toString()
  //            << e.timeEaten.value_or(QDateTime{}).toString();
  // }
}

void LogsImpl::populateHistoricalEvents(auto const& day, auto const& now) {
  for (auto const& historical : data.at(day.toJulianDay())) {
    if (eventIsLessThan24HoursOld(historical.time, now)) {
      if (Logs::Event::dispenseTypes.contains(historical.type)) {
        events.data.append({//
                            .timeDispensed = historical.time,
                            .grams = historical.weight,
                            .timeEaten = {}});
      } else if (historical.type == Type::Eat) {
        if (events.data.empty() == false) {
          events.data.last().timeEaten = historical.time;
        }
      }
    }
  }
}

optional<Logs::Event*> LogsImpl::findLastDispense(auto& day) {
  for (auto it = day.rbegin(); it != day.rend(); ++it) {
    if (Logs::Event::dispenseTypes.contains(it->type)) {
      return &(*it);
    }
  }
  return {};
}

QString LogsImpl::dateToFileName(const QDate& date) { return date.toString(Qt::ISODate) + ".txt"; }
QString Logs::dateToFilePath(const QDate& date) const { return impl->dateToFilePath(date); }
QString LogsImpl::dateToFilePath(const QDate& date) const {
  return logDirectory.path() + "/" + dateToFileName(date);
}
optional<QDate> LogsImpl::fileNameToDate(const QString& fileName) {
  auto parseInt = [](const auto& str, auto& value) {
    auto ok = false;
    value = str.toInt(&ok);
    return ok;
  };
  auto f = fileName;
  f.remove(".txt");
  auto splitted = f.split("-");
  int year = 0, month = 0, day = 0;
  if (splitted.size() == 3 &&          //
      parseInt(splitted[0], year) &&   //
      parseInt(splitted[1], month) &&  //
      parseInt(splitted[2], day)) {
    return QDate{year, month, day};
  }
  qDebug() << "bad log file name" << fileName;
  return {};
}

void LogsImpl::updateLogFile(bool& logFileChanged) {
  auto const newPath = dateToFilePath(QDate::currentDate());
  auto const oldPath = logFile.fileName();
  if (newPath != oldPath) {
    logFileChanged = true;
    if (oldPath.isEmpty() == false) {
      logFile.close();
    }
    logFile.setFileName(newPath);
    logFile.open(QIODeviceBase::WriteOnly | QIODeviceBase::Append);
  }
}

void LogsImpl::logEvent(QString const& event, bool& logFileChanged) {
  // print log event to file
  updateLogFile(logFileChanged);
  logFile.write((event + "\n").toUtf8());
  logFile.flush();

  // print log event to console
  cout << event.toStdString() << endl;
}

QList<Logs::Event> const& Logs::readHistoricalData(QDate const& day) const {
  impl->readHistoricalData(day, QDate::currentDate());
  return impl->data.at(day.toJulianDay());
}

bool Logs::hasHistoricalData(QDate const& day) const { return impl->hasHistoricalData(day); }
bool LogsImpl::hasHistoricalData(QDate const& day) const {
  return Compressor::Exists(dateToFilePath(day));
}

void LogsImpl::readHistoricalData(auto const& day, auto const& today) {
  auto dayIndex = day.toJulianDay();

  if (hasHistoricalData(day) == false) {
    data.purge(dayIndex);
    return;
  }
  if (data.mustLoad(dayIndex) == false) {
    // we do not reload historical data, it has not changed
    return;
  }

  auto dayEvents = QList<Logs::Event>{};

  // === boot === Tue Sep 2 23:01:02 2025
  // Tue Sep 2 23:25:47 2025, dispense, Automatic
  // Tue Sep 2 22:13:42 2025, dispense, Manual
  // Tue Sep 2 22:13:49 2025, dispense, Repeat
  // DispensedWeight: 1.234 grams
  // Tue Sep 2 22:14:03 2025, eat
  // Tue Aug 26 00:31:08 2025, recent Weights {MilliSecsAgo, Grams}, {146,0.35}, {358,0.34}, (...)

  auto path = dateToFilePath(day);
  for (auto const& l : Compressor::Read(path).split('\n')) {
    auto line = QString{l};

    auto parseSimpleLine = [&](const auto& pattern, const auto type) {
      // TODO use QRegularExpression
      if (line.contains(pattern)) {
        // qDebug() << line;
        line.remove(pattern);
        auto time = QDateTime::fromString(line);
        if (time.isValid() == false) {
          qDebug() << "invalid line with time" << l;
          return;
        }
        dayEvents.append({
            .type = type,
            .time = time,
            .weight = 0.0,
        });
        // qDebug() << (int)d.last().type << d.last().time.toString();
      }
    };
    parseSimpleLine("=== boot === ", Type::Boot);
    parseSimpleLine(", eat", Type::Eat);
    parseSimpleLine(", dispense, manual", Type::DispenseManual);
    parseSimpleLine(", dispense, automatic", Type::DispenseAuto);
    // do not create an event for repeat dispenses
    // because the dispensed weight is owned by only one dispense event
    // the manual or auto dispense event, not the repeated
    // TODO log in dispense event if it was repeated

    // set the weight to the eat event from previous dispense
    if (dayEvents.empty() == false) {
      auto& lastEvent = dayEvents.last();
      if (lastEvent.type == Type::Eat) {
        if (auto lastDispense = findLastDispense(dayEvents)) {
          lastEvent.weight = lastDispense.value()->weight;
        }
      }
    }

    static auto const patternWeight = "DispensedWeight: ";
    if (line.contains(patternWeight)) {
      line.remove(patternWeight);
      line.remove(" grams");
      auto ok = false;
      auto weight = line.toDouble(&ok);
      if (ok) {
        // qDebug() << patternWeight << value;
        if (auto lastDispense = findLastDispense(dayEvents)) {
          // qDebug() << "last dispense" << (int)lastDispense->type << it->time.toString();
          lastDispense.value()->weight = weight;
        }
      } else {
        qDebug() << "invalid weight in line" << l;
      }
    }
  }

  data.insert(dayEvents, dayIndex, today.toJulianDay());

  // // debug
  // for (const auto& e : d) {
  //   qDebug() << (int)e.type << e.time.toString()
  //            << (Data::dispenseTypes.contains(e.type) ? QString::number(e.weight) : "");
  // }
}

bool LogsImpl::eventIsLessThan24HoursOld(const QDateTime& event, const QDateTime& now) {
  return event.secsTo(now) < 60 * 60 * 24;
}

void Logs::update(const QDateTime& now) {
  // remove events older than 24 hours
  while (events.data.isEmpty() == false &&
         LogsImpl::eventIsLessThan24HoursOld(events.data.first().timeDispensed, now) == false) {
    events.data.removeFirst();
  }
}
