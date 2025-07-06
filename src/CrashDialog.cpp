#include "CrashDialog.hpp"

#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <map>
#include <string>

#include "CrashHandler.hpp"
#include "Widget.hpp"

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
  Widget::FontSized(this, 20);
  auto layout = new QGridLayout;
  setLayout(layout);
  auto index = 0;
  auto buttonsPerRow = 3;
  for (auto type : signalNames) {
    auto button = new QPushButton{type.second.c_str()};
    QObject::connect(button, &QAbstractButton::released,
                     [=] { CrashHandler::Test::This(type.first); });
    layout->addWidget(button, index / buttonsPerRow, index % buttonsPerRow);
    ++index;
  }
}

void CrashDialog::ShowStackTrace(const QString& error, const QString& stack) {
  // if QApplication is not constructed, construct it because QDialog::exec() needs it
  if (qApp == nullptr) {
    new QApplication(*(new int(0)), (char**)nullptr);
  }

  auto dialog = new QDialog{nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                                         Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint};
  Widget::FontSized(dialog, 20);
  dialog->setModal(true);
  dialog->setWindowTitle("Crash");
  auto layout_root = new QVBoxLayout;
  dialog->setLayout(layout_root);
  auto error_label = new QLabel{"**" + error + "**"};
  error_label->setTextFormat(Qt::MarkdownText);
  layout_root->addWidget(Widget::AlignCentered(error_label));

  auto button_quit = new QPushButton{"Quit"};
  button_quit->setDefault(true);
  QObject::connect(button_quit, &QPushButton::released, [&dialog]() { dialog->accept(); });
  auto pid_string = QString::number(QApplication::applicationPid());

  auto stack_label = new QLabel;
  stack_label->setTextFormat(Qt::MarkdownText);
  auto stack_text = QString{};
  stack_text += "**Process ID** " + pid_string + "\n";
  stack_text += "**Stack Trace**\n";

#warning "TODO here"
  // fancy format the stack (markdown)
  // highlight lines in my code
  for (auto stackline : stack.split("\n")) {
    static auto const patterns =
        QList<QString>{{"/c/Devel/Workspace/CatPi/"}, {"/home/romain/CatPi/"}};
    auto isUserCode = false;
    for (auto const& p : patterns) {
      if (stackline.contains(p)) {
        stackline.remove(p);
        isUserCode = true;
      }
    }
    stack_text += stackline + "\n";
  }

  stack_text.replace("\n", "\n\n");
  stack_label->setText(stack_text.trimmed());

  auto stack_area = new QScrollArea;
  stack_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  stack_area->setWidget(stack_label);
  stack_area->setWidgetResizable(true);
  layout_root->addWidget(button_quit);
  layout_root->addWidget(stack_area);

  dialog->resize(800, 500);
  dialog->exec();
}
