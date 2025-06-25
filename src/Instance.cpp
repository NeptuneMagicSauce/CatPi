#include "Instance.hpp"

#include <QApplication>
#include <QLabel>
#include <QSettings>
#include <QTimer>
#include <iostream>

#include "CentralWidget.hpp"
#include "MainWindow.hpp"
#include "PinCtrl.hpp"
#include "Settings.hpp"
#include "ToolBar.hpp"
#include "Weight.hpp"

using namespace std;

QSettings& Settings::instance() {
  static auto ret = QSettings{"no-organization", "CatPi"};
  return ret;
}

Instance::Instance(int argc, char** argv)
    : app(new QApplication(argc, argv)),
      window(new MainWindow),
      weight(new Weight),
      central(new CentralWidget(weight->widget())),
      toolbar(new ToolBar) {
  window->addToolBar(Qt::LeftToolBarArea, toolbar);
  app->setStyleSheet("QLabel{font-size: 48pt;} QAbstractButton{font-size: 48pt;} ");

  window->setCentralWidget(central);
  window->show();

  connectSignals();  // must be after all members are constructed

  if (window->isSmallScreen) {
    toolbar->fullscreen->setChecked(true);
  }
}
