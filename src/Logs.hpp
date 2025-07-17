#include <QWidget>

struct QMainWindow;

struct Logs : public QWidget {
  void connect(QMainWindow* window);
};
