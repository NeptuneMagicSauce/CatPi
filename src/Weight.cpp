#include "Weight.hpp"

#include <QElapsedTimer>
#include <QLabel>
#include <QSettings>
#include <QTimer>
#include <iomanip>
#include <iostream>
#include <map>

#include "AdvancedHX711.h"
#include "GpioException.h"
#include "Instance.hpp"
#include "Mass.h"
#include "TimeoutException.h"

using namespace std;

namespace {
Instance *instance = nullptr;
struct {
  const QString key = "tare";
  double value = 0;
} tare;
}  // namespace

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
    // with 5th parameter non-default HX711::Rate::HZ_80 -> same results
    auto ret = new HX711::AdvancedHX711(5, 6, calibration.first, calibration.second, HX711::Rate::HZ_80);
    ret->setConfig(HX711::Channel::A, gain);
    return ret;
  } catch (HX711::GpioException &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
  } catch (HX711::TimeoutException &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
  } catch (exception &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
  }
  return (HX711::AdvancedHX711 *)nullptr;
}
}  // namespace

Weight::Weight(Instance *instance) : timer(new QTimer()), label(new QLabel()), hx711(tryCreateHX711()) {
  label->setText("--");
  label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

  ::instance = instance;
  tare.value = ::instance->settings->value(tare.key, 0.0).toDouble();
  // std::cout << "Tare " << tare.value << endl;

  timer->setSingleShot(false);
  if (hx711 != nullptr) {
    timer->start(1000);
  }
}

QWidget *Weight::widget() const { return label; }

void Weight::connect() {
  QObject::connect(timer, &QTimer::timeout, [this]() {
    label->setText("error");
    auto mass = optional<HX711::Mass>{};
    try {
      // auto const samples = 15; // takes 1500 ms
      // auto const samples = 3;  // takes 150 ms
      auto const samples = 1;
      hx711->samplesInTimeOutMode = samples;  // this is my patch to libhx711: no busy wait
      // see more comments in install-libhx711.sh
      auto const durationMs = 100;
      QElapsedTimer elapsed;
      elapsed.start();
      mass = hx711->weight(std::chrono::milliseconds{durationMs});
      auto const ms = elapsed.elapsed();
      // std::cout << ms << " milliseconds " << ms / samples << " per sample " << endl;
      // std::cout << ms << " milliseconds " << durationMs << " max " << samples << " samples " << endl;
    } catch (HX711::GpioException e) {
      std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
      return;
    } catch (HX711::TimeoutException &e) {
      std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
      return;
    } catch (exception &e) {
      std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
      return;
    }

    // auto massString = mass->toString(HX711::Mass::Unit::G);  // Mass::toString() is noexcept
    // // std::cout << massString << endl;
    // auto s = QString::fromStdString(massString);
    // s.replace(" g", "\ngrams");
    // label->setText(s);

    massGrams = mass->getValue(HX711::Mass::Unit::G);  // Mass::getValue() is noexcept
    ostringstream massSs;
    massSs << fixed << setprecision(1) << massGrams;
    label->setText(QString::fromStdString(massSs.str()) + "\ngrams");

    // debug
    cout << "mass " << massGrams << endl;
  });
}
