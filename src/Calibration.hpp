#pragma once

struct QAbstractButton;
struct QShowEvent;

#include <QWidget>
#include <optional>

struct Calibration : public QWidget {
  Calibration();

  struct {
    QAbstractButton* back = nullptr;
  } buttons;

  void showEvent(QShowEvent* e) override;
  void connect();
  void update(std::optional<double> reading);
};
