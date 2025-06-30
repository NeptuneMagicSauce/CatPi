#pragma once

#include <QWidget>

struct Debug : public QWidget {
  Debug();
  static bool Populated();
};
