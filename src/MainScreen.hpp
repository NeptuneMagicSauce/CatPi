struct ProtectedButton;
#include <QWidget>

struct MainScreen : public QWidget {
  MainScreen(QWidget* weight, QWidget* logs);

  ProtectedButton* dispenseButton = nullptr;
};
