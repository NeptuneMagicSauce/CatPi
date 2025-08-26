#include "Menu.hpp"

#include <QBoxLayout>
#include <QGroupBox>
#include <QToolButton>

#include "Temperature.hpp"
#include "Widget.hpp"

Menu::Menu(QWidget* secondWidget) {
  calibration = new QToolButton;
  calibration->setIcon(QIcon{QPixmap{"://weightbalance.png"}});
  calibration->setText("Calibration");
  calibration->setIconSize({100, 100});
  calibration->setSizePolicy(QSizePolicy::Expanding, calibration->sizePolicy().verticalPolicy());

  debug = new QToolButton;
  debug->setIcon(QIcon{QPixmap{"://bug.png"}});
  debug->setText("Debug");
  debug->setIconSize({100, 100});
  debug->setSizePolicy(QSizePolicy::Expanding, debug->sizePolicy().verticalPolicy());

  auto leftSide = new QGroupBox;
  auto leftLayout = new QVBoxLayout;
  leftSide->setLayout(leftLayout);
  leftLayout->addWidget(Widget::Spacer());
  leftLayout->addWidget(debug);
  leftLayout->addWidget(Widget::Spacer());
  leftLayout->addWidget(calibration);
  leftLayout->addWidget(Widget::Spacer());
  leftLayout->addWidget(new Temperature);
  leftLayout->addWidget(Widget::Spacer());

  auto rightSide = new QGroupBox;
  auto rightLayout = new QVBoxLayout;
  rightSide->setLayout(rightLayout);

  rightLayout->addWidget(secondWidget);

  auto layout = new QHBoxLayout;
  setLayout(layout);
  layout->addWidget(Widget::Spacer());
  layout->addWidget(leftSide);
  layout->addWidget(Widget::Spacer());
  layout->addWidget(rightSide);
  layout->addWidget(Widget::Spacer());
}
