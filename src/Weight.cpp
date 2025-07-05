#include "Weight.hpp"

#include <QElapsedTimer>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <iomanip>
#include <iostream>

#include "Emojis.hpp"
#include "LoadCell.hpp"
#include "Settings.hpp"
#include "System.hpp"

using namespace std;

struct WeightImpl {
  WeightImpl();
  void connect();

  optional<double> weightTarred;
  QLabel *label = new QLabel;
  QLabel *labelFooter = new QLabel;
  QTimer *eventTareFinished = new QTimer;
  QLayout *layout = new QVBoxLayout;
  double massGrams = 0;
  struct {
    const QString key = "Tare";
    double value = 0;
    QPushButton *button = new QPushButton();
    QTimer *buttonPressedTimer = new QTimer();
    int buttonPressedTicks = 0;
    const int maxTicks = 40;
    const int interval = 20;
    QString const buttonText = " Tare";
    QProgressBar *progress = new QProgressBar();
  } tare;
};

namespace {
  WeightImpl *impl = nullptr;
}

optional<double> Weight::weightTarred() { return impl->weightTarred; }

Weight::Weight() : messageFinished(Emojis::get(Emojis::Type::OkayWithThreeVSigns)) {
  impl = new WeightImpl();
  setLayout(impl->layout);
}

WeightImpl::WeightImpl() {
  AssertSingleton();
  label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  labelFooter->setText("grams");
  labelFooter->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  labelFooter->setStyleSheet("QWidget{font-size: 30pt;}");

  tare.button->setText(tare.buttonText);
  tare.button->setStyleSheet(
      "QAbstractButton{font-size: 36pt; padding-top: 15px; padding-bottom: 15px} ");
  tare.button->setIcon(QIcon{QPixmap("://weightbalance.png")});
  tare.button->setIconSize({40, 40});

  Settings::load({tare.key,
                  "Tare",
                  "Tare de la balance",
                  "Grammes",
                  0.0,
                  [&](QVariant v) { tare.value = v.toInt(); },
                  {{}, {}}});
  // // debug
  // std::cout << "Tare " << tare.value << endl;
  tare.buttonPressedTimer->setSingleShot(true);
  tare.buttonPressedTimer->setInterval(tare.interval);
  tare.progress->setMaximum(tare.maxTicks);
  tare.progress->setTextVisible(false);
  tare.progress->setMaximumHeight(15);
  tare.progress->setSizePolicy(
      {QSizePolicy::Policy::Minimum, tare.progress->sizePolicy().verticalPolicy()});

  layout->addWidget(label);
  layout->addWidget(labelFooter);
  layout->addWidget(tare.progress);
  tare.progress->setVisible(false);
  layout->addWidget(tare.button);

  eventTareFinished->setSingleShot(true);
  eventTareFinished->setInterval(1);
}

void Weight::connect() { impl->connect(); }

double Weight::tare() { return impl->tare.value; }

QTimer *Weight::eventTareFinished() { return impl->eventTareFinished; }

void Weight::update(std::optional<double> value) {
  if (value.has_value() == false) {
    impl->weightTarred = {};
    impl->label->setText("--");
    return;
  }

  ostringstream massSs;
  impl->weightTarred = value.value() - impl->tare.value;
  massSs << fixed << setprecision(1) << impl->weightTarred.value();
  impl->label->setText(QString::fromStdString(massSs.str()));

  impl->massGrams = value.value();

  // // debug
  // cout << "mass " << value << " tare " << impl->tare.value << endl;
}

void WeightImpl::connect() {
  QObject::connect(tare.button, &QAbstractButton::pressed, [this]() {
    tare.buttonPressedTicks = 0;
    tare.progress->setValue(0);
    tare.progress->setVisible(true);
    tare.buttonPressedTimer->start();
  });
  QObject::connect(tare.button, &QAbstractButton::released, [this]() {
    tare.buttonPressedTimer->stop();
    tare.progress->setVisible(false);
  });
  QObject::connect(tare.buttonPressedTimer, &QTimer::timeout, [this]() {
    ++tare.buttonPressedTicks;
    tare.progress->setValue(tare.buttonPressedTicks);
    if (tare.buttonPressedTicks < tare.maxTicks) {
      tare.buttonPressedTimer->start();
    } else {
      tare.progress->setVisible(false);
      // cout << "long press " << massGrams << endl;
      tare.value = massGrams;
      Settings::set(tare.key, tare.value);
      eventTareFinished->start();
    }
  });
}
