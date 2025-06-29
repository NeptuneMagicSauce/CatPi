#include "ToolBar.hpp"

#include <QApplication>
#include <QPixmap>
#include <QStyle>
#include <QTransform>
// #include <iostream>
#include <optional>

#include "MainWindow.hpp"

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

  quit = new QAction();
  quit->setIcon(QIcon{QPixmap("://quit.png")});
  quit->setText("Quit");

  fullscreen = new QAction();
  fullscreen->setIcon(fullScreenIcon(false));
  fullscreen->setText("Fullscreen");
  fullscreen->setCheckable(true);

  calibration = new QAction();
  calibration->setIcon(QIcon{QPixmap("://settings.png")});
  calibration->setText("Weight Calibration");

  addAction(fullscreen);
  addAction(calibration);
  auto spacer = new QWidget;
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  addWidget(spacer);
  addAction(quit);
}
