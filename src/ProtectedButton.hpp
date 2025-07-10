#include <QWidget>
#include <functional>

struct ProtectedButtonImpl;
struct QPushButton;

struct ProtectedButton : public QWidget {
  ProtectedButton();
  ~ProtectedButton();

  QTimer* finished;
  QPushButton* button;
  ProtectedButtonImpl* impl;
};
