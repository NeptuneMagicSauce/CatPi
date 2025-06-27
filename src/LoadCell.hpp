#pragma once

#include <optional>

struct LoadCell {
  LoadCell();

  std::optional<double> valueGrams() const;
};
