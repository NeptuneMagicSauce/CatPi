#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QTimer>
#include <iostream>

#include "Calibration.hpp"
#include "CentralWidget.hpp"
#include "MainWindow.hpp"
#include "PinCtrl.hpp"
#include "Settings.hpp"
#include "ToolBar.hpp"
#include "Weight.hpp"

using namespace std;
using PinCtrl::pinctrl;

struct Main {
  Main(int& argc, char** argv);
  void connectSignals();

  QApplication* app = nullptr;
  Weight* weight = nullptr;
  Calibration* calibration = nullptr;
  CentralWidget* central = nullptr;
  ToolBar* toolbar = nullptr;
  MainWindow* window = nullptr;
};

QSettings& Settings::instance() {
  static auto ret = QSettings{"no-organization", "CatPi"};
  return ret;
}

int main(int argc, char** argv) { return Main{argc, argv}.app->exec(); }

Main::Main(int& argc, char** argv)
    : app(new QApplication(argc, argv)),
      weight(new Weight),
      calibration(new Calibration),
      central(new CentralWidget(weight->widget(), calibration)),
      toolbar(new ToolBar),
      window(new MainWindow(central, toolbar)) {
  app->setStyleSheet("QLabel{font-size: 48pt;} QAbstractButton{font-size: 48pt;} ");

  window->show();    // must be after window is  finished constructing and after setStyleSheet
  connectSignals();  // must be after all members are constructed

  auto startFullscreen = window->isSmallScreen;
  toolbar->fullscreen->setChecked(startFullscreen);
  window->toggleFullscreen(startFullscreen);
}

void Main::connectSignals() {
  QObject::connect(toolbar->quit, &QAction::triggered, app, &QApplication::quit);

  QObject::connect(central->dispenseButton(), &QPushButton::released, [this]() {
    std::cout << "released" << std::endl;
    // pinctrl("-p");
    pinctrl("set 17 op dh");
    central->dispenseButton()->setEnabled(false);
    QTimer::singleShot(4000, [this]() {
      pinctrl("set 17 op dl");
      central->dispenseButton()->setEnabled(true);
    });
  });

  QObject::connect(toolbar->fullscreen, &QAction::toggled, [&](bool checked) {
    window->toggleFullscreen(checked);
    toolbar->fullscreen->setIcon(ToolBar::fullScreenIcon(checked));
  });

  QObject::connect(toolbar->calibration, &QAction::triggered,
                   [&]() { central->setPage(CentralWidget::Page::Calibration); });

  weight->connect();

  auto quitShortcut = new QShortcut(QKeySequence(Qt::Key_F12), window);
  quitShortcut->setContext(Qt::ApplicationShortcut);
  QObject::connect(quitShortcut, &QShortcut::activated, app, &QApplication::quit);

  auto fullscreenShortcut = new QShortcut(QKeySequence(Qt::Key_F11), window);
  fullscreenShortcut->setContext(Qt::ApplicationShortcut);
  QObject::connect(fullscreenShortcut, &QShortcut::activated, toolbar->fullscreen, &QAction::toggle);

  QObject::connect(calibration->buttons.back, &QPushButton::released,
                   [&] { central->setPage(CentralWidget::Page::Main); });
  QObject::connect(calibration->buttons.step1, &QPushButton::released,
                   [&] { calibration->callbacks.step1(); });
  QObject::connect(calibration->buttons.step2, &QPushButton::released,
                   [&] { calibration->callbacks.step2(); });
}
