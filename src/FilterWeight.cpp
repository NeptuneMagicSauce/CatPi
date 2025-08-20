#include "FilterWeight.hpp"

#include <algorithm>
#include <vector>

#include "Settings.hpp"
#include "System.hpp"

using namespace std;

namespace {
  vector<double> values;
  int indexSamples = 0;
  int sampleCount = 10 + 1;  // this algo works on odd counts
}

FilterWeight::FilterWeight() {
  AssertSingleton();
  Settings::load({"WeightFilterNbSamples",
                  "Filtre Poids Nb Samples",
                  "Nombre d'échantillons pour le filtre du poids",
                  "Nombre",
                  10,
                  [&](QVariant v) {
                    auto newCount = v.toInt();
                    if ((newCount % 2) == 0) {
                      ++newCount;  // must be odd
                    }
                    sampleCount = newCount;

                    indexSamples = 0;
                    values.clear();
                  },
                  {1, 30}});
}

double FilterWeight::update(double value) {
  // invariant: the vector values is always full of values

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
