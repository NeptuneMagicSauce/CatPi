#include "WaitWidgets.hpp"

#include <QHBoxLayout>
#include <QLabel>

#include "System.hpp"

struct WaitWidgetsImpl {
  WaitWidgetsImpl(WaitWidgets* parent);
};

namespace {
WaitWidgetsImpl* impl = nullptr;
}

WaitWidgets::WaitWidgets() {
  AssertSingleton();
  impl = new WaitWidgetsImpl(this);
}

WaitWidgetsImpl::WaitWidgetsImpl(WaitWidgets* parent) {
  auto layout = new QHBoxLayout;
  parent->setLayout(layout);

  auto& timeTo = parent->timeToDispense;
  timeTo = new QLabel;
  layout->addWidget(timeTo);
  timeTo->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}
