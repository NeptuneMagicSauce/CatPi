#include <QWidget>

struct QMainWindow;

struct Plots : public QWidget {
  void connect(QMainWindow* window);
};
