#include "Weight.hpp"

#include <QLabel>
#include <QTimer>
#include <iostream>

#include "Instance.hpp"
#include "SimpleHX711.h"
#include "common.h"  // libhx711

using namespace std;

namespace {
auto tryCreateHX711() {
  try {
    return new HX711::SimpleHX711(5, 6, -896, -46937);
  } catch (HX711::GpioException e) {
    std::cerr << e.what() << endl;
    return (HX711::SimpleHX711 *)nullptr;
  }
}
}  // namespace

Weight::Weight() : timer(new QTimer()), label(new QLabel()), hx711(tryCreateHX711()) {
  label->setText("--");
  label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

  timer->setSingleShot(false);
  if (hx711 != nullptr) {
    timer->start(1000);
  }
}

QWidget *Weight::widget() const { return label; }

void Weight::connect() {
  QObject::connect(timer, &QTimer::timeout, [this]() {
    auto const mass = hx711->weight(3);
    auto const massGrams = mass.toString(HX711::Mass::Unit::G);
    // std::cout << massGrams << endl;
    auto s = QString::fromStdString(massGrams);
    s.replace(" g", "\ngrams");
    label->setText(s);
  });
}
