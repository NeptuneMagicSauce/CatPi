#include "ScreenBrightness.hpp"

#include <QDir>
#include <QFile>

#include "Settings.hpp"

using namespace std;

namespace {
  QString findProcFilePath() {
    auto dir = QDir{"/sys/class/backlight/"};
    auto files = dir.entryList({"*"}, QDir::Dirs | QDir::NoDotAndDotDot);
    if (files.isEmpty()) {
      return "";
    }
    return dir.path() + "/" + files.first() + "/brightness";
  }
  auto procfile = QFile{findProcFilePath()};

  auto const rangeMin = 1, rangeMax = 100;

  void change(int byte) {
    if (procfile.exists() == false) {
      return;
    }
    procfile.open(QIODeviceBase::WriteOnly | QIODeviceBase::Append);
    byte *= 255;
    byte /= rangeMax;
    procfile.write(QString::number(byte).toUtf8());
    procfile.close();
  }
}

ScreenBrightness::ScreenBrightness() {
  Settings::load({"ScreenBrightness",
                  "Luminosité de l'écran",
                  "",
                  "Pourcentage",
                  rangeMax,
                  [&](QVariant v) { change(v.toInt()); },
                  {rangeMin, rangeMax}});
}
