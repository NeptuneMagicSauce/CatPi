#include "Delay.hpp"

#include <QBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QToolButton>
// #include <iostream>

#include "DeltaDial.hpp"
#include "Settings.hpp"
#include "System.hpp"
#include "Widget.hpp"

struct DelayImpl {
  DelayImpl(Delay* parent);

  QLabel* label = new QLabel;
  QWidget* subwidget = new QWidget;
  QProgressBar* progress = new QProgressBar;
  QString textDelay, textRemaining;

  static QString FormatTime(int seconds);

  void buildSubWidget(Delay* parent);
  void updateLabel();
};

namespace {
  DelayImpl* impl = nullptr;
}

Delay::Delay() {
  AssertSingleton();
  setMinimumWidth(380);
  delayDial = new DeltaDial;
  delayDial->setSizePolicy(
      {delayDial->sizePolicy().horizontalPolicy(), QSizePolicy::Policy::Expanding});
  buttonDay = new QToolButton;
  buttonNight = new QToolButton;
  impl = new DelayImpl(this);
}

void DelayImpl::buildSubWidget(Delay* parent) {
  auto layoutSubWidget = new QHBoxLayout;
  subwidget->setLayout(layoutSubWidget);
  layoutSubWidget->addWidget(label);
  layoutSubWidget->addWidget(parent->buttonDay);
  layoutSubWidget->addWidget(parent->buttonNight);

  label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  label->setTextFormat(Qt::RichText);

  parent->buttonDay->setIcon(QIcon{QPixmap("://day.png")});
  parent->buttonNight->setIcon(QIcon{QPixmap("://night.png")});

  parent->buttonDay->setIconSize({50, 50});
  parent->buttonNight->setIconSize(parent->buttonDay->iconSize());

  Settings::load({"DelayDayButton",
                  "Bouton Jour",
                  "Attente ouverture du bouton Jour",
                  "Minutes",
                  15,
                  [parent](QVariant v) { parent->delayDaySeconds = v.toInt(); },
                  {5, 60}});
  Settings::load({"DelayNightButton",
                  "Bouton Nuit",
                  "Attente ouverture du bouton Nuit",
                  "Minutes",
                  60,
                  [parent](QVariant v) { parent->delayNightSeconds = v.toInt(); },
                  {30, 120}});
}

DelayImpl::DelayImpl(Delay* parent) {
  Widget::FontSized(parent, 20);
  auto layout = new QVBoxLayout;
  parent->setLayout(layout);

  buildSubWidget(parent);

  layout->addWidget(parent->delayDial);
  layout->addWidget(subwidget);
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
  impl->textRemaining =  //"Attente: " +
      impl->FormatTime(seconds);
  impl->updateLabel();
  impl->progress->setValue(impl->progress->maximum() - seconds);
}

void DelayImpl::updateLabel() { label->setText(textRemaining + "<br>" + textDelay); }
