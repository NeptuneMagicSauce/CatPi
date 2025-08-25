#include <QWidget>

struct QAbstractButton;

struct Menu : public QWidget {
  Menu(QWidget* secondWidget);

  QAbstractButton* debug = nullptr;
  QAbstractButton* calibration = nullptr;
};
