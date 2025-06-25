#pragma once

struct QApplication;
struct QSettings;
struct ToolBar;
struct MainWindow;
struct Weight;
struct CentralWidget;

struct Instance {
  static Instance* instance;

  Instance(int argc, char** argv);

  void connectSignals();

  QApplication* app = nullptr;
  QSettings* settings = nullptr;
  MainWindow* window = nullptr;
  Weight* weight = nullptr;
  CentralWidget* central = nullptr;
  ToolBar* toolbar = nullptr;
};
