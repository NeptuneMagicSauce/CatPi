#include "Logs.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <iostream>

#include "System.hpp"

using namespace std;

struct LogsImpl {
  QDir logDirectory;
  QFile logFile;

  LogsImpl();
  void updateLogFile();
  void logEvent(QString const& event);
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
}

void LogsImpl::updateLogFile() {
  auto const fileName = QDateTime::currentDateTime().date().toString(Qt::ISODate);
  auto const path = logDirectory.path() + "/" + fileName + ".txt";
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
