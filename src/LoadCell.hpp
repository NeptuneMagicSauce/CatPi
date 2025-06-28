#pragma once

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

  void recalibrate(std::pair<int, int>);

  QTimer* timer = nullptr;
};
