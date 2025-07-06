#include "CrashDialog.hpp"

#include <QApplication>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <iostream>
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

namespace {
  QString formatStackItem(const QString& line, bool isUserCode) {
    //   5# main at Main.cpp:30
    //   6# __libc_start_call_main at ../sysdeps/nptl/libc_start_call_main.h:58

    static auto iswhitespace = [](const QString& str) {
      auto regexp = QRegularExpression{"^ *$"};
      return regexp.match(str).hasMatch();
    };
    static auto bold = [](const QString& str) {
      if (iswhitespace(str)) {
        return str;
      }
      return QString{"**"} + str + "**";
    };
    static auto italic = [](const QString& str) {
      if (iswhitespace(str)) {
        return str;
      }
      return QString{"*"} + str + "*";
    };
    static auto formatIndex = [](const QString& index) {
      return bold("[" + QString::number(index.toInt()) + "]");
    };

    try {
      auto regexep = QRegularExpression{"(^ *\\d*)(# )(.*)( at )(.*)"};
      auto match = regexep.match(line);
      if (match.hasMatch() == false) {
        auto regexpIndexOnly = QRegularExpression{"(^ *\\d*)(#.*)"};
        auto matchIndexOnly = regexpIndexOnly.match(line);
        if (matchIndexOnly.hasMatch()) {
          return formatIndex(matchIndexOnly.captured(1));
        }
        return line;
      }
      auto index = match.captured(1);
      auto function = match.captured(3);
      auto location = match.captured(5);
      // qDebug() << index << function << location;

      auto ret = QString{};

      ret += formatIndex(index) + " ";

      function = italic(function);
      ret += isUserCode ? bold(function) : function;

      ret += "\n";

      if (location.isEmpty() || location == ":0") {
        ret += "--";
      } else {
        ret += isUserCode ? bold(location) : location;
        // ret += location;
      }
      ret += " ";
      return ret;
    } catch (exception&) {
      return line;
    }
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

  auto lines = stack.split("\n");
  for (auto stackline : lines) {
    static auto const patterns =
        // TODO use cmake build dir and realpath, do not leak personal info
        QList<QString>{{"/c/Devel/Workspace/CatPi/"}, {"/home/romain/CatPi/"}};
    auto isUserCode = false;
    for (auto const& p : patterns) {
      if (stackline.contains(p)) {
        stackline.remove(p);
        isUserCode = true;
      }
    }
    auto formatted = formatStackItem(stackline, isUserCode) + "\n";
    // cout << formatted.toStdString();
    stack_text += formatted;
  }
  // qDebug() << stack_text;

  // TODO bug: there are blank lines between each line
  // TODO bug: we're missing the bottom half of the stack
  stack_text.replace("\n", "  \n");  // markdown new-lines

  // debug
  auto file = QFile{"/home/romain/log.md"};
  file.open(QIODeviceBase::WriteOnly | QIODeviceBase::Append);
  file.write(stack_text.toUtf8());
  cout << "\n\n\n" << stack_text.toStdString() << endl;

  stack_label->setText(stack_text);

  auto stack_area = new QScrollArea;
  stack_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  stack_area->setWidget(stack_label);
  stack_area->setWidgetResizable(true);
  layout_root->addWidget(button_quit);
  // TODO button to copy the PID
  layout_root->addWidget(stack_area);

  dialog->resize(800, 500);
  dialog->exec();
}
