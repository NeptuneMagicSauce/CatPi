#pragma once

#include <QDateTime>
#include <QList>
#include <optional>

struct Event {
  QDateTime timeDispensed;
  std::optional<QDateTime> timeEaten;
};
