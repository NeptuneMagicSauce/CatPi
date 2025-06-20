#include <cstdint>
#include <iostream>

#include <QFile>
#include <QStandardPaths>
#include <QProcess>
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

#include <ranges>

using namespace std;

struct ToolBar;
struct MainWindow;

struct Impl
{
  Impl(int argc, char** argv);
  void connectSignals();


  QApplication* app = nullptr;
  MainWindow* w = nullptr;
  ToolBar* toolbar = nullptr;
  bool const isSmallScreen;

  struct {
    QPushButton* buttonDispense = nullptr;
  } panelAction;

  struct {
    QTimer watcher;
    QString measure;
    QLabel label;
  } weight;
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
  Central(Impl* instance);
};

struct ToolBar: public QToolBar {
  ToolBar();
  QAction* quit = nullptr;
  QAction* fullscreen = nullptr;
};

extern "C" {
  // that's the raspberry-utils program pinctrl as a lib
  int main_pinctrl(int argc, char *argv[]);
}

void pinctrl(string_view sv) {
  // split on blank space
  auto splitted = sv
    | views::split(' ')
    | views::transform([](auto i) { return string{string_view{i}}; });
  // to vector<char*>
  vector<const char*> char_args = { "pinctrl" };;
  ranges::transform(
    vector<string>{ splitted.begin(), splitted.end() },
    back_inserter(char_args),
    [] (auto& i) { return i.data(); });
  // // debug print
  // ranges::for_each(char_args, [](auto i) { cout << (void*)i << " " << i << endl; });
  main_pinctrl(char_args.size(), (char**)char_args.data());
}

void Impl::connectSignals() {
  QObject::connect(
    instance->toolbar->quit, &QAction::triggered,
    instance->app,  &QApplication::quit);
  QObject::connect(
    instance->toolbar->fullscreen, &QAction::toggled, [](bool checked) {
      if (checked) {
        instance->w->showFullScreen();
      } else {
        instance->w->showNormal();
        if (instance->isSmallScreen) {
          instance->w->setWindowState(Qt::WindowMaximized);
        }
      }
    });
  QObject::connect(
    instance->panelAction.buttonDispense,
    &QPushButton::released,
    []() {
      // std::cout << "released" << std::endl;
      // pinctrl("-p");
      pinctrl("set 17 op dh");
      instance->panelAction.buttonDispense->setEnabled(false);
      QTimer::singleShot(4000, []() {
        pinctrl("set 17 op dl");
        instance->panelAction.buttonDispense->setEnabled(true);
      });
    });
  QObject::connect(
    &instance->weight.watcher,
    &QTimer::timeout,
    [] () {
      auto file = QFile("/home/pi/weights.measures");
      file.open(QIODeviceBase::ReadOnly);
      auto str = file.readAll().trimmed();
      instance->weight.measure = str;
      instance->weight.label.setText(str + " grams");
      // std::cout << instance->weight.measure.toStdString() << endl;
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

  qint64 pid = 0;
  auto measureProcess = QProcess();

  auto const home =
    QStandardPaths::standardLocations(
      QStandardPaths::StandardLocation::HomeLocation).first();

  measureProcess.setProgram(home + "/examples/min.py");
  measureProcess.setArguments({ "-q" });
  measureProcess.startDetached(&pid);
  std::cout << "measure process pid " << pid << endl;
  
  auto ret = instance->app->exec();

  // TODO do this clean-up when Ctrl-C
  measureProcess.setProgram("kill");
  measureProcess.setArguments({ QString::number(pid) });
  measureProcess.startDetached();

  return ret;
}
Impl::Impl(int argc, char **argv)
    : app(new QApplication(argc, argv)),
      w(new MainWindow),
      isSmallScreen(QGuiApplication::primaryScreen()->geometry().height() <= 720)
{
  w->setCentralWidget(new Central(this));
  toolbar = new ToolBar();
  w->addToolBar(Qt::LeftToolBarArea, toolbar);
  app->setStyleSheet("QLabel{font-size: 48pt;} QAbstractButton{font-size: 48pt;} ");
  
  weight.watcher.setSingleShot(false);
  weight.watcher.start(1000);

  if (isSmallScreen) {
    w->showFullScreen();
    toolbar->fullscreen->setChecked(true);
  } else {
    w->show();
  }
}

Central::Central(Impl* instance) {
  auto layout = new QHBoxLayout(this);
  auto& b = instance->panelAction.buttonDispense;
  b = new QPushButton("Now!");
  b->setSizePolicy({QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding});
  instance->weight.label.setText("--");
  layout->addWidget(&instance->weight.label);
  layout->addWidget(b);
}

ToolBar::ToolBar() {

  setMovable(false);
  setFloatable(false);
  setContextMenuPolicy(Qt::ContextMenuPolicy::PreventContextMenu);
  setFixedWidth(90);
  setIconSize({90, 90});

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
