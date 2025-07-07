#pragma once

#include <QIcon>
#include <QMainWindow>
#include <QStyle>

struct QWidget;
struct QToolBar;

struct MainWindow : public QMainWindow {
  MainWindow(QWidget* centralWidget, QToolBar* toolbar);

  bool const isSmallScreen;

  void toggleFullscreen(bool& isFullscreen);
  void setAutomaticSize();

  virtual void mouseMoveEvent(QMouseEvent* event) override;
  virtual void mousePressEvent(QMouseEvent* event) override;

  void connect(std::function<int()> delayScreenSaverMinutes,  //
               std::function<void()> turnOffScreen,           //
               std::function<void()> turnOnScreen);

  static QIcon StandardIcon(QStyle::StandardPixmap);
};
