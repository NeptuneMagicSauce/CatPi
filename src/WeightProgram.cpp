#include "WeightProgram.hpp"

#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <iostream>

using namespace std;

WeightProgram::WeightProgram() {
  auto measureProcess = QProcess();

  auto const home = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::HomeLocation).first();

  auto measureProgram = home + "/examples/min.py";
  if (QFile{measureProgram}.exists()) {
    measureProcess.setProgram(measureProgram);
    measureProcess.setArguments({"-q"});
    measureProcess.startDetached(&pid);
    std::cout << "measure process pid " << pid << endl;
  }
}

WeightProgram::~WeightProgram() {
  std::cout << __PRETTY_FUNCTION__ << endl;
  // TODO do this clean-up when Ctrl-C
  if (pid != 0) {
    QProcess killProcess;
    killProcess.setProgram("kill");
    killProcess.setArguments({QString::number(pid)});
    killProcess.startDetached();
  }
}
