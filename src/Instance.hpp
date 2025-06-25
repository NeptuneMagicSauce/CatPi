#pragma once

struct QApplication;
struct QPushButton;
struct QTimer;
struct QString;
struct QLabel;
struct QSettings;
struct ToolBar;
struct MainWindow;
struct Weight;
struct CentralWidget;

struct Instance {
  Instance(int argc, char** argv);

  void connectSignals();

  QApplication* app = nullptr;
  QSettings* settings = nullptr;
  MainWindow* window = nullptr;
  Weight* weight = nullptr;
  CentralWidget* central = nullptr;
  ToolBar* toolbar = nullptr;

  struct {
    QPushButton* buttonDispense = nullptr;
  } action;
};
