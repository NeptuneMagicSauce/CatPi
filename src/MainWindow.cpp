#include "MainWindow.hpp"

#include <QApplication>
#include <QTimer>

#include "System.hpp"
#include "Widget.hpp"
// #include <iostream>

namespace {
  auto isFullscreen = false;
  QTimer timerInactive;

  struct {
    std::function<int()> delayScreenSaverMinutes = nullptr;
    std::function<void(bool)> setScreenIsOn = nullptr;
  } callbacks;

  void onEvent() {
    assert(callbacks.delayScreenSaverMinutes != nullptr);
#warning "debug minutes to seconds here, to remove"
    timerInactive.start(callbacks.delayScreenSaverMinutes() * 1000);
    callbacks.setScreenIsOn(true);
  }
}

MainWindow::MainWindow(QWidget* centralWidget, QToolBar* toolbar)
    : isSmallScreen(Widget::IsSmallScreen()) {
  AssertSingleton();

  addToolBar(Qt::LeftToolBarArea, toolbar);
  setCentralWidget(centralWidget);
}

void MainWindow::toggleFullscreen(bool& isFullscreen) {
  ::isFullscreen = !::isFullscreen;
  setAutomaticSize();
  isFullscreen = ::isFullscreen;
}

void MainWindow::setAutomaticSize() {
  if (::isFullscreen) {
    showFullScreen();
  } else {
    showNormal();
    if (isSmallScreen) {
      setWindowState(Qt::WindowMaximized);
    } else {
      // use the resolution of the pi
      resize(800, 480);
    }
  }
}

QIcon MainWindow::StandardIcon(QStyle::StandardPixmap name) {
  return reinterpret_cast<QApplication*>(QApplication::instance())->style()->standardIcon(name);
}

void MainWindow::connect(std::function<int()> delayScreenSaverMinutes,
                         std::function<void(bool)> setScreenIsOn) {
  timerInactive.setSingleShot(true);
  QObject::connect(&timerInactive, &QTimer::timeout, [&] {
    assert(callbacks.setScreenIsOn != nullptr);
    callbacks.setScreenIsOn(false);
  });

  callbacks.delayScreenSaverMinutes = delayScreenSaverMinutes;
  callbacks.setScreenIsOn = setScreenIsOn;
  onEvent();
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
  onEvent();
  QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
  onEvent();
  QMainWindow::mousePressEvent(event);
}
