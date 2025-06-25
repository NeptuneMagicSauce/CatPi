#pragma once

struct QApplication;
struct ToolBar;
struct MainWindow;
struct Weight;
struct CentralWidget;

struct Instance {
  Instance(int argc, char** argv);

  void connectSignals();

  QApplication* app = nullptr;
  MainWindow* window = nullptr;
  Weight* weight = nullptr;
  CentralWidget* central = nullptr;
  ToolBar* toolbar = nullptr;
};
