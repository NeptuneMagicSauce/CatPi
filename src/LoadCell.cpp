#include "LoadCell.hpp"

#include <QTimer>
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
  optional<double> valueGrams() const noexcept;
  optional<double> reading() const noexcept;

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

optional<double> LoadCell::valueGrams() const noexcept { return impl->valueGrams(); }
optional<double> LoadCellImpl::valueGrams() const noexcept {
  if (hx711 == nullptr) {
    return {};
  }

  try {
    return hx711->weight(pollingInterval).getValue(Mass::Unit::G);
  } catch (GpioException e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " GpioException " << e.what() << endl;
  } catch (TimeoutException &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " TimeoutException " << e.what() << endl;
  } catch (exception &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
  }
  return {};
}

optional<double> LoadCell::reading() const noexcept { return impl->reading(); }
optional<double> LoadCellImpl::reading() const noexcept {
  try {
    auto ret = hx711->read(Options{pollingInterval});
    // un-normalize
    return (ret * hx711->getReferenceUnit()) + hx711->getOffset();
  } catch (GpioException e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " GpioException " << e.what() << endl;
  } catch (TimeoutException &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " TimeoutException " << e.what() << endl;
  } catch (exception &e) {
    std::cerr << __FILE__ << ":" << __LINE__ << " " << e.what() << endl;
  }
  return {};
}
