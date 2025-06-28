#include "Weight.hpp"

#include <QElapsedTimer>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <iomanip>
#include <iostream>

#include "LoadCell.hpp"
#include "Settings.hpp"
#include "System.hpp"

using namespace std;

struct WeightImpl {
  WeightImpl(auto parent, LoadCell *loadcell);
  void connect();

  QLabel *label = new QLabel;
  QLabel *labelFooter = new QLabel;
  double massGrams = 0;
  LoadCell *loadcell;
  struct {
    const QString key = "tare";
    double value = 0;
    QPushButton *button = new QPushButton();
    QTimer *buttonPressedTimer = new QTimer();
    int buttonPressedTicks = 0;
    const int maxTicks = 40;
    const int interval = 20;
    QString const buttonText = "⚖️ Tare";
    QProgressBar *progress = new QProgressBar();
  } tare;
};

namespace {
WeightImpl *impl = nullptr;
}

Weight::Weight(LoadCell *loadcell) { impl = new WeightImpl(this, loadcell); }

WeightImpl::WeightImpl(auto parent, LoadCell *loadcell) : loadcell(loadcell) {
  AssertSingleton();
  label->setText("--");
  label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  labelFooter->setText("grams");
  labelFooter->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  labelFooter->setStyleSheet("QWidget{font-size: 30pt;}");

  tare.button->setText(tare.buttonText);
  tare.button->setStyleSheet("QAbstractButton{font-size: 36pt; padding-top: 15px; padding-bottom: 15px} ");

  tare.value = Settings::instance().value(tare.key, 0.0).toDouble();
  // // debug
  // std::cout << "Tare " << tare.value << endl;
  tare.buttonPressedTimer->setSingleShot(true);
  tare.buttonPressedTimer->setInterval(tare.interval);
  tare.progress->setMaximum(tare.maxTicks);
  tare.progress->setTextVisible(false);
  tare.progress->setMaximumHeight(15);
  tare.progress->setSizePolicy({QSizePolicy::Policy::Minimum, tare.progress->sizePolicy().verticalPolicy()});

  auto layout = new QVBoxLayout();
  parent->setLayout(layout);
  layout->addWidget(label);
  layout->addWidget(labelFooter);
  layout->addWidget(tare.progress);
  tare.progress->setVisible(false);
  layout->addWidget(tare.button);
}

void Weight::connect() { impl->connect(); }

double Weight::tare() { return impl->tare.value; }

double Weight::update(std::optional<double> value) {
  if (value.has_value() == false) {
    impl->label->setText("Error");
    return 0;
  }

  ostringstream massSs;
  auto weightTarred = *value - impl->tare.value;
  massSs << fixed << setprecision(1) << weightTarred;
  impl->label->setText(QString::fromStdString(massSs.str()));

  impl->massGrams = *value;

  return weightTarred;

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
      Settings::instance().setValue(tare.key, tare.value);
    }
  });
}
