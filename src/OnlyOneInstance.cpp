#include "OnlyOneInstance.hpp"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>

OnlyOneInstance::OnlyOneInstance() {
  assert(qApp != nullptr);
  auto pid = QApplication::applicationPid();

  auto procDir = QDir{"/proc"};
  auto exeOfPid = [&](auto pid) {
    return QFile{procDir.path() + "/" + pid + "/exe"}.symLinkTarget();
  };

  auto myExe = exeOfPid(QString::number(pid));

  for (auto process : procDir.entryList({"[0-9]*"})) {
    if (process.toInt() == pid) {
      continue;
    }
    auto exe = procDir.path() + "/" + process + "/exe";
    if (exeOfPid(process) == myExe) {
      QMessageBox::critical(nullptr, "", "Already running");
      std::exit(1);
    }
  }
}
