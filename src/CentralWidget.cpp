#include "CentralWidget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "Instance.hpp"

CentralWidget::CentralWidget(Instance* instance) {
  auto layout = new QHBoxLayout(this);
  auto& b = instance->action.buttonDispense;
  b = new QPushButton("Now!");
  b->setSizePolicy({QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding});
  layout->addWidget(instance->weight.label);
  layout->addWidget(b);
}
