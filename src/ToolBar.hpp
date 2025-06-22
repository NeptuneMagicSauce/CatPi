#pragma once

#include <QToolBar>

struct Instance;
struct QAction;

struct ToolBar : public QToolBar {
  ToolBar(Instance*);
  QAction* quit = nullptr;
  QAction* fullscreen = nullptr;
};
