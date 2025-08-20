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
        temperature = "temp=";
      }
      // temperature = "temp=49.6'C";

      process->start("vcgencmd", {"measure_volts"});
      process->waitForFinished();
      auto volts = process->readAll();
      if (volts.startsWith("volt=") == false) {
        volts = "volts=";
      }
      // volts = "volt=0.9260V";

      label->setText(temperature + "\n" + volts);
    }
  });
}
