#pragma once

#include <memory>
#include "WeightProgram.hpp"

struct QApplication;
struct QPushButton;
struct QTimer;
struct QString;
struct QLabel;
struct ToolBar;
struct MainWindow;

struct Instance
{
  Instance(int argc, char** argv);
  void connectSignals();

  QApplication* app = nullptr;
  MainWindow* window = nullptr;
  ToolBar* toolbar = nullptr;
  bool const isSmallScreen = false;

  struct {
    QPushButton* buttonDispense = nullptr;
  } action;

  struct {
    QTimer* watcher = nullptr;
    QString* measure = nullptr;
    QLabel* label = nullptr;
    std::unique_ptr<WeightProgram> program;
  } weight;
};
