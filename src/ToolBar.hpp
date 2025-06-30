#pragma once

#include <QToolBar>

struct QAction;

struct ToolBar : public QToolBar {
  ToolBar();

  static QIcon& fullScreenIcon(bool isFullScreen);

  QAction* fullscreen = nullptr;
  QAction* calibration = nullptr;
  QAction* debug = nullptr;
  QAction* quit = nullptr;
};
