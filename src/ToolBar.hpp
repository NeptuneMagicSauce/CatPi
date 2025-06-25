#pragma once

#include <QToolBar>

struct QAction;

struct ToolBar : public QToolBar {
  ToolBar();
  void connect();
  QAction* quit = nullptr;
  QAction* fullscreen = nullptr;
  QAction* calibration = nullptr;
};
