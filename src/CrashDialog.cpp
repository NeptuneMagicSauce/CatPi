#include "CrashDialog.hpp"

#include <QGridLayout>
#include <QPushButton>
#include <map>
#include <stacktrace>
#include <string>

#include "CrashHandler.hpp"

using namespace std;

namespace {
  const map<CrashHandler::Test::Type, string> signalNames = {
      {CrashHandler::Test::Type::NullPtr, "NullPtr"},
      {CrashHandler::Test::Type::Assert, "Assert"},
      {CrashHandler::Test::Type::StdContainerAccess, "StdContainerAccess"},
      {CrashHandler::Test::Type::DivByZeroInteger, "DivByZeroInteger"},
      {CrashHandler::Test::Type::FloatInf, "FloatInf"},
      {CrashHandler::Test::Type::FloatNaN, "FloatNaN"},
      {CrashHandler::Test::Type::ThrowEmpty, "ThrowEmpty"},
      {CrashHandler::Test::Type::Exception, "Exception"},
      {CrashHandler::Test::Type::Terminate, "Terminate"},
      {CrashHandler::Test::Type::Abort, "Abort"},
      {CrashHandler::Test::Type::SigSegv, "SigSegv"},
      {CrashHandler::Test::Type::SigAbort, "SigAbort"},
      {CrashHandler::Test::Type::SigFPE, "SigFPE"},
      {CrashHandler::Test::Type::SigIllFormed, "SigIllFormed"},
      {CrashHandler::Test::Type::SigInterrupt, "SigInterrupt"},
      {CrashHandler::Test::Type::SigTerminate, "SigTerminate"},
  };
}

CrashTester::CrashTester()
    : QDialog(nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint) {
  hide();
  setWindowTitle("CrashTester");
  setModal(true);
  auto layout = new QGridLayout;
  setLayout(layout);
  auto index = 0;
  auto buttonsPerRow = 3;
  for (auto type :
       {CrashHandler::Test::Type::NullPtr, CrashHandler::Test::Type::Assert,
        CrashHandler::Test::Type::StdContainerAccess, CrashHandler::Test::Type::DivByZeroInteger,
        CrashHandler::Test::Type::FloatInf, CrashHandler::Test::Type::FloatNaN,
        CrashHandler::Test::Type::ThrowEmpty, CrashHandler::Test::Type::Exception,
        CrashHandler::Test::Type::Terminate, CrashHandler::Test::Type::Abort,
        CrashHandler::Test::Type::SigSegv, CrashHandler::Test::Type::SigAbort,
        CrashHandler::Test::Type::SigFPE, CrashHandler::Test::Type::SigIllFormed,
        CrashHandler::Test::Type::SigInterrupt, CrashHandler::Test::Type::SigTerminate}) {
    auto button = new QPushButton{signalNames.at(type).c_str()};
    QObject::connect(button, &QAbstractButton::released,
                     [type] { CrashHandler::Test::This(type); });
    layout->addWidget(button, index / buttonsPerRow, index % buttonsPerRow);
    ++index;
  }
}
