#include <QWidget>

struct QAbstractButton;

struct Menu : public QWidget {
  Menu();

  QAbstractButton* debug = nullptr;
  QAbstractButton* calibration = nullptr;
};
