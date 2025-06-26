#include "Calibration.hpp"

#include <QDial>
#include <QFile>
#include <QGroupBox>
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
int knownWeight = 200;
QLabel* knownWeightLabel = nullptr;

// a dial to input small deltas, wrapping
// @member delta: the difference in value when it changed
struct QDeltaDial : public QDial {
  int oldValue = 0;
  int delta = 0;
  QDeltaDial(QWidget* parent = nullptr) : QDial(parent) {
    setMaximum(10);
    setWrapping(true);
  }
  void connect() {
    QObject::connect(this, &QAbstractSlider::valueChanged, [&](auto value) {
      delta = value - oldValue;
      if (std::abs(delta) > maximum() / 4) {  // just wrapped, delta is max range
        delta = 0;
      }
      oldValue = value;
    });
  }
};
struct {
  QAbstractButton* step1 = nullptr;
  QAbstractButton* step2 = nullptr;
  QAbstractSlider* knownWeight = nullptr;
  QDeltaDial* deltaDial = nullptr;

} buttons;

struct Callbacks {
  void step1();
  void step2();
  void knowWeightChanged(int value);
} callbacks;
}  // namespace

#include "System.hpp"

Calibration::Calibration() {
  AssertSingleton();

  setStyleSheet("QWidget{font-size: 20pt; } ");

  ::buttons.step1 = new QPushButton;
  ::buttons.step2 = new QPushButton;
  buttons.back = new QPushButton;
  buttons.back->setSizePolicy(buttons.back->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);

  buttons.back->setIcon(MainWindow::StandardIcon(QStyle::StandardPixmap::SP_MediaSeekBackward));

  auto widgetAlignCentered = [](auto widget) {
    widget->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    return widget;
  };

  auto widgetStyleSheeted = [](auto widget, auto stylesheet) {
    widget->setStyleSheet(stylesheet);
    return widget;
  };

  auto widgetFontSized = [&](auto widget, auto fontsize) {
    return widgetStyleSheeted(widget,
                              QString("QWidget{font-size: ") + QString::number(fontsize) + QString("pt; }"));
  };

  auto topbar = [&]() {
    auto ret = new QGroupBox;
    auto layout = new QHBoxLayout;
    ret->setLayout(layout);
    layout->addWidget(buttons.back, 1);
    auto title = new QLabel("Calibration");
    widgetAlignCentered(title);
    widgetFontSized(title, 28);
    layout->addWidget(title, 4);
    return ret;
  }();

  auto measure = [&](auto button, auto& title, auto& prompt) {
    auto ret = new QWidget;
    auto layout = new QHBoxLayout;
    ret->setLayout(layout);

    auto widget = new QWidget;
    auto widgetLayout = new QVBoxLayout;
    widget->setLayout(widgetLayout);

    widgetLayout->addWidget(widgetAlignCentered(new QLabel(title)));
    widgetLayout->addWidget(widgetAlignCentered(new QLabel(QString{prompt} + "\nAnd press Ready")));

    if (button == ::buttons.step2) {  // known weight screen
      auto parent = new QWidget;
      auto layout = new QVBoxLayout;
      parent->setLayout(layout);
      layout->addWidget(widget);
      auto dial = new QDial;
      dial->setNotchesVisible(true);
      dial->setNotchTarget(50);
      dial->setMinimum(10);
      dial->setMaximum(1000);
      dial->setWrapping(false);
      dial->setValue(knownWeight);
      ::buttons.knownWeight = dial;
      knownWeightLabel = widgetAlignCentered(new QLabel);
      callbacks.knowWeightChanged(knownWeight);

      auto deltaDial = new QDeltaDial();
      ::buttons.deltaDial = deltaDial;

      auto group = new QGroupBox;
      auto groupLayout = new QHBoxLayout;
      group->setLayout(groupLayout);

      groupLayout->addWidget(dial, 2);
      groupLayout->addWidget(deltaDial, 1);
      groupLayout->addWidget(knownWeightLabel, 2);

      // alternative layout from previous 3 lines ->
      // auto subgroup = new QWidget;
      // auto subgroupLayout = new QVBoxLayout;
      // subgroup->setLayout(subgroupLayout);
      // subgroupLayout->addWidget(deltaDial);
      // subgroupLayout->addWidget(knownWeightLabel);
      // groupLayout->addWidget(dial);
      // groupLayout->addWidget(subgroup);

      layout->addWidget(group);
      widget = parent;
    }

    layout->addWidget(widget, 3);
    layout->addWidget(button, 1);

    button->setText("Ready");
    button->setSizePolicy(ret->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    return ret;
  };

  auto central = [&]() {
    auto ret = new QWidget;
    screens = new QStackedLayout;
    ret->setLayout(screens);
    ret->setSizePolicy(ret->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    screens->addWidget(measure(::buttons.step1, "Empty Measure", "Remove any object from the scale"));
    screens->addWidget(
        measure(::buttons.step2, "Known Measure", "Put an object of\nknown weight on the scale"));
    return ret;
  }();

  // TODO
  // some way to do hx711
  // a live measure of the readings
  // the calibration algorithm
  // stored and restored

  auto layout = new QVBoxLayout();
  setLayout(layout);

  layout->addWidget(topbar, 1);
  layout->addWidget(central, 4);
}

void Calibration::showEvent(QShowEvent* e) {
  screens->setCurrentIndex(0);
  QWidget::showEvent(e);
}
void ::Callbacks::step1() { screens->setCurrentIndex(1); }
void ::Callbacks::step2() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
void ::Callbacks::knowWeightChanged(int value) {
  knownWeight = value;
  knownWeightLabel->setText(QString::number(knownWeight) + " grams");
}
void Calibration::connect() {
  QObject::connect(::buttons.step1, &QAbstractButton::released, [&] { callbacks.step1(); });
  QObject::connect(::buttons.step2, &QAbstractButton::released, [&] { callbacks.step2(); });
  QObject::connect(::buttons.knownWeight, &QAbstractSlider::valueChanged,
                   [&](auto value) { callbacks.knowWeightChanged(value); });
  QObject::connect(::buttons.deltaDial, &QAbstractSlider::valueChanged, [&] {
    auto newWeight = knownWeight + ::buttons.deltaDial->delta;
    if (newWeight > ::buttons.knownWeight->maximum() || newWeight < ::buttons.knownWeight->minimum()) {
      return;
    }
    callbacks.knowWeightChanged(newWeight);
    ::buttons.knownWeight->setValue(knownWeight);
  });
  ::buttons.deltaDial->connect();
}
