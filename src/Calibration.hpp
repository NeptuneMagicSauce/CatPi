#pragma once

struct QAbstractButton;
#include <QWidget>

struct Calibration : public QWidget {
  Calibration();

  struct {
    QAbstractButton* step1 = nullptr;
    QAbstractButton* step2 = nullptr;
    QAbstractButton* back = nullptr;
  } buttons;

  struct Callbacks {
    void step1();
    void step2();
  } callbacks;
};
