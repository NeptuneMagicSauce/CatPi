#include "Instance.hpp"

#include <QApplication>
#include <QScreen>
#include <QLabel>
#include <QTimer>
#include <iostream>
#include "PinCtrl.hpp"
#include "ToolBar.hpp"
#include "MainWindow.hpp"
#include "CentralWidget.hpp"
#include "WeightProgram.hpp"

using namespace std;

Instance::Instance(int argc, char **argv)
    : app(new QApplication(argc, argv)),
      window(new MainWindow),
      isSmallScreen(QGuiApplication::primaryScreen()->geometry().height() <= 720)
{
  toolbar = new ToolBar(this);
  window->addToolBar(Qt::LeftToolBarArea, toolbar);
  app->setStyleSheet("QLabel{font-size: 48pt;} QAbstractButton{font-size: 48pt;} ");

  weight = {
    new QTimer(),
    new QString(),
    new QLabel(),
    make_unique<WeightProgram>(),
  };
  weight.label->setText("--");
  weight.label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  weight.watcher->setSingleShot(false);
  weight.watcher->start(100);

  window->setCentralWidget(new CentralWidget(this));

  if (isSmallScreen) {
    window->showFullScreen();
    toolbar->fullscreen->setChecked(true);
  } else {
    window->show();
  }

  connectSignals(); // must be after all membes are constructed
}
