#include <QIcon>
#include <QImage>
#include <QLabel>
#include <QWidget>
#include <iostream>

auto debugFormat(auto filename, auto widget) {
  auto file = QFile(filename);
  std::cout << filename << " " << file.exists() << std::endl;
  if (file.exists()) {
    file.open(QIODeviceBase::ReadOnly);
    try {
      widget->setStyleSheet(file.readAll());
    } catch (std::exception e) {
      std::cout << e.what() << std::endl;
    }
  }
  return widget;
}

auto rotateIcon() {
  static auto iconYes =
      QIcon{};  // MainWindow::StandardIcon(QStyle::StandardPixmap::SP_ToolBarVerticalExtensionButton);
  static auto iconNo = std::optional<QIcon>{};
  if (iconNo.has_value() == false) {
    QTransform transform;
    transform.rotate(180, Qt::ZAxis);
    auto pixmap = iconYes.pixmap(QSize{64, 64}).transformed(transform);
    iconNo = QIcon{pixmap};
  }
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

auto spacer() {
  auto spacer = new QWidget;
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  return spacer;
};

struct QLabelScaledDownFontFont : public QLabel {
  QLabelScaledDownFontFont(const QString& text) : QLabel(text) {}
  void resizeEvent(QResizeEvent* e) override {
    QLabel::resizeEvent(e);
    auto contents = contentsRect();
    auto theFont = font();
    auto computeTextSize = [&] { return QFontMetrics(theFont).boundingRect(text()); };
    auto textSize = computeTextSize();
    qDebug() << text() << contents;
    qDebug() << metaObject()->className() << textSize << theFont.family() << theFont.pointSize();
    while (contents.width() < textSize.width() || contents.height() < textSize.height()) {
      if (theFont.pointSize() <= 1) {
        qDebug() << "internal error" << __PRETTY_FUNCTION__;
        break;
      }
      theFont.setPointSize(theFont.pointSize() - 1);
      textSize = computeTextSize();
      qDebug() << metaObject()->className() << textSize << theFont.family() << theFont.pointSize();
      setFont(theFont);  // this does not work! ...
      // the size is correctly computed
      // but I do not see the new size being applied after setFont()
    }
  }
};

auto separateWords(auto line) {
  auto asWords = QString{};
  auto prev = QChar{' '};
  for (auto c : line) {
    if (c.isUpper() || (c.isDigit() && prev.isDigit() == false)) {
      asWords += " ";
    }
    asWords += c;
    prev = c;
  }
  return asWords;
}
