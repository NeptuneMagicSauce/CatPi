#include "Compressor.hpp"

#include <QFileInfo>
#include <QProcess>
#include <iostream>

namespace {
  static constexpr auto suffix = ".gz";
  void printProcessOutputsToConsole(auto& process) {
    std::cerr << QString{process.readAllStandardError()}.toStdString();
    std::cout << QString{process.readAllStandardOutput()}.toStdString();
  }
}

void Compressor::Do(QString const& pathSource, QString& pathDest) {
  if (QFileInfo{pathSource}.exists() == false) {
    return;
  }
  auto process = QProcess{};
  process.start("gzip", {pathSource});
  process.waitForFinished();
  if (process.exitCode() == 0) {
    pathDest = pathSource + ::suffix;
  } else {
    printProcessOutputsToConsole(process);
  }
}

bool Compressor::Exists(QString const& path) {
  return QFileInfo{path}.exists() || QFileInfo{path + ::suffix}.exists();
}

QByteArray Compressor::Read(QString const& path) {
  for (auto compressed : QList<bool>{false, true}) {
    if (compressed == false) {
      auto file = QFile{path};
      if (file.exists() == false) {
        continue;
      }
      file.open(QIODeviceBase::ReadOnly);
      return file.readAll();
    } else {
      auto process = QProcess{};
      process.start("zcat", {path + ::suffix});
      process.waitForFinished();
      if (process.exitCode() == 0) {
        return process.readAll();
      } else {
        printProcessOutputsToConsole(process);
      }
    }
  }
  return {};
}
