#include "ToolBar.hpp"

#include <QApplication>
#include <QStyle>
#include <QTransform>
#include <iostream>
#include <optional>

auto standardIcon(auto name) {
  return reinterpret_cast<QApplication*>(QApplication::instance())->style()->standardIcon(name);
}

QIcon& ToolBar::fullScreenIcon(bool isFullScreen) {
  static auto iconYes = standardIcon(QStyle::StandardPixmap::SP_ToolBarVerticalExtensionButton);
  static auto iconNo = std::optional<QIcon>{};
  if (iconNo == std::nullopt) {
    QTransform transform;
    transform.rotate(180, Qt::ZAxis);
    auto pixmap = iconYes.pixmap(QSize{64, 64}).transformed(transform);
    iconNo = QIcon{pixmap};
  }

  return isFullScreen ? iconYes : *iconNo;
}

auto grayscaleQImage(QImage& image) {
  for (int y = 0; y < image.height(); ++y) {
    QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
    for (int x = 0; x < image.width(); ++x) {
      QRgb& rgb = line[x];
      auto r = (float)qRed(rgb) / 255, g = (float)qGreen(rgb) / 255, b = (float)qBlue(rgb) / 255;
      // https://stackoverflow.com/a/17619494
      // http://en.wikipedia.org/wiki/Grayscale#Converting_color_to_grayscale
      auto cLinear = 0.2126f * r + 0.7152f * g + 0.0722f * b;
      auto cSrgb =
          cLinear <= 0.0031308 ? (12.92f * cLinear) : (1.055f * std::pow(cLinear, 1.f / 2.4f) - 0.055f);
      auto cLinear255 = (int)(cSrgb * 255);
      rgb = qRgba(cLinear255, cLinear255, cLinear255, qAlpha(rgb));
    }
  }
}

auto grayscaleQIcon(const QIcon& icon, const QSize& size) {
  auto pixmap = icon.pixmap(size);
  auto image = pixmap.toImage();
  grayscaleQImage(image);
  pixmap = QPixmap::fromImage(image);
  return QIcon{pixmap};
}

ToolBar::ToolBar() {
  setMovable(false);
  setFloatable(false);
  setContextMenuPolicy(Qt::ContextMenuPolicy::PreventContextMenu);
  setFixedWidth(90);
  setIconSize({90, 90});

  quit = new QAction();
  quit->setIcon(standardIcon(QStyle::StandardPixmap::SP_TitleBarCloseButton));
  // SP_TabCloseButton));
  quit->setText("Quit");

  fullscreen = new QAction();
  // SP_ToolBarVerticalExtensionButton)); // good
  // SP_TitleBarNormalButton)); // good
  // SP_DesktopIcon)); // ok
  // SP_DialogHelpButton));
  // SP_FileDialogListView));
  fullscreen->setIcon(fullScreenIcon(false));
  fullscreen->setText("Fullscreen");
  fullscreen->setCheckable(true);

  calibration = new QAction();
  calibration->setIcon(grayscaleQIcon(standardIcon(QStyle::StandardPixmap::SP_BrowserReload), {64, 64}));
  calibration->setText("Weight Calibration");

  addAction(fullscreen);
  addAction(quit);
  addAction(calibration);
}
