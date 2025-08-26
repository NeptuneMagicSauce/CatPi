#include "ScreenBrightness.hpp"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QTimer>

#include "Settings.hpp"
#include "System.hpp"

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
  int delayScreenSaverMinutes = 1;
  QTimer timerInactive;
  std::function<void(bool)> onChangeCallback = nullptr;

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
    if (onChangeCallback != nullptr) {
      onChangeCallback(isOn);
    }
    // qDebug() << Q_FUNC_INFO << isOn;
    change(isOn ? Settings::get("ScreenBrightness").toInt() : 0);
  }
}

ScreenBrightness::ScreenBrightness() {
  AssertSingleton();

  Settings::load({.key = "ScreenBrightness",
                  .name = "Luminosité de l'écran",
                  .prompt = "",
                  .unit = "Pourcentage",
                  .defaultValue = rangeMax,
                  .callback = [&](QVariant v) { change(v.toInt()); },
                  .limits = {.minimum = rangeMin, .maximum = rangeMax}});

  Settings::load({.key = "ScreenSaverDelay",
                  .name = "Attente éteinte écran",
                  .prompt = "Temps d'atteinte après lequel l'écran s'éteint",
                  .unit = "Minutes",
                  .defaultValue = 5,
                  .callback =
                      [&](QVariant v) {
                        delayScreenSaverMinutes = v.toInt();
                        timerInactive.setInterval(delayScreenSaverMinutes * 1000 * 60);
                        timerInactive.start();
                      },
                  .limits = {.minimum = 0, .maximum = 20}});

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

void ScreenBrightness::setCallbackOnChange(std::function<void(bool)> onChangeCallback) {
  ::onChangeCallback = onChangeCallback;
}
