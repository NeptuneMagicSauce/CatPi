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

struct Instance {
  Instance(int argc, char** argv);

  void connectSignals();

  QApplication* app = nullptr;
  MainWindow* window = nullptr;
  ToolBar* toolbar = nullptr;
  QSettings* settings = nullptr;
  bool const isSmallScreen = false;

  struct {
    QPushButton* buttonDispense = nullptr;
  } action;

  Weight* weight = nullptr;
};
