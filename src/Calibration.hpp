#pragma once

struct QAbstractButton;
struct QShowEvent;

#include <QWidget>

struct Calibration : public QWidget {
  Calibration();

  struct {
    QAbstractButton* step1 = nullptr;
    QAbstractButton* step2 = nullptr;
    QAbstractButton* back = nullptr;
  } buttons;
  void showEvent(QShowEvent* e) override;
  struct Callbacks {
    void step1();
    void step2();
  } callbacks;
};
