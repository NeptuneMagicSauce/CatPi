#include <QAbstractButton>
#include <QApplication>
#include <QShortcut>
// #include <iostream>

#include "Calibration.hpp"
#include "CentralWidget.hpp"
#include "CrashDialog.hpp"
#include "CrashHandler.hpp"
#include "CrashTester.hpp"
#include "Debug.hpp"
#include "Delay.hpp"
#include "DeltaDial.hpp"
#include "LoadCell.hpp"
#include "Logic.hpp"
#include "Logs.hpp"
#include "MainScreen.hpp"
#include "MainWindow.hpp"
#include "OnlyOneInstance.hpp"
#include "ProtectedButton.hpp"
#include "ScreenBrightness.hpp"
#include "SubScreen.hpp"
#include "ToolBar.hpp"
#include "Weight.hpp"

using namespace std;

namespace {
  void cleanup() {
    Logic::closeRelay();
    ScreenBrightness::reset();
  }
}

int main(int argc, char** argv) {
  CrashHandler::instance->cleanUpCallback = [] { cleanup(); };
  CrashHandler::instance->reportCallback = [](const std::string& error, const string& stack) {
    CrashDialog::ShowStackTrace(QString::fromStdString(error), QString::fromStdString(stack));
  };

  auto app = new QApplication(argc, argv);
  auto onlyone = OnlyOneInstance{};
  auto brightness = ScreenBrightness{};
  auto loadcell = new LoadCell;
  auto weight = new Weight;
  auto calibration = new Calibration;
  auto logic = new Logic;
  auto delay = new Delay;
  auto logs = new Logs;
  auto toolbar = new ToolBar;
  auto debug = new Debug;
  auto mainscreen = new MainScreen{weight, delay};
  auto central = new CentralWidget{{new SubScreen("", mainscreen),
                                    new SubScreen("Calibration", calibration),  //
                                    new SubScreen("Debug", debug)}};
  auto window = new MainWindow{central, toolbar};

  // Initialize globals
  {
    app->setStyleSheet("QWidget{font-size: 48pt;} ");

    Logic::hasGPIO = loadcell->hasGPIO();

    // update Delay so that it's displayed on startup, no wait for first tick
    delay->setRemaining(logic->timeToDispense());

    window->show();  // must be after window is  finished constructing and after setStyleSheet
  }

  // Connect Signals
  {
    // Shortcuts
    auto quitShortcut = new QShortcut(QKeySequence(Qt::Key_F12), window);
    quitShortcut->setContext(Qt::ApplicationShortcut);
    QObject::connect(quitShortcut, &QShortcut::activated, app, &QApplication::quit);
    auto fullscreenShortcut = new QShortcut(QKeySequence(Qt::Key_F11), window);
    fullscreenShortcut->setContext(Qt::ApplicationShortcut);
    QObject::connect(fullscreenShortcut, &QShortcut::activated, toolbar->fullscreen,
                     &QAction::trigger);

    // ToolBar
    QObject::connect(toolbar->quit, &QAction::triggered, app, &QApplication::quit);
    QObject::connect(toolbar->fullscreen, &QAction::triggered, [&] {
      auto isFullscreen = false;
      window->toggleFullscreen(isFullscreen);
      toolbar->fullscreen->setIcon(ToolBar::fullScreenIcon(isFullscreen));
    });
    QObject::connect(toolbar->calibration, &QAction::triggered,
                     [&]() { central->setPage(calibration); });
    QObject::connect(toolbar->debug, &QAction::triggered, [&] { central->setPage(debug); });

    // Dispense
    QObject::connect(mainscreen->dispenseButton->finished, &QTimer::timeout, [&]() {
      mainscreen->dispenseButton->setEnabled(false);
      logic->manualDispense();
    });

    // LoadCell
    QObject::connect(loadcell->timer, &QTimer::timeout, [&]() {
      if (auto data = loadcell->read()) {
        weight->update(data.value().value);
        calibration->update(data.value().reading);
      } else {
        weight->update({});
        calibration->update({});
      }
    });

    // Weight
    QObject::connect(weight->eventTareFinished(), &QTimer::timeout,
                     [&] { central->statusMessage(weight->messageFinished); });

    // SubScreens
    SubScreen::connect([&] { central->setPage(mainscreen); });

    // Calibration
    QObject::connect(calibration->buttons.step1, &QAbstractButton::released,
                     [&] { calibration->callbacks.step1(loadcell->readPreciseRaw()); });
    QObject::connect(calibration->buttons.step2, &QAbstractButton::released, [&] {
      QString status;
      if (auto readingKnown = loadcell->readPreciseRaw()) {
        if (auto calibrationData = calibration->callbacks.step2(readingKnown.value())) {
          loadcell->recalibrate(calibrationData.value(), status);
        }
      } else {
        status = "Failed to read the weight";
      }
      central->setPage(mainscreen);
      central->statusMessage(status);
    });
    calibration->connect();

    // Logic
    logic->connect([&](int newDelay) { delay->setDelay(newDelay); });
    QObject::connect(logic->timerEndDispense, &QTimer::timeout,
                     [&] { mainscreen->dispenseButton->setEnabled(true); });
    QObject::connect(logic->timerUpdate, &QTimer::timeout, [&] {
      delay->setRemaining(logic->timeToDispense());

      auto tare = weight->tare();
      auto weightTarred = weight->weightTarred();
      auto dispensed = false;
      logic->update(weightTarred, tare, dispensed);
      if (dispensed) {
        mainscreen->dispenseButton->setEnabled(false);
      }
    });

    // Delay
    QObject::connect(delay->delayDial, &DeltaDial::valueChanged, [&] {
      logic->changeDelay(delay->delayDial->delta);
      delay->setDelay(logic->delaySeconds());
    });
    QObject::connect(delay->buttonDay, &QAbstractButton::released, [&] {
      logic->setDelaySeconds(delay->delayDaySeconds * 60);
      delay->setDelay(logic->delaySeconds());
    });
    QObject::connect(delay->buttonNight, &QAbstractButton::released, [&] {
      logic->setDelaySeconds(delay->delayNightSeconds * 60);
      delay->setDelay(logic->delaySeconds());
    });

    // Debug
    debug->connect([&] { central->setPage(debug); },
                   [&](QWidget* item) { central->setSettingPage(item); });

    // CrashTester
    auto crashTester = new CrashTester;
    auto crashShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F1), window);
    crashShortcut->setContext(Qt::ApplicationShortcut);
    QObject::connect(crashShortcut, &QShortcut::activated, app, [&]() { crashTester->show(); });

    // Logs
    logs->connect(window);
  }

  // Initializing that needs the signals connected
  {
    if (window->isSmallScreen) {
      toolbar->fullscreen->trigger();
    } else {
      window->setAutomaticSize();
    }
  }

  auto ret = app->exec();

  // Clean Up
  cleanup();

  return ret;
}
