#include <QAbstractButton>
#include <QApplication>
#include <QLabel>
#include <QShortcut>
#include <QStatusBar>
#include <QTimer>
// #include <iostream>

#include "Calibration.hpp"
#include "CentralWidget.hpp"
#include "Delay.hpp"
#include "DeltaDial.hpp"
#include "LoadCell.hpp"
#include "Logic.hpp"
#include "MainWindow.hpp"
#include "SubScreen.hpp"
#include "ToolBar.hpp"
#include "Weight.hpp"

using namespace std;

struct Main {
  Main(int& argc, char** argv);
  void connectSignals();

  QApplication* app = nullptr;
  LoadCell* loadcell = nullptr;
  Weight* weight = nullptr;
  Calibration* calibration = nullptr;
  Logic* logic = nullptr;
  Delay* delay = nullptr;
  CentralWidget* central = nullptr;
  ToolBar* toolbar = nullptr;
  MainWindow* window = nullptr;
};

int main(int argc, char** argv) {
  auto main = Main{argc, argv};

  auto ret = main.app->exec();

  main.logic->reset();

  return ret;
}

Main::Main(int& argc, char** argv)
    : app(new QApplication(argc, argv)),
      loadcell(new LoadCell),
      weight(new Weight(loadcell)),
      calibration(new Calibration),
      logic(new Logic(loadcell->hasGPIO())),
      delay(new Delay(logic->delaySeconds())),
      central(new CentralWidget(weight, new SubScreen("Calibration", calibration), delay)),
      toolbar(new ToolBar),
      window(new MainWindow(central, toolbar)) {
  app->setStyleSheet("QWidget{font-size: 48pt;} ");

  window->show();    // must be after window is  finished constructing and after setStyleSheet
  connectSignals();  // must be after all members are constructed

  if (window->isSmallScreen) {
    toolbar->fullscreen->trigger();
  } else {
    window->setAutomaticSize();
  }
}

void Main::connectSignals() {
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
    window->toggleFullscreen();
    toolbar->fullscreen->setIcon(ToolBar::fullScreenIcon(window->isFullscreen()));
  });
  QObject::connect(toolbar->calibration, &QAction::triggered,
                   [&]() { central->setPage(CentralWidget::Page::Calibration); });

  // Dispense
  QObject::connect(central->dispenseButton(), &QAbstractButton::released, [this]() {
    central->dispenseButton()->setEnabled(false);
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
        central->dispenseButton()->setEnabled(false);
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
  SubScreen::connect([&] { central->setPage(CentralWidget::Page::Main); });

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
    central->setPage(CentralWidget::Page::Main);
    central->statusMessage(status);
  });
  calibration->connect();

  // Logic
  logic->connect();
  QObject::connect(logic->timerEndDispense(), &QTimer::timeout,
                   [&] { central->dispenseButton()->setEnabled(true); });

  // Delay
  delay->connect();
  QObject::connect(delay->delayDial, &DeltaDial::valueChanged, [&] {
    logic->changeDelay(delay->delayDial->delta);
    delay->setDelay(logic->delaySeconds());
  });
}
