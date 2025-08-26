#include "Calibration.hpp"

#include <QDial>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>
#include <QStackedLayout>
#include <QToolButton>
#include <QVBoxLayout>
// #include <iostream>

#include "DeltaDial.hpp"
#include "MainWindow.hpp"
#include "Settings.hpp"
#include "System.hpp"
#include "Widget.hpp"

using std::optional;
using std::pair;

namespace {

  QStackedLayout* screens = nullptr;
  int knownWeight = 0;
  const QString knownWeightKey = "KnownWeight";
  QLabel* knownWeightLabel = nullptr;
  QLabel* readingLabel = nullptr;
  DeltaDial* deltaDial = nullptr;

  double readingZero = 0;

  auto const minWeight = 10;
  auto const maxWeight = 1000;
}

Calibration::Calibration() {
  AssertSingleton();

  Widget::FontSized(this, 20);

  knownWeightLabel = new QLabel;
  Settings::load({.key = knownWeightKey,
                  .name = "Poids de référence",
                  .prompt = "Poids de référence pour la calibration",
                  .unit = "Grammes",
                  .defaultValue = 200,
                  .callback =
                      [&](QVariant v) {
                        knownWeight = v.toInt();
                        updateLabel();
                      },
                  .limits = {.minimum = {}, .maximum = {}}});

  buttons.step1 = new QPushButton;
  buttons.step2 = new QPushButton;

  auto measure = [&](auto button, auto& title, auto& prompt) {
    auto ret = new QWidget;
    auto layout = new QHBoxLayout;
    ret->setLayout(layout);

    auto widget = new QWidget;
    auto widgetLayout = new QVBoxLayout;
    widget->setLayout(widgetLayout);

    widgetLayout->addWidget(Widget::FontSized(Widget::AlignCentered(new QLabel(title)), 15));
    widgetLayout->addWidget(Widget::FontSized(
        Widget::AlignCentered(new QLabel(QString{prompt} + "\nAnd press Ready")), 15));

    if (button == buttons.step2) {  // known weight screen
      auto parent = new QWidget;
      auto layout = new QVBoxLayout;
      parent->setLayout(layout);
      layout->addWidget(widget);

      Widget::AlignCentered(knownWeightLabel);

      deltaDial = new DeltaDial;

      auto group = new QGroupBox;
      auto groupLayout = new QHBoxLayout;
      group->setLayout(groupLayout);

      groupLayout->addWidget(deltaDial);
      groupLayout->addWidget(knownWeightLabel);

      layout->addWidget(group);
      widget = parent;
    }

    layout->addWidget(widget, 3);
    layout->addWidget(button, 1);

    button->setText("Ready");
    button->setSizePolicy(ret->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);

    return ret;
  };

  auto central = [&] {
    auto ret = new QWidget;
    screens = new QStackedLayout;
    ret->setLayout(screens);
    ret->setSizePolicy(ret->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    screens->addWidget(measure(buttons.step1, "Empty Measure", "Remove any object from the scale"));
    screens->addWidget(
        measure(buttons.step2, "Known Measure", "Put an object of known weight on the scale"));
    return ret;
  }();

  auto bottombar = [&] {
    auto ret = new QGroupBox;
    auto layout = new QVBoxLayout;
    ret->setLayout(layout);
    readingLabel = new QLabel;
    Widget::AlignCentered(readingLabel);
    layout->addWidget(readingLabel);
    return ret;
  }();

  auto layout = new QVBoxLayout();
  setLayout(layout);

  layout->addWidget(central, 4);
  layout->addWidget(bottombar, 1);
}

void Calibration::showEvent(QShowEvent* e) {
  screens->setCurrentIndex(0);
  QWidget::showEvent(e);
}
void Calibration::Callbacks::step1(optional<double> rawPrecise) {
  if (rawPrecise.has_value()) {
    readingZero = rawPrecise.value();
  }
  screens->setCurrentIndex(1);
}
optional<pair<int, int>> Calibration::Callbacks::step2(optional<double> rawPrecise) {
  if (rawPrecise.has_value() == false) {
    return {};
  }

  auto readingKnown = rawPrecise.value();
  if (readingZero && readingKnown) {
    auto ref = round(readingKnown - readingZero) / knownWeight;
    auto offset = round(readingZero);

    return pair{ref, offset};
  }
  return {};
}

void Calibration::updateLabel() {
  knownWeightLabel->setText(QString::number(knownWeight) + " grams");
}

void Calibration::connect() {
  QObject::connect(deltaDial, &QAbstractSlider::valueChanged, [&] {
    auto newWeight = knownWeight + deltaDial->delta;
    if (newWeight > maxWeight || newWeight < minWeight) {
      return;
    }
    knownWeight = newWeight;
    Settings::set(knownWeightKey, knownWeight);
    updateLabel();
  });
}

void Calibration::update(optional<double> reading) {
  readingLabel->setText("Raw reading: " + (reading.has_value() ? QString::number(reading.value())
                                                               : QString{"error"}));
}
