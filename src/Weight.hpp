#pragma once

struct LoadCell;

#include <QWidget>
#include <optional>

class Weight : public QWidget {
 public:
  Weight(LoadCell*);

  void connect();
  void update(std::optional<double> value);
};
