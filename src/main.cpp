#include <cstdint>
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
#include <memory>
#include <qaction.h>
#include <qapplication.h>
#include <qnamespace.h>
#include <type_traits>

struct ToolBar;

struct Impl
{
  Impl(int argc, char** argv);
  void connectSignals();

  QApplication* app = nullptr;
  QMainWindow* w = new QMainWindow;
  ToolBar* toolbar = nullptr;
};
namespace {
  std::unique_ptr<Impl> instance;
}

struct Central : public QWidget {
  Central();
};

struct ToolBar: public QToolBar {
  ToolBar();
  QAction* quit = nullptr;
  QAction* fullscreen = nullptr;
};

void Impl::connectSignals() {
  QObject::connect(
    instance->toolbar->quit, &QAction::triggered,
    instance->app, &QApplication::quit);
  QObject::connect(
    instance->toolbar->fullscreen, &QAction::toggled, [] (bool checked) {
      if (checked) {
        instance->w->showFullScreen();
      } else {
        instance->w->showNormal();
      }

    });
}

int main(int argc, char **argv) {
  instance = std::make_unique<Impl>(argc, argv);
  instance->connectSignals();
  return instance->app->exec();
}

Impl::Impl(int argc, char** argv) : app(new QApplication(argc, argv)) {
  w->setCentralWidget(new Central());
  toolbar = new ToolBar();
  w->addToolBar(Qt::LeftToolBarArea, toolbar);
  w->show();
}

Central::Central() {
  auto layout = new QHBoxLayout(this);
  layout->addWidget(new QLabel("foo"));
  layout->addWidget(new QLabel("bar"));
}

ToolBar::ToolBar() {

  setMovable(false);
  setFloatable(false);
  setContextMenuPolicy(Qt::ContextMenuPolicy::PreventContextMenu);
  // setFixedWidth(30);

  quit = new QAction();
  quit->setIcon(instance->app->style()->standardIcon(QStyle::StandardPixmap::SP_TabCloseButton));
  quit->setText("Quit");
  addAction(quit);

  fullscreen = new QAction();
  fullscreen->setIcon(instance->app->style()->standardIcon(QStyle::StandardPixmap::SP_DesktopIcon));
  fullscreen->setText("Fullscreen");
  fullscreen->setCheckable(true);
  addAction(fullscreen);
}
