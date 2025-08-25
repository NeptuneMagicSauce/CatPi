#include "MainScreen.hpp"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>

#include "ProtectedButton.hpp"
#include "Widget.hpp"

MainScreen::MainScreen(QWidget* weight, QWidget* secondWidget) {
  dispenseButton = new ProtectedButton;
  dispenseButton->button->setText("Croquettes !");
  Widget::FontSized(dispenseButton, 25);
  dispenseButton->button->setSizePolicy(
      {QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum});
  auto dispense = new QGroupBox;
  auto dispenseLayout = new QHBoxLayout;
  dispenseLayout->addWidget(dispenseButton);
  dispense->setLayout(dispenseLayout);

  auto layoutLeft = new QVBoxLayout;
  layoutLeft->addWidget(weight);
  layoutLeft->addWidget(dispense);

  auto layoutMain = new QHBoxLayout;
  setLayout(layoutMain);
  layoutMain->addLayout(layoutLeft);
  layoutMain->addWidget(secondWidget);
}
