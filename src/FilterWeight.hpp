#include <optional>

struct FilterWeight {
  FilterWeight();
  double update(double value);
  std::optional<double> value() const;
  bool isInitializingFinished() const;
};
