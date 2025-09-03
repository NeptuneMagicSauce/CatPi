#include "Logs.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMap>
#include <QSet>
#include <QStandardPaths>
#include <iostream>
#include <optional>

#include "System.hpp"

using namespace std;

namespace {
  struct Data {
    enum struct Type {  //
      Boot,
      DispenseAuto,
      DispenseManual,
      Eat,
    };
    Type type;
    QDateTime time;
    double weight;

    static const QSet<Type> dispenseTypes;
  };
}

using Type = Data::Type;

const QSet<Type> Data::dispenseTypes = {
    Type::DispenseAuto,  //
    Type::DispenseManual,
};

struct LogsImpl {
  Events& events;

  QDir logDirectory;
  QFile logFile;
  QMap<qint64, QList<Data>> data;

  LogsImpl(Events& events);
  void updateLogFile();
  void logEvent(QString const& event);
  void readHistoricalData(auto const& day);
  optional<Data*> findLatestDispense(auto& day);
  void populateHistoricalEvents(auto const& day, auto const& now);

  QString dateToFilePath(const QDate& date);
  static QString dateToFileName(const QDate& date);
  static optional<QDate> fileNameToDate(const QString& fileName);

  static bool eventIsLessThan24HoursOld(const QDateTime& event, const QDateTime& now);
};

namespace {
  LogsImpl* impl = nullptr;
}

Logs::Logs() {
  AssertSingleton();
  impl = new LogsImpl{events};
}

void Logs::logEvent(QString const& event) { impl->logEvent(event); }

LogsImpl::LogsImpl(Events& events) : events(events) {
  logDirectory =
      QStandardPaths::standardLocations(QStandardPaths::StandardLocation::AppLocalDataLocation)
          .first() +
      "/logs";
  std::filesystem::create_directories(logDirectory.path().toStdString());
  cout << "Logs: " << logDirectory.path().toStdString() << "/" << endl;

  auto now = QDateTime::currentDateTime();
  auto today = now.date();
  auto yesterday = today.addDays(-1);

  for (auto const& dayToLoad : QList{yesterday, today}) {
    readHistoricalData(dayToLoad);
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
  for (auto const& historical : data[day.toJulianDay()]) {
    if (eventIsLessThan24HoursOld(historical.time, now)) {
      if (Data::dispenseTypes.contains(historical.type)) {
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

optional<Data*> LogsImpl::findLatestDispense(auto& day) {
  for (auto it = day.rbegin(); it != day.rend(); ++it) {
    if (Data::dispenseTypes.contains(it->type)) {
      return &(*it);
    }
  }
  return {};
}

QString LogsImpl::dateToFileName(const QDate& date) { return date.toString(Qt::ISODate) + ".txt"; }
QString LogsImpl::dateToFilePath(const QDate& date) {
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

void LogsImpl::updateLogFile() {
  auto const path = dateToFilePath(QDate::currentDate());
  if (logFile.fileName().isEmpty() == false && logFile.fileName() != path) {
    logFile.close();
  }
  logFile.setFileName(path);
}

void LogsImpl::logEvent(QString const& event) {
  // print log event to file
  updateLogFile();
  logFile.open(QIODeviceBase::WriteOnly | QIODeviceBase::Append);
  logFile.write((event + "\n").toUtf8());
  logFile.close();

  // print log event to console
  cout << event.toStdString() << endl;
}

void LogsImpl::readHistoricalData(auto const& day) {
  auto f = QFile{dateToFilePath(day)};
  if (f.exists() == false) {
    return;
  }

  auto& d = data[day.toJulianDay()];

  // === boot === Tue Sep 2 23:01:02 2025
  // Tue Sep 2 23:25:47 2025, dispense, Automatic
  // Tue Sep 2 22:13:42 2025, dispense, Manual
  // Tue Sep 2 22:13:49 2025, dispense, Repeat
  // DispensedWeight: 1.234 grams
  // Tue Sep 2 22:14:03 2025, eat
  // Tue Aug 26 00:31:08 2025, recent Weights {MilliSecsAgo, Grams}, {146,0.35}, {358,0.34}, (...)

  f.open(QIODeviceBase::ReadOnly);
  for (auto const& l : f.readAll().split('\n')) {
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
        d.append({
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

    static auto const patternWeight = "DispensedWeight: ";
    if (line.contains(patternWeight)) {
      line.remove(patternWeight);
      line.remove(" grams");
      auto ok = false;
      auto weight = line.toDouble(&ok);
      if (ok) {
        // qDebug() << patternWeight << value;
        if (auto lastDispense = findLatestDispense(d)) {
          // qDebug() << "last dispense" << (int)lastDispense->type << it->time.toString();
          lastDispense.value()->weight = weight;
        }
      } else {
        qDebug() << "invalid weight in line" << l;
      }
    }
  }

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
