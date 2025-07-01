#pragma once

#include <QScrollArea>

struct Debug : public QScrollArea {
  Debug();
  static bool Populated();
};
