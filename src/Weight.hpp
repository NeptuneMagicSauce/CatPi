#pragma once

struct QWidget;
struct Instance;
struct WeightImpl;

class Weight {
 public:
  Weight(Instance*);
  QWidget* widget() const;
  void connect();  // const
 private:
  WeightImpl* impl;
};
