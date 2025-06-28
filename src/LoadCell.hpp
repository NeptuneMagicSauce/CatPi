#pragma once

struct QString;
#include <QTimer>
#include <optional>

struct LoadCell {
  LoadCell();

  struct Data {
    double value = 0;
    double reading = 0;
  };

  std::optional<Data> read() noexcept;
  std::optional<double> readPreciseRaw() noexcept;

  void recalibrate(std::pair<int, int> calibration, QString& status);

  QTimer* timer = nullptr;
};
