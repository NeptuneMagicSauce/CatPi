#include <QWidget>

struct QAbstractButton;
struct Logs;

struct Menu : public QWidget {
  Menu();

  QAbstractButton* debug = nullptr;
  QAbstractButton* calibration = nullptr;
  Logs* logs = nullptr;
};
