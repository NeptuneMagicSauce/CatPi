#include "LoadCell.hpp"

#include <QTimer>
#include <cassert>
#include <iostream>
#include <map>

#include "AdvancedHX711.h"
#include "GpioException.h"
#include "Settings.hpp"
#include "System.hpp"
#include "TimeoutException.h"

using namespace HX711;
using namespace std;

struct LoadCellImpl {
  const std::chrono::milliseconds pollingInterval{100};
  const QString intervalSettingName = "WeightPollIntervalMilliseconds";

  AdvancedHX711 *hx711 = createHX711(nullopt);

  static AdvancedHX711 *createHX711(optional<pair<int, int>> calibration);
  optional<double> valueGrams(const Options &options) const noexcept;
};

namespace {
LoadCellImpl *impl = nullptr;
}

LoadCell::LoadCell() {
  AssertSingleton();

  impl = new LoadCellImpl;

  timer = new QTimer;
  auto value = Settings::instance().value(impl->intervalSettingName, 1000).toInt();
  timer->setSingleShot(false);
  timer->start(value);
}

AdvancedHX711 *LoadCellImpl::createHX711(optional<pair<int, int>> newCalibrationData) {
  try {
    auto const gain = Gain::GAIN_128;
    // auto const gain = Gain::GAIN_64;

    // GAIN 128
    // calib 1 mouse62 -896, -46937
    // calib 2 mouse64 -867, -47022
    // calib 3 200tare -902, -47069
    // GAIN 64
    // calib1 200tare -452, -22906

    auto const defaultData =
        map<Gain, pair<int, int>>{
            {Gain::GAIN_128, {-902, -47069}},
            {Gain::GAIN_64, {-452, -22906}},
        }
            .at(gain);

    auto keyPrefix = QString{gain == Gain::GAIN_64 ? "CalibrationGain64" : "CalibrationGain128"};
    auto keyRefUnit = keyPrefix + "RefUnit";
    auto keyOffset = keyPrefix + "Offset";

    auto refUnit = 0;
    auto offset = 0;

    if (newCalibrationData.has_value()) {
      refUnit = newCalibrationData->first;
      offset = newCalibrationData->second;
    } else {
      refUnit = Settings::instance().value(keyRefUnit, defaultData.first).toInt();
      offset = Settings::instance().value(keyOffset, defaultData.second).toInt();
    }

    if (refUnit == 0) {  // INFs and NANs
      std::cout << "Invalid calibration: " << refUnit << " " << offset << ", using defaults" << std::endl;

      refUnit = defaultData.first;
      offset = defaultData.second;
    }
    Settings::instance().setValue(keyRefUnit, refUnit);
    Settings::instance().setValue(keyOffset, offset);

    std::cout << "Calibration Ref " << refUnit << " Offset " << offset << std::endl;
    // with 5th parameter non-default Rate::HZ_80 -> same results
    auto ret = new AdvancedHX711(5, 6, refUnit, offset, Rate::HZ_80);
    ret->setConfig(Channel::A, gain);
    ret->samplesInTimeOutMode = 1;  // this is my patch to libhx711: no busy wait
                                    // see more comments in install-libhx711.sh
    return ret;
  } catch (GpioException &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
  } catch (TimeoutException &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
  } catch (exception &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
  }
  return nullptr;
}

optional<LoadCell::Data> readInMode(const Options &options) noexcept {
  auto mass = impl->valueGrams(options);
  if (mass.has_value() == false) {
    return {};
  }

  auto value = *mass;
  // un-normalize
  auto reading = (value * impl->hx711->getReferenceUnit()) + impl->hx711->getOffset();

  return LoadCell::Data{value, reading};
}

optional<LoadCell::Data> LoadCell::read() noexcept {
  // short timeout, only one sample, no busy wait
  return readInMode(Options{impl->pollingInterval});
};

optional<double> LoadCell::readPreciseRaw() noexcept {
  // fixed number of samples, no timeout, busy wait
  if (auto ret = readInMode(Options{15})) {
    return ret->reading;
  }
  return {};
}

optional<double> LoadCellImpl::valueGrams(const Options &options) const noexcept {
  if (hx711 == nullptr) {
    return {};
  }

  try {
    return hx711->weight(options).getValue(Mass::Unit::G);
  } catch (GpioException e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " GpioException " << e.what() << endl;
  } catch (TimeoutException &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " TimeoutException " << e.what() << endl;
  } catch (exception &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
  }
  return {};
}

void LoadCell::recalibrate(std::pair<int, int> calibration) {
  delete (impl->hx711);
  impl->hx711 = impl->createHX711(calibration);
}
