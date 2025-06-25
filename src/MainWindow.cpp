#include "MainWindow.hpp"

#include <QGuiApplication>
#include <QScreen>

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
    }
  }
}
