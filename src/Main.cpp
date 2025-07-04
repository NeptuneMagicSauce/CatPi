#include <QAbstractButton>
#include <QApplication>
#include <QShortcut>
// #include <iostream>

#include "Calibration.hpp"
#include "CentralWidget.hpp"
#include "Debug.hpp"
#include "Delay.hpp"
#include "DeltaDial.hpp"
#include "LoadCell.hpp"
#include "Logic.hpp"
#include "MainScreen.hpp"
#include "MainWindow.hpp"
#include "SubScreen.hpp"
#include "ToolBar.hpp"
#include "Weight.hpp"

using namespace std;

int main(int argc, char** argv) {
  QApplication* app = new QApplication(argc, argv);
  LoadCell* loadcell = new LoadCell;
  Weight* weight = new Weight;
  Calibration* calibration = new Calibration;
  Logic* logic = new Logic;
  Delay* delay = new Delay;
  ToolBar* toolbar = new ToolBar;
  Debug* debug = new Debug;
  MainScreen* mainscreen = new MainScreen{weight, delay};
  CentralWidget* central =
      new CentralWidget{{new SubScreen("", mainscreen), new SubScreen("Calibration", calibration),
                         new SubScreen("Debug", debug)}};
  MainWindow* window = new MainWindow{central, toolbar};

  // Initialize globals
  {
    app->setStyleSheet("QWidget{font-size: 48pt;} ");

    logic->hasGPIO = loadcell->hasGPIO();

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
    QObject::connect(fullscreenShortcut, &QShortcut::activated, toolbar->fullscreen, &QAction::trigger);

    // ToolBar
    QObject::connect(toolbar->quit, &QAction::triggered, app, &QApplication::quit);
    QObject::connect(toolbar->fullscreen, &QAction::triggered, [&] {
      auto isFullscreen = false;
      window->toggleFullscreen(isFullscreen);
      toolbar->fullscreen->setIcon(ToolBar::fullScreenIcon(isFullscreen));
    });
    QObject::connect(toolbar->calibration, &QAction::triggered, [&]() { central->setPage(calibration); });
    QObject::connect(toolbar->debug, &QAction::triggered, [&] { central->setPage(debug); });

    // Dispense
    QObject::connect(mainscreen->dispenseButton, &QAbstractButton::released, [&]() {
      mainscreen->dispenseButton->setEnabled(false);
      logic->manualDispense();
    });

    // LoadCell
    QObject::connect(loadcell->timer, &QTimer::timeout, [&]() {
      auto dispensed = false;
      auto tare = weight->tare();
      auto weightTarred = 0.0;
      if (auto data = loadcell->read()) {
        weight->update(data.value().value, weightTarred);
        calibration->update(data.value().reading);
        logic->update(weightTarred, tare, dispensed);
        if (dispensed) {
          mainscreen->dispenseButton->setEnabled(false);
        }
      } else {
        weight->update({}, weightTarred);
        calibration->update({});
        logic->update({}, tare, dispensed);
      }

      delay->setRemaining(logic->timeToDispense());
    });

    // Weight
    weight->connect();
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
    QObject::connect(logic->timerEndDispense(), &QTimer::timeout,
                     [&] { mainscreen->dispenseButton->setEnabled(true); });

    // Delay
    delay->connect();
    QObject::connect(delay->delayDial, &DeltaDial::valueChanged, [&] {
      logic->changeDelay(delay->delayDial->delta);
      delay->setDelay(logic->delaySeconds());
    });

    // Debug
    debug->connect([&] { central->setPage(debug); }, [&](QWidget* item) { central->setSettingPage(item); });
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
  logic->closeRelay();

  return ret;
}
