#include "ToolBar.hpp"

#include <QApplication>
#include <QPixmap>
#include <QStyle>
#include <QTransform>
// #include <iostream>

#include "Widget.hpp"

QIcon& ToolBar::fullScreenIcon(bool isFullScreen) {
  static auto iconYes = QIcon{QPixmap{"://exitfullscreen.png"}};
  static auto iconNo = QIcon{QPixmap{"://fullscreen.png"}};

  return isFullScreen ? iconYes : iconNo;
}

ToolBar::ToolBar() {
  setMovable(false);
  setFloatable(false);
  setContextMenuPolicy(Qt::ContextMenuPolicy::PreventContextMenu);
  setFixedWidth(90);
  setIconSize({90, 90});

  fullscreen = new QAction;
  fullscreen->setIcon(fullScreenIcon(false));
  fullscreen->setText("Fullscreen");

  menu = new QAction;
  menu->setIcon(QIcon{QPixmap{"://burgermenu.png"}});
  menu->setText("Menu");

  quit = new QAction();
  quit->setIcon(QIcon{QPixmap{"://quit.png"}});
  quit->setText("Quit");

  addAction(menu);
  addAction(fullscreen);
  addWidget(Widget::Spacer());
  addAction(quit);
}
