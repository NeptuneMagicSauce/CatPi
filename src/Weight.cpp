#include "Weight.hpp"

#include <QLabel>
#include <QTimer>
#include <iostream>
#include <map>

#include "Instance.hpp"
#include "SimpleHX711.h"
#include "common.h"  // libhx711

using namespace std;

namespace {
auto tryCreateHX711() {
  try {
    auto const gain = HX711::Gain::GAIN_128;
    // auto const gain = HX711::Gain::GAIN_64;

    // GAIN 128
    // calib 1 mouse62 -896, -46937
    // calib 2 mouse64 -867, -47022
    // calib 3 200tare -902, -47069
    // GAIN 64
    // calib1 200tare -452, -22906
    auto const calibrationData = map<HX711::Gain, pair<int, int>>{
        {HX711::Gain::GAIN_128, {-902, -47069}},
        {HX711::Gain::GAIN_64, {-452, -22906}},
    };
    auto const calibration = calibrationData.at(gain);
    auto ret = new HX711::SimpleHX711(5, 6, calibration.first, calibration.second);
    ret->setConfig(HX711::Channel::A, gain);
    return ret;
  } catch (HX711::GpioException e) {
    std::cerr << __FILE__ << ":" << __LINE__ << e.what() << endl;
  }
  return (HX711::SimpleHX711 *)nullptr;
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
    auto mass = optional<HX711::Mass>{};
    try {
      mass = hx711->weight(3);
    } catch (HX711::GpioException e) {
      std::cerr << __FILE__ << ":" << __LINE__ << e.what() << endl;
      return;
    }
    auto massGrams = optional<string>{};
    try {
      massGrams = mass->toString(HX711::Mass::Unit::G);
    } catch (HX711::GpioException e) {
      std::cerr << __FILE__ << ":" << __LINE__ << e.what() << endl;
      return;
    }
    // std::cout << *massGrams << endl;
    auto s = QString::fromStdString(*massGrams);
    s.replace(" g", "\ngrams");
    label->setText(s);
  });
}
