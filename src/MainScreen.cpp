#include "MainScreen.hpp"

#include <QHBoxLayout>
#include <QPushButton>

MainScreen::MainScreen(QWidget* weight, QWidget* delay) {
  auto layoutMainTop = new QHBoxLayout;
  dispenseButton = new QPushButton("Now!");
  dispenseButton->setSizePolicy(
      {dispenseButton->sizePolicy().horizontalPolicy(), QSizePolicy::Policy::Expanding});
  layoutMainTop->addWidget(weight);
  layoutMainTop->addWidget(dispenseButton);

  auto layoutMain = new QVBoxLayout;
  setLayout(layoutMain);
  layoutMain->addLayout(layoutMainTop, 2);
  layoutMain->addWidget(delay, 1);
}
