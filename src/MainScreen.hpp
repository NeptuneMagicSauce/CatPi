#pragma once

struct QAbstractButton;
#include <QWidget>

struct MainScreen : public QWidget {
  MainScreen(QWidget* weight, QWidget* delay);

  QAbstractButton* dispenseButton = nullptr;
};
