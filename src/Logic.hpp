#pragma once

struct QTimer;

struct Logic {
  Logic();

  QTimer* timer = nullptr;

  void connect();
  void manualDispense();
};
