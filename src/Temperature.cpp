#include "Temperature.hpp"

#include <QBoxLayout>
#include <QLabel>
#include <QProcess>
#include <QTimer>

#include "System.hpp"
#include "Widget.hpp"

namespace {
  QProcess* process = nullptr;
}

Temperature::Temperature() {
  AssertSingleton();
  process = new QProcess;

  auto label = new QLabel;
  Widget::FontSized(label, 15);

  setLayout(new QVBoxLayout);
  layout()->addWidget(label);

  auto timer = new QTimer;
  timer->setSingleShot(false);
  timer->setInterval(1000);
  timer->start();

  QObject::connect(timer, &QTimer::timeout, [=] {
    if (label->isVisible()) {
      process->start("vcgencmd", {"measure_temp"});
      process->waitForFinished();
      auto temperature = process->readAll();
      if (temperature.startsWith("temp=") == false) {
        temperature = "temp=00.0'C";
      }

      process->start("vcgencmd", {"measure_volts"});
      process->waitForFinished();
      auto volts = process->readAll();
      if (volts.startsWith("volt=") == false) {
        volts = "volt=0.0000V";
      }

      // using HTML in QLabel because it has normal line height
      // otherwise it does not fit with big line height of default / non-html
      label->setText("<p>" + temperature + "<br>" + volts + "</p>");
    }
  });
}
