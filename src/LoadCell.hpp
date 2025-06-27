#pragma once

#include <QTimer>
#include <optional>

struct LoadCell {
  LoadCell();

  void update() noexcept;

  struct Data {
    double value = 0;
    double reading = 0;
  };
  std::optional<Data> data;

  QTimer* timer = nullptr;
};
