#include "Instance.hpp"

#include <QApplication>
#include <QLabel>
#include <QSettings>
#include <QTimer>
#include <iostream>

#include "CentralWidget.hpp"
#include "MainWindow.hpp"
#include "PinCtrl.hpp"
#include "ToolBar.hpp"
#include "Weight.hpp"

using namespace std;

struct Application : public QApplication {
  // make instance global

  Application(int& argc, char** argv) : QApplication(argc, argv) {
    QCoreApplication::setOrganizationName("CatPi");
    QCoreApplication::setApplicationName("CatPi");
    // setOrgName must be called before construction of QSettings
  }
};

Instance::Instance(int argc, char** argv)
    : app(new Application(argc, argv)),
      settings(new QSettings),
      window(new MainWindow),
      weight(new Weight(this)),
      // central(new CentralWidget(this)),
      toolbar(new ToolBar(this)) {
  window->addToolBar(Qt::LeftToolBarArea, toolbar);
  app->setStyleSheet("QLabel{font-size: 48pt;} QAbstractButton{font-size: 48pt;} ");

  // TODO why does callback of dispense button fail ...
  // when this->central is constructed same as the others above ?
  // rather than next line ?!
  central = new CentralWidget(this);
  window->setCentralWidget(central);
  window->show();

  connectSignals();  // must be after all members are constructed

  if (window->isSmallScreen) {
    toolbar->fullscreen->setChecked(true);
  }
}
