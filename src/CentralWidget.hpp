#pragma once

#include <QWidget>

struct Instance;

struct CentralWidget : public QWidget {
  CentralWidget(Instance*);
};
