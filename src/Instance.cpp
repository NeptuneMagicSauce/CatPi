#include "Instance.hpp"

#include <QApplication>
#include <QLabel>
#include <QScreen>
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
  Application(int& argc, char** argv) : QApplication(argc, argv) {
    QCoreApplication::setOrganizationName("CatPi");
    QCoreApplication::setApplicationName("CatPi");
    // setOrgName must be called before construction of QSettings
  }
};

Instance::Instance(int argc, char** argv)
    : app(new Application(argc, argv)),
      window(new MainWindow),
      toolbar(new ToolBar(this)),
      settings(new QSettings()),
      isSmallScreen(QGuiApplication::primaryScreen()->geometry().height() <= 720) {
  window->addToolBar(Qt::LeftToolBarArea, toolbar);
  app->setStyleSheet("QLabel{font-size: 48pt;} QAbstractButton{font-size: 48pt;} ");

  weight = new Weight(this);

  window->setCentralWidget(new CentralWidget(this));
  window->show();

  connectSignals();  // must be after all members are constructed

  if (isSmallScreen) {
    toolbar->fullscreen->setChecked(true);
  }
}
