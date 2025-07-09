#include "CrashTester.hpp"

#include <QGridLayout>
#include <QPushButton>
#include <csignal>
#include <iostream>
#include <map>

#include "Widget.hpp"

using namespace std;

namespace {
  enum struct Type {
    NullPtr,
    Assert,
    StdContainerAccess,
    DivByZeroInteger,
    FloatInf,
    FloatNaN,
    ThrowEmpty,
    Exception,
    Terminate,
    Abort,
    SigSegv,
    SigAbort,
    SigFPE,
    SigIllFormed,
    SigInterrupt,
    SigTerminate,
  };

  void TestThis(Type type) {
    switch (type) {
      case Type::NullPtr:
        cout << *(volatile int*)nullptr;
        break;
      case Type::Assert:
        assert(false);
        break;
      case Type::StdContainerAccess:
        cout << vector<int>{}.at(0);
        break;
      case Type::DivByZeroInteger:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdivision-by-zero"
        cout << 1 / 0 << endl;
#pragma clang diagnostic pop
        break;
      case Type::FloatInf:
        [] {
          auto one = 1.0;
          cout << one / 0.0 << endl;
        }();
        break;
      case Type::FloatNaN:
        cout << sqrt(-1.0) << endl;
        break;
      case Type::ThrowEmpty:
        throw;
      case Type::Exception:
        throw runtime_error{""};
      case Type::Terminate:
        terminate();
      case Type::Abort:
        abort();
      case Type::SigSegv:
        raise(SIGSEGV);
        break;
      case Type::SigAbort:
        raise(SIGABRT);
        break;
      case Type::SigFPE:
        raise(SIGFPE);
        break;
      case Type::SigIllFormed:
        raise(SIGILL);
        break;
      case Type::SigInterrupt:
        raise(SIGINT);
        break;
      case Type::SigTerminate:
        raise(SIGTERM);
        break;
    }
  }
}

CrashTester::CrashTester()
    : QDialog(nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint) {
  hide();
  setWindowTitle("CrashTester");
  setModal(true);
  Widget::FontSized(this, 20);
  auto layout = new QGridLayout;
  setLayout(layout);
  auto index = 0;
  auto buttonsPerRow = 3;
  const std::map<Type, QString> signalNames = {
      {Type::NullPtr, "NullPtr"},
      {Type::Assert, "Assert"},
      {Type::StdContainerAccess, "StdContainerAccess"},
      {Type::DivByZeroInteger, "DivByZeroInteger"},
      {Type::FloatInf, "FloatInf"},
      {Type::FloatNaN, "FloatNaN"},
      {Type::ThrowEmpty, "ThrowEmpty"},
      {Type::Exception, "Exception"},
      {Type::Terminate, "Terminate"},
      {Type::Abort, "Abort"},
      {Type::SigSegv, "SigSegv"},
      {Type::SigAbort, "SigAbort"},
      {Type::SigFPE, "SigFPE"},
      {Type::SigIllFormed, "SigIllegal"},
      {Type::SigInterrupt, "SigInterrupt"},
      {Type::SigTerminate, "SigTerminate"},
  };
  for (auto type : signalNames) {
    auto button = new QPushButton{type.second};
    QObject::connect(button, &QAbstractButton::released, [=] { TestThis(type.first); });
    layout->addWidget(button, index / buttonsPerRow, index % buttonsPerRow);
    ++index;
  }
}
