#include "CrashDialog.hpp"

#include <QApplication>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QMap>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QTemporaryFile>
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
  auto bold(const QString& str, bool isUserCode = true) {
    if (isUserCode == false) {
      return str;
    }
    return QString{"<b>"} + str + "</b>";
  }
  auto italic(const QString& str) { return QString{"<i>"} + str + "</i>"; }
  auto formatIndex(const QString& index) { return "[" + QString::number(index.toInt()) + "]"; }
  auto newline(int count = 1) {
    auto ret = QString{};
    for (int i = 0; i < count; ++i) {
      ret += "<br>";
    }
    return ret;
  }

  struct Frame {
    QString file;
    QString function;
    QString address;

    QString info;  // that's the output of backtrace_symbols()
    QString canonicalPath;
    QString filename;
  };
  QList<Frame> frames;

  QString parseStackItem(const QString& line, bool isUserCode) {
    try {
      // #ifdef __aarch64__
      {
        // /home/romain/CatPi/CatPi(+0x2d7d4) [0x556da4d7d4]
        // linux-vdso.so.1(__kernel_rt_sigreturn+0) [0x7f99c84820]
        // /lib/aarch64-linux-gnu/libQt6Widgets.so.6(_ZN15QAbstractButton15keyReleaseEventEP9QKeyEvent+0x11c)[0x7f99799efc]
        // auto regexep = QRegularExpression{"(^.*)(\\(.*\\))"};
        auto regexep = QRegularExpression{"(^.*)(\\()(.*)(\\[)(.*)(\\])"};
        auto match = regexep.match(line);
        if (match.hasMatch() == false) {
          return line;
        }

        auto file = match.captured(1);
        auto function = match.captured(3);
        auto address = match.captured(5);
        auto fileinfo = QFileInfo{file};
        frames.append(
            {file, function, address, line, fileinfo.canonicalFilePath(), fileinfo.fileName()});
        // qDebug() << match.captured(1) << match.captured(2) << match.captured(3);
        return address;
      }
      // #elifdef __x86_64__
      //   5# main at Main.cpp:30
      //   6# __libc_start_call_main at ../sysdeps/nptl/libc_start_call_main.h:58
      //   7#      at :0
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
      // #endif
      // qDebug() << index << function << location;

      auto ret = QString{};

      auto indexStr = formatIndex(index);
      // TODO move this test in bold()
      if (isUserCode) {
        indexStr = bold(indexStr);
      }
      ret += indexStr;

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

QStringList cppfilt(const QString& mangled) {
  auto process1 = QProcess{};
  auto process2 = QProcess{};
  auto f = QTemporaryFile{};
  f.open();
  f.write(mangled.toUtf8());
  f.close();

  auto args = QStringList{} << "-c" << "cat " + f.fileName() + " | c++filt";
  process1.start("sh", args);
  process1.waitForFinished();
  auto ret = QString{process1.readAll()}.split("\n");
  return ret;
}

QString addr2line(const QString& file, const QString& addresses) {
  QProcess p;
  QStringList args = {
      "-C",  // --demangle
      // "-s" // --basenames
      // "-a",  // --addresses
      "-f",  // --functions
      "-p",  // --pretty-print
      "-e",
      file,
  };
  args << addresses.trimmed().split(" ");

  p.start("addr2line", args);
  if (!p.waitForStarted()) {
    QStringList ret;
    return "addr2line failed for " + addresses;
  }
  p.waitForFinished();
  auto process_output = QString{p.readAll()};
  return process_output;
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

  auto addresses = QString{};
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
    auto formatted = parseStackItem(stackline, isUserCode);
    addresses += formatted + " ";
    stack_text += formatted + newline();
  }
  // stack_text = addr2line(addresses).replace("\n", newline());
  stack_text = "";

  auto const mypath = QFileInfo{qApp->applicationFilePath()}.canonicalFilePath();

  // demangle functions
  auto functions = QString{};
  auto areUserCode = QMap<int, bool>{};
  int indexFunction = 0;
  for (const auto& frame : frames) {
    auto isUserCode = (bool)(frame.canonicalPath == mypath);
    areUserCode[indexFunction] = isUserCode;
    functions += (isUserCode ? "" : frame.function) + "\n";
    ++indexFunction;
  }
  auto demangled = cppfilt(functions);
  // qDebug() << demangled;

  // format frames
  int index = 0;
  for (const auto& frame : frames) {
    auto isUserCode = areUserCode[index];
    auto line = formatIndex(QString::number(index)) + " ";
    if (isUserCode) {
      // backtrace_symbols() can not print my addresses
      line += addr2line(frame.file, frame.address);
      line.replace(" at ", newline());
    } else {
      line += demangled[index] + newline() + frame.filename;
    }

    line = bold(line, isUserCode);

    stack_text += line + newline();

    ++index;
  }
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
