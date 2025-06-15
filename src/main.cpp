#include <iostream>

#include <QAction>
#include <QApplication>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QShortcut>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QWidget>

int main(int argc, char** argv) {
  std::cout << "foo" << std::endl;


  QApplication a(argc, argv);
  QMainWindow w;
  w.show();

  return a.exec();

}
