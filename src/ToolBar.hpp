#pragma once

#include <QToolBar>

struct Instance;
struct QAction;

struct ToolBar : public QToolBar {
  ToolBar(Instance*);
  void connect(Instance*);
  QAction* quit = nullptr;
  QAction* fullscreen = nullptr;
  QAction* calibration = nullptr;
};
