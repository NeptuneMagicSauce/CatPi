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
  auto isOn = true;

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

  Settings::load({"ScreenSaverDelay",
                  "Attente éteinte écran",
                  "Temps d'atteinte après lequel l'écran s'éteint",
                  "Minutes",
                  5,
                  [&](QVariant v) {
                    delayScreenSaverMinutes = v.toInt();  // how can this work with a DeltaDial !?
                  },
                  {1, 20}});
}

void ScreenBrightness::setIsOn(bool isOn) {
  if (::isOn == isOn) {
    return;
  }
  ::isOn = isOn;
  qDebug() << Q_FUNC_INFO << isOn;
  change(isOn ? Settings::get("ScreenBrightness").toInt() : 0);
}
