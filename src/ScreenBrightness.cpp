#include "ScreenBrightness.hpp"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QTimer>

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

  auto const rangeMin = 1;
  auto const rangeMax = 100;
  auto procfile = QFile{findProcFilePath()};
  auto isOn = true;
  int delayScreenSaverMinutes;
  QTimer timerInactive;

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

  void setIsOn(bool isOn) {
    if (::isOn == isOn) {
      return;
    }
    ::isOn = isOn;
    // qDebug() << Q_FUNC_INFO << isOn;
    change(isOn ? Settings::get("ScreenBrightness").toInt() : 0);
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
                    delayScreenSaverMinutes = v.toInt();
                    timerInactive.setInterval(delayScreenSaverMinutes * 1000 * 60);
                    timerInactive.start();
                  },
                  {0, 20}});

  // install watcher to detect activity
  timerInactive.setSingleShot(true);
  QObject::connect(&timerInactive, &QTimer::timeout, [&] {
    if (delayScreenSaverMinutes == 0) {
      // that's a disabled screen saver
      return;
    }
    setIsOn(false);
  });
  struct Watcher : public QObject {
    bool eventFilter(QObject* dest, QEvent* event) override {
      auto type = event->type();
      if (type == QEvent::MouseMove || type == QEvent::MouseButtonPress) {
        setIsOn(true);
        timerInactive.start();
      }
      return QObject::eventFilter(dest, event);
    }
  };
  assert(qApp != nullptr);
  qApp->installEventFilter(new Watcher);
}

void ScreenBrightness::reset() { change(rangeMax); }
