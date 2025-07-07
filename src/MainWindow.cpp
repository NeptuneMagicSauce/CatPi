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
    std::function<void()> turnOffScreen = nullptr;
    std::function<void()> reportActivity = nullptr;
  } callbacks;

  void onEvent() {
    assert(callbacks.delayScreenSaverMinutes != nullptr);
    timerInactive.start(callbacks.delayScreenSaverMinutes() * 1000);  // debug seconds
    callbacks.reportActivity();
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
                         std::function<void()> turnOffScreen,
                         std::function<void()> reportActivity) {
  timerInactive.setSingleShot(true);
  QObject::connect(&timerInactive, &QTimer::timeout, [&] {
    assert(callbacks.turnOffScreen != nullptr);
    callbacks.turnOffScreen();
  });

  callbacks.delayScreenSaverMinutes = delayScreenSaverMinutes;
  callbacks.turnOffScreen = turnOffScreen;
  callbacks.reportActivity = reportActivity;
  onEvent();
}

// TODO detect even if it hits a child widget
// maybe with QApplication::mouseMoveEvent()
void MainWindow::mouseMoveEvent(QMouseEvent* event) {
  onEvent();
  QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
  onEvent();
  QMainWindow::mousePressEvent(event);
}
