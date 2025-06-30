#include "Delay.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>

#include "DeltaDial.hpp"
#include "System.hpp"

struct DelayImpl {
  DelayImpl(Delay* parent);

  QLabel* timeToDispense = nullptr;
  QLabel* delayLabel = new QLabel;
  QProgressBar* progress = new QProgressBar;

  static QString FormatTime(int seconds);
};

namespace {
DelayImpl* impl = nullptr;
}

Delay::Delay(int delaySeconds) {
  AssertSingleton();
  delayDial = new DeltaDial;
  delayDial->setMaximum(10);
  impl = new DelayImpl(this);
  setDelay(delaySeconds);
}

void Delay::connect() { delayDial->connect(); }

DelayImpl::DelayImpl(Delay* parent) {
  auto layout = new QHBoxLayout;
  parent->setLayout(layout);

  auto times = new QWidget;
  times->setStyleSheet("QWidget{font-size: 15pt;}");
  auto layoutTimes = new QVBoxLayout;
  times->setLayout(layoutTimes);

  auto& timeTo = timeToDispense;
  timeTo = new QLabel;
  timeTo->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  delayLabel->setAlignment(timeTo->alignment());

  layoutTimes->addWidget(timeTo);
  layoutTimes->addWidget(delayLabel);
  layoutTimes->addWidget(progress);

  progress->setTextVisible(false);
  progress->setSizePolicy({QSizePolicy::Policy::Minimum, progress->sizePolicy().verticalPolicy()});

  layout->addWidget(times);
  layout->addWidget(parent->delayDial);
}

QString DelayImpl::FormatTime(int seconds) {
  if (seconds >= 60) {
    auto minutes = std::round((double)seconds / 60);
    return QString::number(minutes) + " minute" + (minutes > 1 ? "s" : "");
  }
  return QString::number(seconds) + " second" + (seconds > 1 ? "s" : "");
}

void Delay::setDelay(int delaySeconds) {
  impl->delayLabel->setText(QString{"Delay: "} + impl->FormatTime(delaySeconds));
  impl->progress->setMaximum(delaySeconds);
}

void Delay::setTimeToDispense(int seconds) {
  impl->timeToDispense->setText(QString{"Remaining: "} + impl->FormatTime(seconds));
  impl->progress->setValue(impl->progress->maximum() - seconds);
}
