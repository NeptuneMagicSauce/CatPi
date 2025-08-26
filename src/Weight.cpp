#include "Weight.hpp"

#include <QDateTime>
#include <QElapsedTimer>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <iomanip>
#include <iostream>

#include "ProtectedButton.hpp"
#include "Settings.hpp"
#include "System.hpp"
#include "Widget.hpp"

using namespace std;

struct WeightImpl {
  WeightImpl();

  optional<double> weightTarred;
  QLabel *label = new QLabel;
  QLabel *labelFooter = new QLabel;
  QLayout *layout = new QVBoxLayout;
  double massGrams = 0;

  struct {
    ProtectedButton button;
    double value = 0;
    const QString key = "Tare";
  } tare;

  struct Measures {
    struct Measure {
      QDateTime dateTime;
      optional<double> value;
    };
    QList<Measure> data;
  } measures;

  struct {
    double weightThresholdValue = 0.4;
    int weightThresholdDurationMilliSecs = 0;
  } settings;

  bool isBelowThreshold() const;
};

namespace {
  WeightImpl *impl = nullptr;
}

optional<double> Weight::weightTarred() { return impl->weightTarred; }

Weight::Weight()
    :  // messageFinished(Emojis::get(Emojis::Type::OkayWithThreeVSigns))
      messageFinished("Tare OK") {
  AssertSingleton();
  impl = new WeightImpl();
  setLayout(impl->layout);

  QObject::connect(impl->tare.button.finished, &QTimer::timeout, [&] { doTare(); });

  setEnabled(false);
}

WeightImpl::WeightImpl() {
  label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  labelFooter->setText("grammes");
  labelFooter->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  labelFooter->setStyleSheet("QWidget{font-size: 30pt;}");

  tare.button.button->setText("Tare");
  Widget::FontSized(tare.button.button, 25);
  // tare.button.button->setStyleSheet(
  // "QAbstractButton{font-size: 25pt; padding-top: 15px; padding-bottom: 15px} ");
  // tare.button.button->setIcon(QIcon{QPixmap("://weightbalance.png")});
  tare.button.button->setIconSize({40, 40});

  Settings::load({.key = tare.key,
                  .name = "Tare",
                  .prompt = "Tare de la balance",
                  .unit = "Grammes",
                  .defaultValue = 0.0,
                  .callback = [&](QVariant v) { tare.value = v.toDouble(); },
                  .limits = {.minimum = {}, .maximum = {}}});
  Settings::load(
      {.key = "WeightThresholdValue",
       .name = "Poids Minimum Mangé",
       .prompt = "Poids en dessous duquel on détecte que c'est mangé",
       .unit = "Déci-Grammes",
       .defaultValue = 4,
       .callback = [&](QVariant v) { settings.weightThresholdValue = (double)v.toInt() / 10; },
       .limits = {.minimum = 1, .maximum = 20}});
  Settings::load(
      {.key = "WeightThresholdDuration",
       .name = "Durée Détection Vide",
       .prompt =
           "Temps passé avec le poids en dessous du seuil pour considérer que la gamelle est vide",
       .unit = "Secondes",
       .defaultValue = 2,
       .callback =
           [&](QVariant v) { settings.weightThresholdDurationMilliSecs = v.toInt() * 1000; },
       .limits = {.minimum = 1, .maximum = 10}});

  layout->addWidget(label);
  layout->addWidget(labelFooter);
  layout->addWidget(&tare.button);
}

double Weight::getTare() { return impl->tare.value; }

void Weight::doTare() {
  // qDebug() << Q_FUNC_INFO << impl->tare.value << "->" << impl->massGrams;
  impl->tare.value = impl->massGrams;
  Settings::set(impl->tare.key, impl->tare.value);
}

QTimer *Weight::eventTareFinished() { return impl->tare.button.finished; }

void Weight::update(std::optional<double> value) {
  if (value.has_value() == false) {
    impl->weightTarred = {};
    impl->label->setText("--");
  } else {
    ostringstream massSs;
    impl->weightTarred = value.value() - impl->tare.value;
    massSs << fixed << setprecision(1) << impl->weightTarred.value();
    impl->label->setText(QString::fromStdString(massSs.str()));

    impl->massGrams = value.value();
  }

  // store recent measures
  auto &measures = impl->measures.data;
  auto const now = QDateTime::currentDateTime();
  auto measure = WeightImpl::Measures::Measure{.dateTime = now, .value = {}};
  if (value.has_value()) {
    measure.value = impl->weightTarred;
  }
  measures.append(measure);

  // remove old measures
  while (measures.empty() == false && measures.first().dateTime.secsTo(now) > 30) {
    measures.removeFirst();
  }
}

bool Weight::isBelowThreshold() const { return impl->isBelowThreshold(); }

bool WeightImpl::isBelowThreshold() const {
  if (measures.data.empty()) {
    return true;
  }

  auto const now = QDateTime::currentDateTime();
  int index = measures.data.size() - 1;
  while (index >= 0) {
    auto const &measure = measures.data[index];
    if (measure.dateTime.msecsTo(now) > settings.weightThresholdDurationMilliSecs) {
      break;
    }
    if (measure.value.value_or(0) > settings.weightThresholdValue) {
      return false;
    }
    --index;
  }
  return true;
}

QString Weight::toString() const {
  auto const now = QDateTime::currentDateTime();
  ostringstream ss;
  ss << now.toString().toStdString() + ", recent Weights {MilliSecsAgo, Grams}, ";
  int index = impl->measures.data.size() - 1;
  while (index >= 0) {
    auto const &measure = impl->measures.data[index];
    if (measure.dateTime.msecsTo(now) > impl->settings.weightThresholdDurationMilliSecs) {
      break;
    }
    ss << "{" << measure.dateTime.msecsTo(now) << ",";
    if (measure.value.has_value()) {
      ss << fixed << setprecision(2) << measure.value.value();
    } else {
      ss << "-";
    }
    ss << "}, ";
    --index;
  }
  return QString::fromStdString(ss.str());
}
