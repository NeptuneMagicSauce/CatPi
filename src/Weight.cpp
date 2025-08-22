#include "Weight.hpp"

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

  Settings::load({tare.key,
                  "Tare",
                  "Tare de la balance",
                  "Grammes",
                  0.0,
                  [&](QVariant v) { tare.value = v.toDouble(); },
                  {{}, {}}});
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
    return;
  }

  ostringstream massSs;
  impl->weightTarred = value.value() - impl->tare.value;
  massSs << fixed << setprecision(1) << impl->weightTarred.value();
  impl->label->setText(QString::fromStdString(massSs.str()));

  impl->massGrams = value.value();
}
