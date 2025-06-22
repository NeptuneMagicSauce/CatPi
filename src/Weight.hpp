#pragma once

struct QTimer;
struct QLabel;
struct QWidget;
struct Instance;
namespace HX711 {
struct SimpleHX711;
}

class Weight {
public:
  Weight();
  QWidget* widget() const;
  void connect(); // const
private:  
  QTimer* timer = nullptr;
  QLabel* label = nullptr;
  HX711::SimpleHX711* hx711 = nullptr;
};
