#include "MainScreen.hpp"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>

#include "ProtectedButton.hpp"
#include "System.hpp"
#include "Widget.hpp"

struct MainScreenImpl {
  QWidget* delayTextAndButtons = nullptr;
  QWidget* delayProgress = nullptr;
  QVBoxLayout* layoutRight = nullptr;
}* impl = nullptr;

MainScreen::MainScreen(QWidget* weight,               //
                       QWidget* secondWidget,         //
                       QWidget* delayTextAndButtons,  //
                       QWidget* delayProgress) {
  AssertSingleton();

  impl = new MainScreenImpl;

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

  auto& layoutRight = impl->layoutRight;
  layoutRight = new QVBoxLayout;
  auto widgetRight = new QGroupBox;
  widgetRight->setLayout(layoutRight);
  layoutRight->addWidget(secondWidget);
  layoutRight->addWidget(delayTextAndButtons);
  layoutRight->addWidget(delayProgress);

  impl->delayTextAndButtons = delayTextAndButtons;
  impl->delayProgress = delayProgress;

  auto layoutMain = new QHBoxLayout;
  setLayout(layoutMain);
  layoutMain->addLayout(layoutLeft);
  layoutMain->addWidget(widgetRight);
}

void MainScreen::attachWidgets() {
  impl->layoutRight->addWidget(impl->delayTextAndButtons);
  impl->layoutRight->addWidget(impl->delayProgress);
}
