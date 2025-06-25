#pragma once

struct QApplication;
struct QSettings;
struct ToolBar;
struct MainWindow;
struct Weight;
struct CentralWidget;

struct Instance {
  Instance(int argc, char** argv);

  void connectSignals();

  static QSettings& settings();

  QApplication* app = nullptr;
  MainWindow* window = nullptr;
  Weight* weight = nullptr;
  CentralWidget* central = nullptr;
  ToolBar* toolbar = nullptr;
};
