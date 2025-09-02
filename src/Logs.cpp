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
      DispenseRepeat,
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
    Type::DispenseAuto,
    Type::DispenseManual,
    Type::DispenseRepeat,
};

struct LogsImpl {
  QDir logDirectory;
  QFile logFile;
  QMap<qint64, QList<Data>> data;

  LogsImpl();
  void updateLogFile();
  void logEvent(QString const& event);
  void readHistoricalData(const QDate& date);

  QString dateToFilePath(const QDate& date);
  static QString dateToFileName(const QDate& date);
  static optional<QDate> fileNameToDate(const QString& fileName);
};

namespace {
  LogsImpl* impl = nullptr;
}

Logs::Logs() {
  AssertSingleton();
  impl = new LogsImpl;
}

void Logs::logEvent(QString const& event) { impl->logEvent(event); }

LogsImpl::LogsImpl() {
  logDirectory =
      QStandardPaths::standardLocations(QStandardPaths::StandardLocation::AppLocalDataLocation)
          .first() +
      "/logs";
  std::filesystem::create_directories(logDirectory.path().toStdString());
  cout << "Logs: " << logDirectory.path().toStdString() << "/" << endl;

  auto today = QDate::currentDate();
  auto yesterday = today.addDays(-1);
  readHistoricalData(yesterday);
  readHistoricalData(today);

#warning "TODO here"
  // populate logic->events with relevant data: today or less than 24 hours old
  // handle repeat dispenses: because they were not tarred, they must not be accumulated
  // log more precise weights
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
  // cout << log.toStdString() << endl;

  // print log event to file
  updateLogFile();
  logFile.open(QIODeviceBase::WriteOnly | QIODeviceBase::Append);
  logFile.write((event + "\n").toUtf8());
  logFile.close();

  // print log event to console
  cout << event.toStdString() << endl;
}

void LogsImpl::readHistoricalData(const QDate& date) {
  auto f = QFile{dateToFilePath(date)};
  if (f.exists() == false) {
    return;
  }

  auto& d = data[date.toJulianDay()];

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
    parseSimpleLine(", dispense, Manual", Type::DispenseManual);
    parseSimpleLine(", dispense, Automatic", Type::DispenseAuto);
    parseSimpleLine(", dispense, Repeat", Type::DispenseRepeat);

    static auto const patternWeight = "DispensedWeight: ";
    if (line.contains(patternWeight)) {
      line.remove(patternWeight);
      line.remove(" grams");
      auto ok = false;
      auto weight = line.toDouble(&ok);
      if (ok) {
        // qDebug() << patternWeight << value;
        // find last dispense item
        for (auto it = d.rbegin(); it != d.rend(); ++it) {
          if (Data::dispenseTypes.contains(it->type)) {
            // qDebug() << "latest dispense" << (int)it->type << it->time.toString();
            it->weight = weight;
            break;
          }
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
