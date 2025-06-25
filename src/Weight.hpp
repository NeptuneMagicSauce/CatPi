#pragma once

struct QWidget;
struct WeightImpl;

class Weight {
 public:
  Weight();
  QWidget* widget() const;
  void connect();  // const
 private:
  WeightImpl* impl;
};
