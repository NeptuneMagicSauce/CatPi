#pragma once

struct ProtectedButton;
#include <QWidget>

struct MainScreen : public QWidget {
  MainScreen(QWidget* weight, QWidget* delay);

  ProtectedButton* dispenseButton = nullptr;
};
