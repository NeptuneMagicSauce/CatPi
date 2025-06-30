#include "Delay.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
// #include <iostream>

#include "DeltaDial.hpp"
#include "System.hpp"

struct DelayImpl {
  DelayImpl(Delay* parent);

  QLabel* remaining = nullptr;
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

  remaining = new QLabel;
  remaining->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  delayLabel->setAlignment(remaining->alignment());

  layoutTimes->addWidget(remaining);
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

void Delay::setDelay(int seconds) {
  impl->delayLabel->setText(QString{"Delay: "} + impl->FormatTime(seconds));
  impl->progress->setMaximum(seconds);
}

void Delay::setRemaining(int seconds) {
  impl->remaining->setText(QString{"Remaining: "} + impl->FormatTime(seconds));
  impl->progress->setValue(impl->progress->maximum() - seconds);
}
