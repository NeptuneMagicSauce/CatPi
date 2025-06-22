#include "Instance.hpp"

#include <QApplication>
#include <QLabel>
#include <QScreen>
#include <QTimer>
#include <iostream>

#include "CentralWidget.hpp"
#include "MainWindow.hpp"
#include "PinCtrl.hpp"
#include "ToolBar.hpp"
#include "Weight.hpp"

using namespace std;

Instance::Instance(int argc, char **argv)
    : app(new QApplication(argc, argv)),
      window(new MainWindow),
      isSmallScreen(QGuiApplication::primaryScreen()->geometry().height() <= 720) {
  
  toolbar = new ToolBar(this);
  window->addToolBar(Qt::LeftToolBarArea, toolbar);
  app->setStyleSheet("QLabel{font-size: 48pt;} QAbstractButton{font-size: 48pt;} ");

  weight = new Weight();

  window->setCentralWidget(new CentralWidget(this));

  if (isSmallScreen) {
    window->showFullScreen();
    toolbar->fullscreen->setChecked(true);
  } else {
    window->show();
  }

  connectSignals();  // must be after all membes are constructed
}
