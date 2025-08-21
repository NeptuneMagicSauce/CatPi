#include "Menu.hpp"

#include <QBoxLayout>
#include <QGroupBox>
#include <QToolButton>

#include "Logs.hpp"
#include "Temperature.hpp"
#include "Widget.hpp"

Menu::Menu() {
  calibration = new QToolButton;
  calibration->setIcon(QIcon{QPixmap{"://weightbalance.png"}});
  calibration->setText("Calibration");
  calibration->setIconSize({100, 100});

  debug = new QToolButton;
  debug->setIcon(QIcon{QPixmap{"://bug.png"}});
  debug->setText("Debug");
  debug->setIconSize({100, 100});

  auto leftSide = new QGroupBox;
  auto leftLayout = new QVBoxLayout;
  leftSide->setLayout(leftLayout);
  leftLayout->addWidget(debug);
  leftLayout->addWidget(calibration);
  leftLayout->addWidget(new Temperature);

  auto rightSide = new QGroupBox;
  auto rightLayout = new QVBoxLayout;
  rightSide->setLayout(rightLayout);
  logs = new Logs;
  rightLayout->addWidget(logs);

  auto layout = new QHBoxLayout;
  setLayout(layout);
  layout->addWidget(Widget::Spacer());
  layout->addWidget(leftSide);
  layout->addWidget(Widget::Spacer());
  layout->addWidget(rightSide);
  layout->addWidget(Widget::Spacer());
}
