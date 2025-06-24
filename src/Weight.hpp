#pragma once

struct QTimer;
struct QLabel;
struct QWidget;
struct Instance;
namespace HX711 {
struct AdvancedHX711;
}

class Weight {
 public:
  Weight(Instance*);
  QWidget* widget() const;
  void connect();  // const
 private:
  QTimer* timer = nullptr;
  QLabel* label = nullptr;
  double massGrams = 0;
  HX711::AdvancedHX711* hx711 = nullptr;
};
