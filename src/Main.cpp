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
#include "FilterWeight.hpp"
#include "LoadCell.hpp"
#include "Logic.hpp"
#include "Logs.hpp"
#include "LogsSmallWidget.hpp"
#include "LogsWidget.hpp"
#include "Mail.hpp"
#include "MainScreen.hpp"
#include "MainWindow.hpp"
#include "Menu.hpp"
#include "OnlyOneInstance.hpp"
#include "Plots.hpp"
#include "ProtectedButton.hpp"
#include "ScreenBrightness.hpp"
#include "SubScreen.hpp"
#include "ToolBar.hpp"
#include "Weight.hpp"
#include "Widget.hpp"

using namespace std;

namespace {
  void cleanup() {
    Logic::closeRelay();
    ScreenBrightness::reset();
  }
}

struct Application : public QApplication {
  Application(int& argc, char** argv) : QApplication(argc, argv) {
    // app name is needed for AppData location, consumed by Logs for logging location
    // otherwise it is derived from file name, which may be a varying symlink
    setApplicationName("CatPi");
  }
};

int main(int argc, char** argv) {
  CrashHandler::instance->cleanUpCallback = [] { cleanup(); };
  CrashHandler::instance->reportCallback = [](const std::string& error, const string& stack) {
    CrashDialog::ShowStackTrace(QString::fromStdString(error), QString::fromStdString(stack));
  };

  auto app = new Application(argc, argv);
  auto onlyone = OnlyOneInstance{};
  auto brightness = ScreenBrightness{};
  auto filterweight = FilterWeight{};
  auto logs = Logs{};
  auto mail = Mail{logs};
  auto loadcell = new LoadCell;
  auto weight = new Weight;
  auto calibration = new Calibration;
  auto logic = new Logic{logs, weight->weightThresholdGrams()};
  auto delay = new Delay;
  auto plots = new Plots;
  auto toolbar = new ToolBar;
  auto debug = new Debug;
  auto logsSmallWidget = new LogsSmallWidget;
  auto logsWidget = new LogsWidget{logs};
  auto menu = new Menu{delay};
  auto mainscreen = new MainScreen{weight, logsSmallWidget, delay->textAndButtons, delay->progress};
  auto central = new CentralWidget{{new SubScreen("", mainscreen),              //
                                    new SubScreen("", menu),                    //
                                    new SubScreen("", logsWidget),              //
                                    new SubScreen("Calibration", calibration),  //
                                    new SubScreen("Debug", debug)}};
  auto window = new MainWindow{central, toolbar};

  // Initialize globals
  {
    app->setStyleSheet(Widget::StyleSheetFontSize(48));

    Logic::hasGPIO = loadcell->hasGPIO();

    // update Delay so that it's displayed on startup, no wait for first tick
    delay->setRemaining(logic->timeToDispenseSeconds());

    brightness.setCallbackOnChange([window](bool isOn) {
      if (isOn == false) {
        // if we're going to screen saver
        // then disable all the widgets
        // because we must not be able to interact with widgets
        // when the screen is off
        window->setEnabled(false);
      } else {
        // if we just woke up from screen saver
        // wait a bit before enabling all the widgets
        // so that they do not react on the event that woke us up
        // because touching/clicking a screen that is off must not handle the event
        QTimer::singleShot(250, [window] { window->setEnabled(true); });
      }
    });
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
    QObject::connect(toolbar->menu, &QAction::triggered, [&](bool checked) {
      if (checked) {
        delay->attachWidgets();
      } else {
        mainscreen->attachWidgets();
      }
    });
    QObject::connect(toolbar->logs, &QAction::triggered, [&](bool checked) {
      if (checked) {
        logsWidget->loadData();
      }
    });
    auto const toolBarSubMenus = QList<pair<QAction*, QWidget*>>{
        {toolbar->menu, menu},
        {toolbar->logs, logsWidget},
    };
    for (auto const& i : toolBarSubMenus) {
      QObject::connect(i.first, &QAction::triggered, [=](bool checked) {
        if (checked) {
          central->setPage(i.second);
          for (auto& otherMenu : toolBarSubMenus) {
            if (otherMenu.first != i.first) {
              otherMenu.first->setChecked(false);
            }
          }
        } else {
          central->setPage(mainscreen);
        }
      });
    }

    // Menu
    QObject::connect(menu->calibration, &QAbstractButton::released,
                     [&]() { central->setPage(calibration); });
    QObject::connect(menu->debug, &QAbstractButton::released, [&] { central->setPage(debug); });

    // LoadCell
    QObject::connect(loadcell->timer, &QTimer::timeout, [&]() {
      if (auto data = loadcell->read()) {
        weight->update(filterweight.update(data.value().value));
        calibration->update(data.value().reading);
        static auto doFirstTare = true;
        if (filterweight.isInitializingFinished() && doFirstTare) {
          doFirstTare = false;
          weight->doTare();
          weight->setEnabled(true);
        }
      } else {
        weight->update(filterweight.value());
        calibration->update({});
      }
    });

    // Weight
    QObject::connect(weight->eventTareFinished(), &QTimer::timeout,
                     [&] { central->statusMessage(weight->messageFinished); });

    // SubScreens
    SubScreen::connect([&] {
      if (menu->isVisible()) {
        central->setPage(mainscreen);
      } else {
        central->setPage(menu);
      }
    });

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

    // Manual Dispense
    QObject::connect(mainscreen->dispenseButton->finished, &QTimer::timeout,
                     [&]() { logic->manualDispense(); });

    // Logic
    logic->connect({.updateGuiDelay = [&](int newDelay) { delay->setDelay(newDelay); },
                    .onDispense =
                        [&](bool doTare) {
                          mainscreen->dispenseButton->setEnabled(false);
                          if (doTare) {
                            weight->doTare();
                          }
                        }});

    QObject::connect(logic->timerAllowManualDispense, &QTimer::timeout,
                     [&] { mainscreen->dispenseButton->setEnabled(true); });
    QObject::connect(logic->timerUpdate, &QTimer::timeout, [&] {
      auto justAte = false;
      auto detectingDispensedWeight = false;

      logic->update(weight->weightTarred(), weight->isBelowThreshold(), justAte,
                    detectingDispensedWeight);

      if (justAte) {
        logs.logEvent(weight->toString());
        // } else if (detectingDispensedWeight) {
        // logs.logEvent(weight->toString());
      }

      delay->setRemaining(logic->timeToDispenseSeconds());
      logsSmallWidget->update(logs.events.data);
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

    // Plots
    plots->connect(window);
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
