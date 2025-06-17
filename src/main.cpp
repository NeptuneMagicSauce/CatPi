#include <cstdint>
#include <iostream>

#include <QScreen>
#include <QAction>
#include <QApplication>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QResizeEvent>
#include <QPushButton>
#include <QShortcut>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QWidget>
#include <memory>
#include <qaction.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qnamespace.h>
#include <qwidget.h>
#include <string>
#include <type_traits>

using namespace std;

struct ToolBar;
struct MainWindow;

struct Impl
{
  Impl(int argc, char** argv);
  void connectSignals();


  QApplication* app = nullptr;
  MainWindow* w;
  ToolBar* toolbar = nullptr;
  bool const isSmallScreen;
};
namespace {
  std::unique_ptr<Impl> instance;
  }

struct MainWindow: public QMainWindow {
//   void resizeEvent(QResizeEvent* event) override {
//     QMainWindow::resizeEvent(event);
//     std::cout << event->size().width() << "/" << event->size().height()
//               << std::endl;
//   }
};

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

auto printDesktop() {
  auto s = QGuiApplication::primaryScreen();

  auto p = [] (string name, auto i) {
    return name + string(" ") + to_string(i);
  };
  auto pSize = [] (string name, QSize i) {
    return name + string(" ") + to_string(i.width()) + "/" + to_string(i.height());
  };
  auto pRect = [] (string name, QRect i) {
    return name + string(" ") + to_string(i.left()) + "/" + to_string(i.top()) + "/"
+ to_string(i.right()) + "/" + to_string(i.bottom())
      ;
  };
  std::cout << "[ " << s->name().toStdString() << " ][ "
            << s->manufacturer().toStdString() << " ][ "
            << s->model().toStdString() << " ][ "
            << s->serialNumber().toStdString() << " ][ "
            << p("depth", s->depth()) << " ][ " << pSize("size", s->size())
            << " ][ " << pRect("geometry", s->geometry()) << " ][ "
            << "physicalSize " << s->physicalSize().width() << "/"
            << s->physicalSize().height() << " ][ "
            << "physicalDotsPerInch " << s->physicalDotsPerInch() << " ][ "
            << "logicalDotsPerInch " << s->logicalDotsPerInch() << " ][ "
            << "devicePixelRatio " << s->devicePixelRatio() << " ][ "
            << pSize("availableSize", s->availableSize()) << " ][ "
            << pRect("availableGeometry", s->availableGeometry()) << " ][ "
            << pSize("virtualSize ", s->virtualSize()) << " ][ "
            << pRect("virtualGeom ", s->virtualGeometry()) << " ][ "
            << pSize("availVirtualSize ", s->availableVirtualSize()) << " ][ "
            << pRect("availVirtualGeom ", s->availableVirtualGeometry())
            << " ][ " << p("primaryOrientation ", (int)s->primaryOrientation())
            << " ][ " << p("orientation ", (int)s->orientation()) << " ][ "
            << p("nativeOrientation ", (int)s->nativeOrientation()) << " ][ "
            << " ] " <<
      std::endl;
}

int main(int argc, char **argv) {
  instance = std::make_unique<Impl>(argc, argv);
  instance->connectSignals();
  return instance->app->exec();
}

Impl::Impl(int argc, char **argv)
    : app(new QApplication(argc, argv)), w(new MainWindow),
      isSmallScreen(QGuiApplication::primaryScreen()->geometry().height() <= 720) {
  if (isSmallScreen) {
    w->setWindowState(Qt::WindowMaximized);
  }
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
