#pragma once

#include <QToolBar>

struct QAction;

struct ToolBar : public QToolBar {
  ToolBar();

  static QIcon& fullScreenIcon(bool isFullScreen);

  QAction* quit = nullptr;
  QAction* fullscreen = nullptr;
  QAction* calibration = nullptr;
};
