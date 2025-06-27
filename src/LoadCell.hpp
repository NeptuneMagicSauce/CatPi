#pragma once

#include <optional>

struct LoadCell {
  LoadCell();

  std::optional<double> valueGrams() const noexcept;
  std::optional<double> reading() const noexcept;  // the decimal value uninterpreted
};
