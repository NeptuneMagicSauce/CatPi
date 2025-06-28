#include <QAbstractButton>
#include <QApplication>
#include <QLabel>
#include <QShortcut>
#include <QStatusBar>
#include <QTimer>
// #include <iostream>

#include "Calibration.hpp"
#include "CentralWidget.hpp"
#include "LoadCell.hpp"
#include "Logic.hpp"
#include "MainWindow.hpp"
#include "Settings.hpp"
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
  CentralWidget* central = nullptr;
  ToolBar* toolbar = nullptr;
  MainWindow* window = nullptr;
  Logic* logic = nullptr;
};

QSettings& Settings::instance() {
  static auto ret = QSettings{"no-organization", "CatPi"};
  return ret;
}

int main(int argc, char** argv) { return Main{argc, argv}.app->exec(); }

Main::Main(int& argc, char** argv)
    : app(new QApplication(argc, argv)),
      loadcell(new LoadCell),
      weight(new Weight(loadcell)),
      calibration(new Calibration),
      central(new CentralWidget(weight, calibration)),
      toolbar(new ToolBar),
      window(new MainWindow(central, toolbar)),
      logic(new Logic) {
  app->setStyleSheet("QWidget{font-size: 48pt;} ");

  window->show();    // must be after window is  finished constructing and after setStyleSheet
  connectSignals();  // must be after all members are constructed

  auto startFullscreen = window->isSmallScreen;
  toolbar->fullscreen->setChecked(startFullscreen);
  window->toggleFullscreen(startFullscreen);
}

void Main::connectSignals() {
  // Shortcuts
  auto quitShortcut = new QShortcut(QKeySequence(Qt::Key_F12), window);
  quitShortcut->setContext(Qt::ApplicationShortcut);
  QObject::connect(quitShortcut, &QShortcut::activated, app, &QApplication::quit);
  auto fullscreenShortcut = new QShortcut(QKeySequence(Qt::Key_F11), window);
  fullscreenShortcut->setContext(Qt::ApplicationShortcut);
  QObject::connect(fullscreenShortcut, &QShortcut::activated, toolbar->fullscreen, &QAction::toggle);

  // ToolBar
  QObject::connect(toolbar->quit, &QAction::triggered, app, &QApplication::quit);
  QObject::connect(toolbar->fullscreen, &QAction::toggled, [&](bool checked) {
    window->toggleFullscreen(checked);
    toolbar->fullscreen->setIcon(ToolBar::fullScreenIcon(checked));
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
    if (auto data = loadcell->read()) {
      weight->update(data->value);
      calibration->update(data->reading);
    } else {
      weight->update({});
      calibration->update({});
    }
  });

  // Weight
  weight->connect();

  // Calibration
  QObject::connect(calibration->buttons.back, &QAbstractButton::released,
                   [&] { central->setPage(CentralWidget::Page::Main); });
  QObject::connect(calibration->buttons.step1, &QAbstractButton::released,
                   [&] { calibration->callbacks.step1(loadcell->readPreciseRaw()); });
  QObject::connect(calibration->buttons.step2, &QAbstractButton::released, [&] {
    QString status;
    if (auto readingKnown = loadcell->readPreciseRaw()) {
      if (auto calibrationData = calibration->callbacks.step2(*readingKnown)) {
        loadcell->recalibrate(*calibrationData, status);
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
  QObject::connect(logic->timer, &QTimer::timeout, [&] { central->dispenseButton()->setEnabled(true); });
}
