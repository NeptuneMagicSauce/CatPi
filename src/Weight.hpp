#pragma once

#include <QWidget>
struct WeightImpl;

class Weight : public QWidget {
 public:
  Weight();
  void connect();

 private:
  WeightImpl* impl;
};
