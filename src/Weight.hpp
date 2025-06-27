#pragma once

struct LoadCell;
#include <QWidget>

class Weight : public QWidget {
 public:
  Weight(LoadCell*);
  void connect();
};
