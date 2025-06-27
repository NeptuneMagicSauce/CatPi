#pragma once

struct QAbstractButton;
struct QShowEvent;

#include <QWidget>

struct Calibration : public QWidget {
  Calibration();

  struct {
    QAbstractButton* back = nullptr;
  } buttons;

  void showEvent(QShowEvent* e) override;
  void connect();
};
