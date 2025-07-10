#pragma once

struct QTimer;

#include <QString>
#include <QWidget>
#include <optional>

class Weight : public QWidget {
 public:
  Weight();

  const QString messageFinished;
  QTimer* eventTareFinished();

  void update(std::optional<double> value);
  double tare();
  std::optional<double> weightTarred();
};
