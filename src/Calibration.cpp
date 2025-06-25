#include "Calibration.hpp"

#include <QLabel>
#include <QVBoxLayout>

struct CalibrationImpl {};

Calibration::Calibration() : impl(new CalibrationImpl) {
  auto layout = new QVBoxLayout();
  layout->addWidget(new QLabel("foo"));
  setLayout(layout);
}
