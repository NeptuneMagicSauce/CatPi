#include "Weight.hpp"

#include <QElapsedTimer>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>
#include <iomanip>
#include <iostream>
#include <map>

#include "AdvancedHX711.h"
#include "GpioException.h"
#include "Instance.hpp"
#include "Mass.h"
#include "TimeoutException.h"

using namespace std;

struct WeightImpl {
  QWidget *widget = new QWidget();
  QTimer *timer = new QTimer;
  QLabel *label = new QLabel;
  double massGrams = 0;
  HX711::AdvancedHX711 *hx711 = tryCreateHX711();
  Instance *instance = nullptr;
  struct {
    const QString key = "tare";
    double value = 0;
    QPushButton *button = new QPushButton();
    QTimer *buttonPressedTimer = new QTimer();
    int buttonPressedTicks = 0;
    const int maxTicks = 40;
    const int interval = 20;
    QString const buttonText = "⚖️ Tare";
    QProgressBar *progress = new QProgressBar();
  } tare;
  static HX711::AdvancedHX711 *tryCreateHX711();
};

HX711::AdvancedHX711 *WeightImpl::tryCreateHX711() {
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
    ret->samplesInTimeOutMode = 1;  // this is my patch to libhx711: no busy wait
                                    // see more comments in install-libhx711.sh
    return ret;
  } catch (HX711::GpioException &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
  } catch (HX711::TimeoutException &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
  } catch (exception &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
  }
  return nullptr;
}

Weight::Weight(Instance *instance) : impl(new WeightImpl()) {
  impl->label->setText("--");
  impl->label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

  auto &tare = impl->tare;
  tare.button->setText(tare.buttonText);
  tare.button->setStyleSheet("QAbstractButton{font-size: 36pt; padding-top: 15px; padding-bottom: 15px} ");

  impl->instance = instance;
  tare.value = instance->settings->value(tare.key, 0.0).toDouble();
  // // debug
  // std::cout << "Tare " << tare.value << endl;
  tare.buttonPressedTimer->setSingleShot(true);
  tare.buttonPressedTimer->setInterval(tare.interval);
  tare.progress->setMaximum(tare.maxTicks);
  tare.progress->setTextVisible(false);

  auto layout = new QVBoxLayout();
  impl->widget->setLayout(layout);
  layout->addWidget(impl->label);
  layout->addWidget(tare.progress);
  tare.progress->setVisible(false);
  layout->addWidget(tare.button);

  impl->timer->setSingleShot(false);
  if (impl->hx711 != nullptr) {
    impl->timer->start(1000);
  }
}

QWidget *Weight::widget() const { return impl->widget; }

void Weight::connect() {
  QObject::connect(impl->timer, &QTimer::timeout, [this]() {
    impl->label->setText("error");
    auto mass = optional<HX711::Mass>{};
    try {
      // auto const samples = 15; // takes 1500 ms
      // auto const samples = 3;  // takes 150 ms
      // auto const samples = 1;
      auto const durationMs = 100;
      QElapsedTimer elapsed;
      elapsed.start();
      mass = impl->hx711->weight(std::chrono::milliseconds{durationMs});
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

    impl->massGrams = mass->getValue(HX711::Mass::Unit::G);  // Mass::getValue() is noexcept
    ostringstream massSs;
    massSs << fixed << setprecision(1) << impl->massGrams - impl->tare.value;
    impl->label->setText(QString::fromStdString(massSs.str()) + "\ngrams");

    // // debug
    // cout << "mass " << impl->massGrams << " tare " << impl->tare.value << endl;
  });

  QObject::connect(impl->tare.button, &QAbstractButton::pressed, [this]() {
    impl->tare.buttonPressedTicks = 0;
    impl->tare.progress->setValue(0);
    impl->tare.progress->setVisible(true);
    impl->tare.buttonPressedTimer->start();
  });
  QObject::connect(impl->tare.button, &QAbstractButton::released, [this]() {
    impl->tare.buttonPressedTimer->stop();
    impl->tare.progress->setVisible(false);
  });
  QObject::connect(impl->tare.buttonPressedTimer, &QTimer::timeout, [this]() {
    auto &tare = impl->tare;
    ++tare.buttonPressedTicks;
    impl->tare.progress->setValue(tare.buttonPressedTicks);
    if (tare.buttonPressedTicks < tare.maxTicks) {
      tare.buttonPressedTimer->start();
    } else {
      impl->tare.progress->setVisible(false);
      // cout << "long press" << endl;
      impl->tare.value = impl->massGrams;
      impl->instance->settings->setValue(impl->tare.key, impl->tare.value);
    }
  });
}
