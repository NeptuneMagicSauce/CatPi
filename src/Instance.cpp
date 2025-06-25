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

QSettings& Instance::settings() {
  static auto ret = QSettings{};
  return ret;
}

struct Application : public QApplication {
  Application(int& argc, char** argv, Instance* instance) : QApplication(argc, argv) {
    QCoreApplication::setOrganizationName("CatPi");
    QCoreApplication::setApplicationName("CatPi");
    // setOrgName must be called before construction of QSettings
  }
};

Instance::Instance(int argc, char** argv)
    : app(new Application(argc, argv, this)),
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
