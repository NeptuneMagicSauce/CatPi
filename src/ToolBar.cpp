#include "ToolBar.hpp"

#include <QApplication>
#include <QStyle>
#include <QTransform>
#include <optional>

#include "Instance.hpp"
#include "MainWindow.hpp"

auto& fullScreenIcon(bool isFullScreen, Instance* instance) {
  static auto iconYes =
      instance->app->style()->standardIcon(QStyle::StandardPixmap::SP_ToolBarVerticalExtensionButton);
  static auto iconNo = std::optional<QIcon>{};
  if (iconNo == std::nullopt) {
    QTransform transform;
    transform.rotate(180, Qt::ZAxis);
    auto pixmap = iconYes.pixmap(QSize{64, 64}).transformed(transform);
    iconNo = QIcon{pixmap};
  }

  return isFullScreen ? iconYes : *iconNo;
}

ToolBar::ToolBar(Instance* instance) {
  setMovable(false);
  setFloatable(false);
  setContextMenuPolicy(Qt::ContextMenuPolicy::PreventContextMenu);
  setFixedWidth(90);
  setIconSize({90, 90});

  quit = new QAction();
  quit->setIcon(instance->app->style()->standardIcon(QStyle::StandardPixmap::SP_TitleBarCloseButton));
  // SP_TabCloseButton));
  quit->setText("Quit");
  addAction(quit);

  fullscreen = new QAction();
  // SP_ToolBarVerticalExtensionButton)); // good
  // SP_TitleBarNormalButton)); // good
  // SP_DesktopIcon)); // ok
  // SP_DialogHelpButton));
  // SP_FileDialogListView));
  fullscreen->setIcon(fullScreenIcon(false, instance));
  fullscreen->setText("Fullscreen");
  fullscreen->setCheckable(true);
  addAction(fullscreen);

  calibration = new QAction();
  // TODO icon: test all QIcon::fromThem(QIcon::ThemeIcon::)
  // and test all Standard Pixmaps ?
  calibration->setText("C");  // Weight Calibration");
  addAction(calibration);
}

void ToolBar::connect(Instance* instance) {
  QObject::connect(fullscreen, &QAction::toggled, [instance](bool checked) {
    if (checked) {
      instance->window->showFullScreen();
    } else {
      instance->window->showNormal();
      if (instance->isSmallScreen) {
        instance->window->setWindowState(Qt::WindowMaximized);
      }
    }
    instance->toolbar->fullscreen->setIcon(fullScreenIcon(checked, instance));
  });
}
