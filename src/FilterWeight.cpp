#include "FilterWeight.hpp"

// #include <QElapsedTimer>
#include <algorithm>
#include <vector>

#include "Settings.hpp"
#include "System.hpp"

using namespace std;

namespace {
  vector<double> values;
  int indexSamples = 0;
  int sampleCount = 10 + 1;  // this algo works on odd counts
  int numberOfCallsToFillBuffer = 0;
}

FilterWeight::FilterWeight() {
  AssertSingleton();
  Settings::load({.key = "WeightFilterNbSamples",
                  .name = "Filtre Poids Nb Samples",
                  .prompt = "Nombre d'échantillons pour le filtre du poids",
                  .unit = "Nombre",
                  .defaultValue = 6,
                  // value 6 is good compromise between stability and speed of measuring
                  .callback =
                      [&](QVariant v) {
                        auto newCount = v.toInt();
                        if ((newCount % 2) == 0) {
                          ++newCount;  // must be odd
                        }
                        sampleCount = newCount;

                        indexSamples = 0;
                        values.clear();
                      },
                  .limits = {.minimum = 1, .maximum = 30}});
}

bool FilterWeight::isInitializingFinished() const {
  return numberOfCallsToFillBuffer >= sampleCount;
}

double FilterWeight::update(double value) {
  // static auto t = QElapsedTimer{};
  // qDebug() << "                                   ### Filter" << value << "samples" <<
  // sampleCount
  // << "ms" << t.elapsed();
  // t.restart();

  // invariant: the vector values is always full of values

  if (isInitializingFinished() == false) {
    ++numberOfCallsToFillBuffer;
  }

  // if empty, initialize it with the first value
  if (values.empty()) {
    values.resize(sampleCount);
    for (auto& v : values) {
      v = value;
    }
    indexSamples = 1;
    return value;
  }

  values[indexSamples++] = value;
  if (indexSamples >= sampleCount) {
    indexSamples = 0;
  }

  return this->value().value();
}

std::optional<double> FilterWeight::value() const {
  if (values.empty()) {
    return {};
  }

  // https://stackoverflow.com/a/1719155
  auto v = values;  // make a copy because it is modified by nth_element
  auto n = v.size() / 2;
  nth_element(v.begin(), v.begin() + n, v.end());
  return v[n];
}
