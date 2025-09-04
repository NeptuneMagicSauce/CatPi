#include "LogsWidget.hpp"

#include <QBarSeries>
#include <QBarSet>
#include <QBoxLayout>
#include <QChartView>
#include <QDateTime>
#include <QGroupBox>
#include <QLabel>

#include "Logs.hpp"
#include "System.hpp"
#include "Widget.hpp"

struct LogsWidgetImpl {
  LogsWidgetImpl(Logs const& logs);

  Logs const& logs;

  QLayout* layout = nullptr;
  QLabel* title = nullptr;
  QDate current = QDate::currentDate();

  QBarSeries barSeries;
  QBarSet bars{""};
  QChart chart;
  QChartView chartView;

  static QString FormatDay(const auto& day);
  void loadData();

  struct Main : public QWidget {
    Main();
  }* widget = nullptr;
};

namespace {
  LogsWidgetImpl* impl = nullptr;
}

QString LogsWidgetImpl::FormatDay(const auto& day) {
  static auto const namesOfDays = QMap<int, QString>{
      {1, "Lundi"},    {2, "Mardi"},  {3, "Mercredi"}, {4, "Jeudi"},
      {5, "Vendredi"}, {6, "Samedi"}, {7, "Dimanche"},
  };
  static auto const namesOfMonths = QMap<int, QString>{
      {1, "Janvier"},   {2, "Février"},  {3, "Mars"},      {4, "Avril"},
      {5, "Mai"},       {6, "Juin"},     {7, "Juillet"},   {8, "Août"},
      {9, "Septembre"}, {10, "Octobre"}, {11, "Novembre"}, {12, "Décembre"},
  };

  auto ret = QString{};
  ret += namesOfDays[day.dayOfWeek()];
  ret += " ";
  ret += QString::number(day.day());
  ret += " ";
  ret += namesOfMonths[day.month()];
  ret += " ";
  ret += QString::number(day.year());
  return ret;
}

LogsWidget::LogsWidget(Logs const& logs) {
  impl = new LogsWidgetImpl{logs};
  setLayout(impl->layout);
}

LogsWidgetImpl::LogsWidgetImpl(Logs const& logs) : logs(logs) {
  layout = new QVBoxLayout;

  auto titleParent = new QGroupBox;
  auto titleLayout = new QVBoxLayout;
  titleParent->setLayout(titleLayout);
  title = new QLabel;
  titleLayout->addWidget(title);

  layout->addWidget(titleParent);

  Widget::FontSized(title, 15);
  Widget::AlignCentered(title);

  widget = new Main;

  chartView.setChart(&chart);
  barSeries.append(&bars);
  // chart.addSeries(&barSeries);
  layout->addWidget(&chartView);
}

void LogsWidget::loadData() { impl->loadData(); }

void LogsWidgetImpl::loadData() {
  title->setText(FormatDay(current));
  auto const& data = logs.readHistoricalData(current);

  bars.remove(0, bars.count());

  QMap<int, QList<Logs::Event>> eventsPerHour;
  QMap<int, double> eatenWeightsPerHour;
  for (const auto& d : data) {
    if (d.type == Logs::Event::Type::Eat) {
      eventsPerHour[d.time.time().hour()].append(d);
    }
  }
  for (int hour = 0; hour < 24; ++hour) {
    auto total = double{0};
    if (eventsPerHour.contains(hour)) {
      for (const auto& e : eventsPerHour[hour]) {
        qDebug() << "EAT" << e.time.toString() << e.weight;
        total += e.weight;
      }
    }
    eatenWeightsPerHour[hour] = total;
  }
  for (int hour = 0; hour < 24; ++hour) {
    qDebug() << "hour" << hour << "weight" << eatenWeightsPerHour[hour];
    bars << eatenWeightsPerHour[hour];
  }

#warning "how should we addSeries?"
  if (chart.series().empty()) {
    chart.addSeries(&barSeries);
  }

#warning TODO
  // histograms are collated per hour of day
  // - events dispense
  // - events eat
  // - weight eaten
  // with a checkbox to change between these
  // day navigation +/- with buttons around title
}

LogsWidgetImpl::Main::Main() {
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  // setLayout(new QHBoxLayout);
  // layout()->addWidget(
}
