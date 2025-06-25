#include "MainWindow.hpp"

#include <QGuiApplication>
#include <QScreen>

MainWindow::MainWindow() : isSmallScreen(QGuiApplication::primaryScreen()->geometry().height() <= 720) {}

void MainWindow::toggleFullscreen(bool checked) {
  if (checked) {
    showFullScreen();
  } else {
    showNormal();
    if (isSmallScreen) {
      setWindowState(Qt::WindowMaximized);
    }
  }
}
