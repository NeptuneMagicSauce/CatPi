#pragma once

struct LoadCell;
struct QTimer;

#include <QString>
#include <QWidget>
#include <optional>

class Weight : public QWidget {
 public:
  Weight(LoadCell*);

  const QString messageFinished;
  QTimer* eventTareFinished();

  void connect();
  double update(std::optional<double> value);
  double tare();
};
