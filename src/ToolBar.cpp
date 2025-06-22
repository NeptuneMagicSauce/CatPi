#include "ToolBar.hpp"

#include <QStyle>
#include <QApplication>
#include "Instance.hpp"

ToolBar::ToolBar(Instance* instance) {

  setMovable(false);
  setFloatable(false);
  setContextMenuPolicy(Qt::ContextMenuPolicy::PreventContextMenu);
  setFixedWidth(90);
  setIconSize({90, 90});

  quit = new QAction();
  quit->setIcon(instance->app->style()->standardIcon(QStyle::StandardPixmap::SP_TabCloseButton));
  quit->setText("Quit");
  addAction(quit);

  fullscreen = new QAction();
  fullscreen->setIcon(instance->app->style()->standardIcon(QStyle::StandardPixmap::SP_DesktopIcon));
  fullscreen->setText("Fullscreen");
  fullscreen->setCheckable(true);
  addAction(fullscreen);
}
