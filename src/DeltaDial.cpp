#include "DeltaDial.hpp"

DeltaDial::DeltaDial() {
  setMaximum(30);
  setWrapping(true);
}

void DeltaDial::connect() {
  QObject::connect(this, &QAbstractSlider::valueChanged, [&](auto value) {
    delta = value - oldValue;
    auto const firstQuarter = maximum() / 4;
    auto const lastQuarter = (maximum() * 3) / 4;
    if ((oldValue < firstQuarter && value > lastQuarter) ||
        (oldValue > lastQuarter && value < firstQuarter)) {
      // just wrapped on the other size of zero <-> delta is invalid
      delta = 0;
    }

    oldValue = value;
  });
}
