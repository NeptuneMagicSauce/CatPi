#include "Compressor.hpp"

#include <QFileInfo>
#include <QProcess>

namespace {
  static constexpr auto suffix = ".gz";
}

void Compressor::Do(QString const& pathSource, QString& pathDest) {
  if (QFileInfo{pathSource}.exists() == false) {
    return;
  }
  auto process = QProcess{};
  process.start("gzip", {pathSource});
  process.waitForFinished();
  if (process.exitStatus() == QProcess::NormalExit) {
    pathDest = pathSource + ::suffix;
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
      if (process.exitStatus() == QProcess::NormalExit) {
        return process.readAll();
      }
    }
  }
  return {};
}
