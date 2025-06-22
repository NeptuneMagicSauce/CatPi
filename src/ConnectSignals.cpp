#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QTimer>

#include "Instance.hpp"
#include "MainWindow.hpp"
#include "PinCtrl.hpp"
#include "ToolBar.hpp"
#include "Weight.hpp"

using PinCtrl::pinctrl;

void Instance::connectSignals() {
  QObject::connect(toolbar->quit, &QAction::triggered, app, &QApplication::quit);
  
  QObject::connect(toolbar->fullscreen, &QAction::toggled, [this](bool checked) {
    if (checked) {
      window->showFullScreen();
    } else {
      window->showNormal();
      if (isSmallScreen) {
        window->setWindowState(Qt::WindowMaximized);
      }
    }
  });
  
  QObject::connect(action.buttonDispense, &QPushButton::released, [this]() {
    // std::cout << "released" << std::endl;
    // pinctrl("-p");
    pinctrl("set 17 op dh");
    action.buttonDispense->setEnabled(false);
    QTimer::singleShot(4000, [this]() {
      pinctrl("set 17 op dl");
      action.buttonDispense->setEnabled(true);
    });
  });
  
  weight->connect();
}
