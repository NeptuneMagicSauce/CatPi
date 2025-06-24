#include "CentralWidget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "Instance.hpp"
#include "Weight.hpp"

CentralWidget::CentralWidget(Instance* instance) {
  auto layout = new QHBoxLayout(this);
  auto& b = instance->action.buttonDispense;
  b = new QPushButton("Now!");
  b->setSizePolicy({b->sizePolicy().horizontalPolicy(), QSizePolicy::Policy::Expanding});
  layout->addWidget(instance->weight->widget());
  layout->addWidget(b);
}
