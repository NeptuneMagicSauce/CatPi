#include "Weight.hpp"

#include <iostream>
#include <QTimer>
#include <QLabel>
#include "Instance.hpp"
#include "common.h" // libhx711

using namespace std;

Weight::Weight() :
  timer(new QTimer()),
  label(new QLabel()),
  hx711(new HX711::SimpleHX711(5, 6, -896, -46937)) {
  
  label->setText("--");
  label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  
  timer->setSingleShot(false);
  timer->start(1000);
}

QWidget* Weight::widget() const { return label; }

void Weight::connect() {
  QObject::connect(timer, &QTimer::timeout, [this] () {
    auto const mass = hx711->weight(3);
    auto const massGrams = mass.toString(HX711::Mass::Unit::G);
    // std::cout << massGrams << endl;
    auto s = QString::fromStdString(massGrams);
    s.replace(" g", "\ngrams");
    label->setText(s);
  });
}
