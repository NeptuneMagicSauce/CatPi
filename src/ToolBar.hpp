#include <QToolBar>

struct QAction;

struct ToolBar : public QToolBar {
  ToolBar();

  static QIcon& fullScreenIcon(bool isFullScreen);

  QAction* fullscreen = nullptr;
  QAction* menu = nullptr;
  QAction* logs = nullptr;
  QAction* quit = nullptr;
};
