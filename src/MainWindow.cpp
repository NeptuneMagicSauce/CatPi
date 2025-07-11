#include "MainWindow.hpp"

#include <QApplication>
#include <QResizeEvent>

#include "System.hpp"
#include "Widget.hpp"
// #include <iostream>

namespace {
  auto isFullscreen = false;
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

void MainWindow::resizeEvent(QResizeEvent* event) {
  qDebug() << "Main Window size" << event->size();
  QMainWindow::resizeEvent(event);
}
