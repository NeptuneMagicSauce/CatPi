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
  QWidget* textAndButtons = new QWidget;
  QProgressBar* progress = new QProgressBar;
  QString textDelay, textRemaining;

  static QString FormatTime(std::optional<int> seconds);

  void buildTextAndButtons(Delay* parent);
  void updateLabel();
};

namespace {
  DelayImpl* impl = nullptr;
}

Delay::Delay() {
  AssertSingleton();
  setMinimumWidth(380);
  delayDial = new DeltaDial;
  delayDial->setSizePolicy(delayDial->sizePolicy().horizontalPolicy(),
                           QSizePolicy::Policy::Expanding);
  buttonDay = new QToolButton;
  buttonNight = new QToolButton;
  impl = new DelayImpl(this);
}

void DelayImpl::buildTextAndButtons(Delay* parent) {
  auto layoutTextAndButtons = new QHBoxLayout;
  textAndButtons->setLayout(layoutTextAndButtons);
  layoutTextAndButtons->addWidget(label);
  layoutTextAndButtons->addWidget(parent->buttonDay);
  layoutTextAndButtons->addWidget(parent->buttonNight);

  label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  label->setTextFormat(Qt::RichText);

  parent->buttonDay->setIcon(QIcon{QPixmap("://day.png")});
  parent->buttonNight->setIcon(QIcon{QPixmap("://night.png")});

  parent->buttonDay->setIconSize({50, 50});
  parent->buttonNight->setIconSize(parent->buttonDay->iconSize());

  Widget::FontSized(textAndButtons, 20);

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
  auto layout = new QVBoxLayout;
  parent->setLayout(layout);

  buildTextAndButtons(parent);

  progress->setTextVisible(false);
  progress->setSizePolicy(QSizePolicy::Policy::Minimum, progress->sizePolicy().verticalPolicy());
  progress->setMaximumHeight(27);

  parent->textAndButtons = textAndButtons;
  parent->progress = progress;
}

void Delay::attachWidgets() {
  auto layout = this->layout();
  layout->addWidget(delayDial);
  layout->addWidget(textAndButtons);
  layout->addWidget(progress);
}

QString DelayImpl::FormatTime(std::optional<int> secondsOptional) {
  if (secondsOptional.has_value() == false) {
    return "--";
  }
  auto const& seconds = secondsOptional.value();
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

void Delay::setRemaining(std::optional<int> seconds) {
  impl->textRemaining =  //"Attente: " +
      impl->FormatTime(seconds);
  impl->updateLabel();
  impl->progress->setValue(seconds.has_value() ? (impl->progress->maximum() - seconds.value()) : 0);
}

void DelayImpl::updateLabel() { label->setText(textRemaining + "<br>" + textDelay); }
