#pragma once

#include <QWidget>

struct CalibrationImpl;

struct Calibration : public QWidget {
  Calibration();
  CalibrationImpl* impl;
};
