#include "Delay.hpp"

#include <QBoxLayout>
#include <QLabel>
#include <QProgressBar>
// #include <iostream>

#include "DeltaDial.hpp"
#include "System.hpp"
#include "Widget.hpp"

struct DelayImpl {
  DelayImpl(Delay* parent);

  QString textDelay, textRemaining;
  QLabel* label = new QLabel;
  QProgressBar* progress = new QProgressBar;

  static QString FormatTime(int seconds);
  void updateLabel();
};

namespace {
  DelayImpl* impl = nullptr;
}

Delay::Delay() {
  AssertSingleton();
  setMinimumWidth(350);
  delayDial = new DeltaDial;
  delayDial->setSizePolicy(
      {delayDial->sizePolicy().horizontalPolicy(), QSizePolicy::Policy::Expanding});
  impl = new DelayImpl(this);
}

DelayImpl::DelayImpl(Delay* parent) {
  Widget::FontSized(parent, 20);
  auto layout = new QVBoxLayout;
  parent->setLayout(layout);

  label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  label->setTextFormat(Qt::RichText);

  layout->addWidget(parent->delayDial);
  layout->addWidget(label);
  layout->addWidget(progress);

  progress->setTextVisible(false);
  progress->setSizePolicy({QSizePolicy::Policy::Minimum, progress->sizePolicy().verticalPolicy()});
}

QString DelayImpl::FormatTime(int seconds) {
  if (seconds >= 60) {
    auto minutes = std::round((double)seconds / 60);
    return QString::number(minutes) + " minute" + (minutes > 1 ? "s" : "");
  }
  return QString::number(seconds) + " seconde" + (seconds > 1 ? "s" : "");
}

void Delay::setDelay(int seconds) {
  impl->textDelay = "<b>" + impl->FormatTime(seconds) + "</b>";
  impl->updateLabel();
  impl->progress->setMaximum(seconds);
}

void Delay::setRemaining(int seconds) {
  impl->textRemaining = "Attente: " + impl->FormatTime(seconds);
  impl->updateLabel();
  impl->progress->setValue(impl->progress->maximum() - seconds);
}

void DelayImpl::updateLabel() { label->setText(textRemaining + "<br>" + textDelay); }
