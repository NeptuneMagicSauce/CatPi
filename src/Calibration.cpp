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

struct Callbacks {
  void knowWeightChanged(int value);
} callbacks;
}  // namespace

Calibration::Calibration() {
  AssertSingleton();

  setStyleSheet("QWidget{font-size: 20pt; } ");

  knownWeight = Settings::instance().value(knownWeightKey, 200).toInt();

  buttons.step1 = new QPushButton;
  buttons.step2 = new QPushButton;
  buttons.back = new QPushButton;
  buttons.back->setSizePolicy(buttons.back->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);

  buttons.back->setIcon(QIcon{QPixmap{"://back.png"}});

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

  auto topbar = [&] {
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

    widgetLayout->addWidget(widgetFontSized(widgetAlignCentered(new QLabel(title)), 15));
    widgetLayout->addWidget(
        widgetAlignCentered(widgetFontSized(new QLabel(QString{prompt} + "\nAnd press Ready"), 15)));

    if (button == buttons.step2) {  // known weight screen
      auto parent = new QWidget;
      auto layout = new QVBoxLayout;
      parent->setLayout(layout);
      layout->addWidget(widget);

      knownWeightLabel = widgetAlignCentered(new QLabel);
      ::callbacks.knowWeightChanged(knownWeight);

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
        measure(buttons.step2, "Known Measure", "Put an object of\nknown weight on the scale"));
    return ret;
  }();

  auto bottombar = [&] {
    auto ret = new QGroupBox;
    auto layout = new QVBoxLayout;
    ret->setLayout(layout);
    readingLabel = new QLabel;
    widgetAlignCentered(readingLabel);
    layout->addWidget(readingLabel);
    return ret;
  }();

  auto layout = new QVBoxLayout();
  setLayout(layout);

  layout->addWidget(topbar, 1);
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
void ::Callbacks::knowWeightChanged(int value) {
  knownWeight = value;
  knownWeightLabel->setText(QString::number(knownWeight) + " grams");
  Settings::instance().setValue(knownWeightKey, knownWeight);
}
void Calibration::connect() {
  QObject::connect(deltaDial, &QAbstractSlider::valueChanged, [&] {
    auto newWeight = knownWeight + deltaDial->delta;
    if (newWeight > maxWeight || newWeight < minWeight) {
      return;
    }
    ::callbacks.knowWeightChanged(newWeight);
  });
  deltaDial->connect();
}

void Calibration::update(optional<double> reading) {
  readingLabel->setText("Raw reading: " +
                        (reading.has_value() ? QString::number(reading.value()) : QString{"error"}));
}
