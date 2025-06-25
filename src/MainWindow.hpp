#pragma once

struct QWidget;
struct QToolBar;

#include <QMainWindow>

struct MainWindow : public QMainWindow {
  MainWindow(QWidget* centralWidget, QToolBar* toolbar);

  bool const isSmallScreen;
  void toggleFullscreen(bool);
};
