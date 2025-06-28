#include "Logic.hpp"

// #include <iostream>
#include <QTimer>

#include "PinCtrl.hpp"
#include "System.hpp"

using namespace std;
using PinCtrl::pinctrl;

struct LogicImpl {
  LogicImpl();
};

namespace {
LogicImpl* impl = nullptr;
}

Logic::Logic() {
  AssertSingleton();
  timer = new QTimer;
  timer->setSingleShot(true);
  timer->setInterval(4000);
  impl = new LogicImpl;
}

LogicImpl::LogicImpl() {}

void Logic::connect() {
  QObject::connect(timer, &QTimer::timeout, [&] { pinctrl("set 17 op dl"); });
}

void Logic::manualDispense() {
  // std::cout << "released" << std::endl;
  // pinctrl("-p");
  pinctrl("set 17 op dh");
  timer->start();
}
