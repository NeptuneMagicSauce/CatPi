#pragma once

#include <QMainWindow>

struct MainWindow : public QMainWindow {
  MainWindow();
  bool const isSmallScreen;

  void toggleFullscreen(bool);

  //   void resizeEvent(QResizeEvent* event) override {
  //     QMainWindow::resizeEvent(event);
  //     std::cout << event->size().width() << "/" << event->size().height()
  //               << std::endl;
  //   }
};
