#include "MainScreen.hpp"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>

#include "ProtectedButton.hpp"
#include "Widget.hpp"

MainScreen::MainScreen(QWidget* weight, QWidget* delay) {
  auto layoutMainTop = new QHBoxLayout;
  dispenseButton = new ProtectedButton;
  dispenseButton->button->setText("Now!");
  Widget::FontSized(dispenseButton, 25);
  dispenseButton->button->setSizePolicy(
      {QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum});
  layoutMainTop->addWidget(weight);
  layoutMainTop->addWidget(delay);

  auto dispense = new QGroupBox;
  auto dispenseLayout = new QHBoxLayout;
  dispenseLayout->addWidget(dispenseButton);
  dispense->setLayout(dispenseLayout);

  auto layoutMain = new QVBoxLayout;
  setLayout(layoutMain);
  layoutMain->addLayout(layoutMainTop);
  layoutMain->addWidget(dispense);
}
