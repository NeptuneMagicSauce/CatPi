#pragma once

struct QAbstractButton;
struct QShowEvent;

#include <QWidget>
#include <optional>
#include <utility>

struct Calibration : public QWidget {
  Calibration();

  struct {
    QAbstractButton* step1 = nullptr;
    QAbstractButton* step2 = nullptr;
    QAbstractButton* back = nullptr;
  } buttons;

  struct Callbacks {
    void step1(std::optional<double> rawPrecise);
    std::optional<std::pair<int, int>> step2(std::optional<double> rawPrecise);
  } callbacks;

  void showEvent(QShowEvent* e) override;
  void connect();
  void update(std::optional<double> reading);
};
