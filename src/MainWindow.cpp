#include "MainWindow.hpp"

#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <iostream>

MainWindow::MainWindow(QWidget* centralWidget, QToolBar* toolbar)
    : isSmallScreen(QGuiApplication::primaryScreen()->geometry().height() <= 720) {
  addToolBar(Qt::LeftToolBarArea, toolbar);
  setCentralWidget(centralWidget);
}

void MainWindow::toggleFullscreen(bool checked) {
  if (checked) {
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
