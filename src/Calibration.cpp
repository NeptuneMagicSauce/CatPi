#include "Calibration.hpp"

#include <QLabel>
#include <QPushButton>
#include <QShowEvent>
#include <QStackedLayout>
#include <QToolButton>
#include <QVBoxLayout>
#include <iostream>

#include "MainWindow.hpp"

// auto spacer = []() {
//   auto spacer = new QWidget;
//   spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//   return spacer;
// };

namespace {
QStackedLayout* screens = nullptr;
}

Calibration::Calibration() {
  static auto first = true;
  assert(first);  // singleton
  first = false;

  setStyleSheet("QWidget{font-size: 20pt; } ");

  buttons.step1 = new QPushButton;
  buttons.step2 = new QPushButton;
  buttons.back = new QToolButton;
  // TODO test on pi: surely this back button is too small
  buttons.back->setText("Back");

  buttons.back->setIcon(MainWindow::StandardIcon(QStyle::StandardPixmap::SP_MediaSeekBackward));

  auto topbar = [&]() {
    auto ret = new QWidget;
    auto layout = new QHBoxLayout;
    ret->setLayout(layout);
    layout->addWidget(buttons.back);
    auto title = new QLabel("  Calibration");
    title->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    layout->addWidget(title);
    return ret;
  }();

  auto measure = [&](auto button, auto& title, auto& prompt) {
    auto ret = new QWidget;
    auto layout = new QHBoxLayout;
    ret->setLayout(layout);

    auto widget = new QWidget;
    auto widgetLayout = new QVBoxLayout;
    widget->setLayout(widgetLayout);

    widgetLayout->addWidget(new QLabel(title));
    widgetLayout->addWidget(new QLabel(QString{prompt} + "\nAnd press Ready"));

    layout->addWidget(widget);
    layout->addWidget(button);

    button->setText("Ready");
    return ret;
  };

  auto central = [&]() {
    auto ret = new QWidget;
    screens = new QStackedLayout;
    ret->setLayout(screens);
    ret->setSizePolicy(ret->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    screens->addWidget(measure(buttons.step1, "Empty Measure", "Remove any object from the scale"));
    screens->addWidget(measure(buttons.step2, "Known Measure", "Put an object of known weight on the scale"));
    return ret;
  }();

  // TODO
  // text formatting: title page, title prompt, alignment
  // a weight input for the second step
  //   compatible with small screen touch only
  //   default 200g
  // some way to do hx711
  // a live measure of the readings
  // the calibration algorithm
  // stored and restored

  auto layout = new QVBoxLayout();
  setLayout(layout);

  layout->addWidget(topbar);
  layout->addWidget(central);
}

void Calibration::showEvent(QShowEvent* e) {
  screens->setCurrentIndex(0);
  QWidget::showEvent(e);
}
void Calibration::Callbacks::step1() { screens->setCurrentIndex(1); }
void Calibration::Callbacks::step2() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
