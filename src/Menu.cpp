#include "Menu.hpp"

#include <QBoxLayout>
#include <QToolButton>

#include "Temperature.hpp"

Menu::Menu() {
  calibration = new QToolButton;
  calibration->setIcon(QIcon{QPixmap{"://weightbalance.png"}});
  calibration->setText("Calibration");
  calibration->setIconSize({100, 100});

  debug = new QToolButton;
  debug->setIcon(QIcon{QPixmap{"://bug.png"}});
  debug->setText("Debug");
  debug->setIconSize({100, 100});

  auto layout = new QVBoxLayout;
  setLayout(layout);
  layout->addWidget(debug);
  layout->addWidget(calibration);
  layout->addWidget(new Temperature);
}
