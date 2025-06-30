#pragma once

struct QWidget;
struct QToolBar;

#include <QIcon>
#include <QMainWindow>
#include <QStyle>

struct MainWindow : public QMainWindow {
  MainWindow(QWidget* centralWidget, QToolBar* toolbar);

  bool const isSmallScreen;
  void toggleFullscreen();
  void setAutomaticSize();
  bool isFullscreen();

  static QIcon StandardIcon(QStyle::StandardPixmap);
};
