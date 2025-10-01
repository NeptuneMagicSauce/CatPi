#include "ScreenBrightness.hpp"

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QTimer>
#include <QWidget>

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
  int delayScreenSaverMinutes = 1;
  QTimer timerLoop;
  QElapsedTimer timerSinceInteraction;
  function<void(bool)> onChangeCallback;

  auto settingToValue(int setting) { return (setting * 255) / rangeMax; }

  int readBrightness() {
    if (procfile.exists() == false) {
      return 0;
    }
    procfile.open(QIODeviceBase::ReadOnly);
    auto ret = QString{procfile.readAll()}.toInt();
    // qDebug() << "BrightnessReading:" << ret;
    procfile.close();
    return ret;
  }

  void writeBrightness(int value) {
    if (procfile.exists() == false) {
      return;
    }
    procfile.open(QIODeviceBase::WriteOnly | QIODeviceBase::Append);
    procfile.write(QString::number(value).toUtf8());
    procfile.close();
  }
}

ScreenBrightness::ScreenBrightness() {
  AssertSingleton();

  Settings::load({.key = "ScreenBrightness",
                  .name = "Luminosité de l'écran",
                  .prompt = "",
                  .unit = "Pourcentage",
                  .defaultValue = rangeMax,
                  .callback = [&](QVariant v) { writeBrightness(settingToValue(v.toInt())); },
                  .limits = {.minimum = rangeMin, .maximum = rangeMax}});

  Settings::load({.key = "ScreenSaverDelay",
                  .name = "Attente éteinte écran",
                  .prompt = "Temps d'atteinte après lequel l'écran s'éteint",
                  .unit = "Minutes",
                  .defaultValue = 5,
                  .callback = [&](QVariant v) { delayScreenSaverMinutes = v.toInt(); },
                  .limits = {.minimum = 0, .maximum = 20}});

  timerSinceInteraction.start();

  timerLoop.setSingleShot(false);
  timerLoop.setInterval(500);
  timerLoop.start();
  QObject::connect(&timerLoop, &QTimer::timeout, [&] {
    auto brightnessValue = settingToValue(Settings::get("ScreenBrightness").toInt());
    auto newValue = [&] {
      auto activeWindow = qApp->activeWindow();
      if (activeWindow == nullptr || activeWindow->isFullScreen() == false) {
        // we are not the active window
        return brightnessValue;
      }
      if (delayScreenSaverMinutes == 0) {
        // the screen saver is disabled
        return brightnessValue;
      }
      auto timeSinceInteractionMinutes = timerSinceInteraction.elapsed() / 1000 / 60;
      if (timeSinceInteractionMinutes >= delayScreenSaverMinutes) {
        return 0;
      } else {
        return brightnessValue;
      }
    }();
    if (readBrightness() != newValue) {
      writeBrightness(newValue);
      if (onChangeCallback != nullptr) {
        onChangeCallback(newValue > 0);
      }
    }
  });

  // install watcher to detect activity
  struct Watcher : public QObject {
    bool eventFilter(QObject* dest, QEvent* event) override {
      auto type = event->type();
      if (type == QEvent::MouseMove || type == QEvent::MouseButtonPress) {
        timerSinceInteraction.restart();
      }
      return QObject::eventFilter(dest, event);
    }
  };
  assert(qApp != nullptr);
  qApp->installEventFilter(new Watcher);
}

void ScreenBrightness::reset() { writeBrightness(settingToValue(rangeMax)); }

void ScreenBrightness::setCallbackOnChange(std::function<void(bool)> onChangeCallback) {
  ::onChangeCallback = onChangeCallback;
}
