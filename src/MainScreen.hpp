struct ProtectedButton;
#include <QWidget>

struct MainScreen : public QWidget {
  MainScreen(QWidget* weight, QWidget* logs, QWidget* delayTextAndButtons, QWidget* delayProgress);

  void attachWidgets();

  ProtectedButton* dispenseButton = nullptr;
};
