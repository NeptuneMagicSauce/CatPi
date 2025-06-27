#include "LoadCell.hpp"

#include <cassert>
#include <iostream>
#include <map>

#include "AdvancedHX711.h"
#include "GpioException.h"
// #include "Mass.h"
#include "System.hpp"
#include "TimeoutException.h"

using namespace HX711;
using namespace std;

struct LoadCellImpl {
  static AdvancedHX711 *tryCreateHX711();

  std::chrono::milliseconds pollingInterval{100};
  optional<double> valueGrams() const;

  AdvancedHX711 *hx711 = tryCreateHX711();
};

namespace {
LoadCellImpl *impl = nullptr;
}

LoadCell::LoadCell() {
  AssertSingleton();
  impl = new LoadCellImpl;
}

AdvancedHX711 *LoadCellImpl::tryCreateHX711() {
  try {
    auto const gain = Gain::GAIN_128;
    // auto const gain = Gain::GAIN_64;

    // GAIN 128
    // calib 1 mouse62 -896, -46937
    // calib 2 mouse64 -867, -47022
    // calib 3 200tare -902, -47069
    // GAIN 64
    // calib1 200tare -452, -22906
    auto const calibrationData = map<Gain, pair<int, int>>{
        {Gain::GAIN_128, {-902, -47069}},
        {Gain::GAIN_64, {-452, -22906}},
    };
    auto const calibration = calibrationData.at(gain);
    // with 5th parameter non-default Rate::HZ_80 -> same results
    auto ret = new AdvancedHX711(5, 6, calibration.first, calibration.second, Rate::HZ_80);
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

// TODO fix emacs TAB goes to next error in compile

optional<double> LoadCell::valueGrams() const { return impl->valueGrams(); }
optional<double> LoadCellImpl::valueGrams() const {
  if (hx711 == nullptr) {
    return {};
  }

  auto mass = optional<Mass>{};
  try {
    mass = hx711->weight(pollingInterval);
  } catch (GpioException e) {
    std::cerr << __FILE__ << " GpioException " << e.what() << endl;
    return {};
  } catch (TimeoutException &e) {
    std::cerr << __FILE__ << " TimeoutException " << e.what() << endl;
    return {};
  } catch (exception &e) {
    std::cerr << __FILE__ << " " << e.what() << endl;
    return {};
  }
  return mass->getValue(Mass::Unit::G);
}
