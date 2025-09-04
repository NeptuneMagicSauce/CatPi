#include <QIcon>

struct QString;
struct QLabel;
struct QWidget;
struct QSize;

struct Widget {
  static QWidget* AlignCentered(QLabel* widget);
  static QWidget* FontSized(QWidget* widget, int fontSize);
  static QString StyleSheetFontSize(int fontSize);
  static QWidget* Spacer();

  static bool IsSmallScreen();

  static QIcon RotateIcon(QIcon const& source, int degrees, QSize const& size);
};
