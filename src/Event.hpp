#pragma once

#include <QDateTime>
#include <QList>
#include <optional>

struct Event {
  QDateTime timeDispensed;
  double grams;
  std::optional<QDateTime> timeEaten;
};
