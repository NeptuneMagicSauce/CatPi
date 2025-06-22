#include "PrintScreen.hpp"

#include <QGuiApplication>
#include <QRect>
#include <QScreen>
#include <iostream>
#include <string>

using namespace std;

void printScreen() {
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
