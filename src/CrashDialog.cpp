#include "CrashDialog.hpp"

#include <QApplication>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
// #include <iostream>
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
      {CrashHandler::Test::Type::SigIllFormed, "SigIllegal"},
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
  auto iswhitespace(const QString& str) {
    auto regexp = QRegularExpression{"^ *$"};
    return regexp.match(str).hasMatch();
  }
  auto isempty(const QString& str) { return str.isEmpty() || iswhitespace(str) || str == ":0"; }
  auto bold(const QString& str) { return QString{"<b>"} + str + "</b>"; }
  auto italic(const QString& str) { return QString{"<i>"} + str + "</i>"; }
  auto formatIndex(const QString& index) { return "[" + QString::number(index.toInt()) + "]"; }
  auto newline(int count = 1) {
    auto ret = QString{};
    for (int i = 0; i < count; ++i) {
      ret += "<br>";
    }
    return ret;
  }

  QString formatStackItem(const QString& line, bool isUserCode) {
    //   5# main at Main.cpp:30
    //   6# __libc_start_call_main at ../sysdeps/nptl/libc_start_call_main.h:58
    //   7#      at :0

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

      auto indexStr = formatIndex(index);
      if (isUserCode) {
        indexStr = bold(indexStr);
      }
      ret += index;

      auto hasfunction = isempty(function) == false;
      if (hasfunction) {
        ret += " ";
        function = italic(function);
        if (isUserCode) {
          function = bold(function);
        }
        ret += function;
      }

      if (isempty(location) == false) {
        ret += hasfunction ? newline() : " ";
        if (isUserCode) {
          location = bold(location);
        }
        ret += location;
      }
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

  auto dialog = new QDialog{nullptr, Qt::Window};
  Widget::FontSized(dialog, 20);
  dialog->setModal(true);
  dialog->setWindowTitle("Crash");

  auto layout_root = new QVBoxLayout;
  dialog->setLayout(layout_root);

  auto error_label = new QLabel{bold(error)};
  error_label->setTextFormat(Qt::RichText);
  layout_root->addWidget(Widget::AlignCentered(error_label));

  auto button_quit = new QPushButton{"Quit"};
  button_quit->setDefault(true);
  QObject::connect(button_quit, &QPushButton::released, [&dialog]() { dialog->accept(); });

  auto stack_label = new QLabel;
  stack_label->setTextFormat(Qt::RichText);
  auto stack_text = QString{};

  auto pid = QString::number(QApplication::applicationPid());
  stack_text += bold("Process ID") + " " + pid + newline(2);

  stack_text += bold("Stack Trace") + newline(2);

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
    auto formatted = formatStackItem(stackline, isUserCode) + newline();
    // cout << formatted.toStdString();
    stack_text += formatted;
  }
  // qDebug() << stack_text;

  stack_label->setText(stack_text);

  auto stack_area = new QScrollArea;
  stack_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  stack_area->setWidget(stack_label);
  stack_area->setWidgetResizable(true);
  layout_root->addWidget(button_quit);
  layout_root->addWidget(stack_area);

  if (Widget::IsSmallScreen()) {
    dialog->resize(800, 450);
    dialog->showMaximized();
  } else {
    dialog->resize(1200, 900);
  }
  dialog->exec();
}
