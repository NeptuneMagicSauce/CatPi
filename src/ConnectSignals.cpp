#include "Instance.hpp"

#include <QObject>
#include <QApplication>
#include <QAction>
#include <QPushButton>
#include <QTimer>
#include <QFile>
#include <QLabel>
#include "ToolBar.hpp"
#include "MainWindow.hpp"
#include "PinCtrl.hpp"

using PinCtrl::pinctrl;

void Instance::connectSignals() {

  QObject::connect(
    toolbar->quit, &QAction::triggered,
    app,  &QApplication::quit);
  QObject::connect(
    toolbar->fullscreen, &QAction::toggled, [this] (bool checked) {
      if (checked) {
        window->showFullScreen();
      } else {
        window->showNormal();
        if (isSmallScreen) {
          window->setWindowState(Qt::WindowMaximized);
        }
      }
    });
  QObject::connect(
    action.buttonDispense,
    &QPushButton::released,
    [this] () {
      // std::cout << "released" << std::endl;
      // pinctrl("-p");
      pinctrl("set 17 op dh");
      action.buttonDispense->setEnabled(false);
      QTimer::singleShot(4000, [this] () {
        pinctrl("set 17 op dl");
        action.buttonDispense->setEnabled(true);
      });
    });
  QObject::connect(
    weight.watcher,
    &QTimer::timeout,
    [this] () {
      auto file = QFile("/home/pi/weights.measures");
      if (file.exists() == false) { return; }
      file.open(QIODeviceBase::ReadOnly);
      auto str = file.readAll().trimmed();
      *weight.measure = str;
      weight.label->setText(str + "\ngrams");
      // std::cout << weight.measure.toStdString() << endl;
    });
}
