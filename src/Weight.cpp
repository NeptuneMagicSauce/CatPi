#include "Weight.hpp"

#include <QElapsedTimer>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>
#include <iomanip>
#include <iostream>
#include <map>

#include "LoadCell.hpp"
#include "Settings.hpp"
#include "System.hpp"

using namespace std;

struct WeightImpl {
  WeightImpl(auto parent, LoadCell *loadcell);
  void connect();

  QTimer *timer = new QTimer;
  QLabel *label = new QLabel;
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

  tare.button->setText(tare.buttonText);
  tare.button->setStyleSheet("QAbstractButton{font-size: 36pt; padding-top: 15px; padding-bottom: 15px} ");

  tare.value = Settings::instance().value(tare.key, 0.0).toDouble();
  // // debug
  // std::cout << "Tare " << tare.value << endl;
  tare.buttonPressedTimer->setSingleShot(true);
  tare.buttonPressedTimer->setInterval(tare.interval);
  tare.progress->setMaximum(tare.maxTicks);
  tare.progress->setTextVisible(false);
  tare.progress->setSizePolicy({QSizePolicy::Policy::Minimum, tare.progress->sizePolicy().verticalPolicy()});

  auto layout = new QVBoxLayout();
  parent->setLayout(layout);
  layout->addWidget(label);
  layout->addWidget(tare.progress);
  tare.progress->setVisible(false);
  layout->addWidget(tare.button);

  timer->setSingleShot(false);
  timer->start(1000);
}

void Weight::connect() { impl->connect(); }

void WeightImpl::connect() {
  QObject::connect(timer, &QTimer::timeout, [this]() {
    auto mass = loadcell->valueGrams();
    if (mass == nullopt) {
      label->setText("error");
      return;
    }

    ostringstream massSs;
    massSs << fixed << setprecision(1) << *mass - tare.value;
    label->setText(QString::fromStdString(massSs.str()) + "\ngrams");

    // // debug
    // cout << "mass " << massGrams << " tare " << tare.value << endl;
  });

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
      // cout << "long press" << endl;
      tare.value = massGrams;
      Settings::instance().setValue(tare.key, tare.value);
    }
  });
}
